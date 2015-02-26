/**
Last modificaton date: 25/02/2015
Modification_1:     //unixTime+=3600;  (Commented - we do not need add 1 hour anymore...till march)
**/

#include <avr/pgmspace.h>
#include <avr/wdt.h> 
#include <Udp.h>
#include <RTClib.h>
#include <IniFile.h>
#include <string.h>

#include <DS1307new.h>  //DS1307 Module
#include <SPI.h>
#include <Ethernet.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SD.h>

#include "RestClient.h"
#include "EmonLib.h"
#include "includes.h"


/** IP Addresses for Networking **/
typedef struct{
  uint8_t myIP[4];
  uint8_t mySubnet[4];
  uint8_t myGateway[4];
  uint8_t myDNS[4];
} NetAddresses;

byte mac[6];
NetAddresses myAddresses;                                         // struct object for IP addresses
RestClient   client = RestClient(IP_DB_SERVER, PORT_DB_SERVER);   //Rest Client 

/** Variables for accessing to RTC clock **/
uint16_t startAddr = 0x0000;            // start address to store in the NV-RAM
uint16_t timeIsSet = 0xaa55;            // helper that time must not set again
uint16_t lastAddr;                      // new address for storing in NV-RAM
uint8_t  dayOfWeek;


/** Variables for accessing to INI file **/
const char *filename        = "/config.ini";   // the name of the config file. Stored in "/"
const      size_t bufferLen = BUFFER_LENGTH;
boolean    rtcOnTime        = false;           // to know if the Arduino was previously set On time or not
char       bufferINIfile[bufferLen];
IniFile    ini(filename);                      // object to access the config file

/*  Current */
uint8_t        currentMeasurePin = CURRENT_PIN; // pin to measure the current flow
boolean        currentIsFlowing  = false;       // to know if the previous state of the current measure
EnergyMonitor  emonInstance;                    // a emonLib instance to read current Values fron current CT sensor


/*Related with NTP server and UDP setup for TxRx */
unsigned long unixTime      = 0;               // variable to store the Unix NTP time retrieved form NTP server
byte          pb[NTP_PACKET_SIZE];             // buffer to hold incoming and outgoing packets 
int           numOfUnixTime = 0;               // used to know if the proccess of getting UnixTime fails more than this variable.
EthernetUDP   udp;                             // a UDP instance to let us send and receive packets over UDP


/*Energy counters*/
float eCoffees    = 0.0;     // energy Consumption making coffees
float ePeaks      = 0.0;     // energy Consumption by Peaks
float eStartTimes = 0.0;     // energy Consumption by Start Times
float auxEnergy   = 0.0;     // aux to obtain an Energy Consumption making coffees

/* Counters */
uint8_t nSTimes      = 0;           // number of Start Time peaks per day
uint8_t nCoffees     = 0;           // number of coffees per day
uint8_t nSByPeaks    = 0;           // number of stand by peaks per day
uint8_t nLoopPower   = 0;           // iteration variable

/* Consumption time working */
unsigned long timeOn            = 0;        // the time that the coffee machine was operating (One hot drink!)
unsigned long countStart        = 0;        // to count when a peak has started
unsigned long lastPeak          = 0;        // this variable is to store when the last peak has been triggered (aim: figure out what kind of peak is [peak, coffee or S. Time.]) 
unsigned long timeCount         = 0;        // the timestamp to store the begining of a peak, coffee or S. Time.
unsigned long totalTimeOn       = 0;        // the time that the coffee machine was operating (during the cicle of 24h)


/* Testers */
boolean isStartTime   = false;   // to know if the detected peak belong to [StartTime or Stand-by]
boolean prevWasCoffee = false;   // test if the last peak was a coffe (to distinguish between coffee and peak)
boolean wasOff        = true;    // test if the coffe maker was previously switch off.
boolean isStable      = false;   // variable to calibrate the current sensor. Wait time until stable.

/* NoSQL DataBase */
String response   = "";          // value returned by the server.
char   postData[400];            // buffer to store the data to send
char   consumptionSecsDB[10];    // to store the second consumming energy
char   consumptionTypeDB[15];    // to store the type of consumption of the coffee maker
char   organisationID[20];       // organization's abbreviation: e.g.  "UDEUSTO", "UPM", "UGENT", etc.
char   dateDB[50];               // to store the date when the peak was detected
char   timeDB[20];               // to store the time when the peak was detected
char   consumptionWhDB[10];      // to store the energy consumed by the peak detected

/* RFID tags */
char    tagValue[12];                          // to strore the RFID tag read 
char    tagValueAux[12];                       // to strore a copy of the the RFID tag read  
boolean cardDetected = false;                  // to detect if the mug has been detected or not
boolean cardInField  = MUG_IN_DEVICE_PIN;      // pin to sense when the coffee maker is placed on the appliance.

/* LEDs to know the status */
uint8_t ledPin       = STATUS_PIN; // pin for feedback


/**********
  SETUP
***********/
void setup() {
  
   //Important  Iboard Pro code to restart the Ethernet appropriately 
  pinMode(47,OUTPUT);  //RESET PIN
  digitalWrite(47, LOW);
  delay(500);
  digitalWrite(47, HIGH); 
  delay(500); 
  
  Serial1.begin(9600);
  //wdt_disable();         // Watch dog code to detect if arduino is blocked anytime
  
#if ECHO_TO_SERIAL  
  Serial.begin(9600); 
  Serial.println("Start");
#endif
   
  pinMode(SD_SELECT,       OUTPUT);
  pinMode(ETHERNET_SELECT, OUTPUT);
  pinMode(ledPin,          OUTPUT);
  pinMode(currentMeasurePin, INPUT);   // sets the analog pin as input (current measure throug the coffe machine plug -mains)*/   
  pinMode(cardInField,       INPUT);
  
  digitalWrite(ledPin,          LOW);
  digitalWrite(SD_SELECT,       HIGH);       // disable SD card  
  digitalWrite(ETHERNET_SELECT, HIGH);       // disable Ethernet 
  
  initializeConfigFile();
  setNetworkAddresses();
  getUnixTime();
  if(!rtcOnTime){
    setRTCTime();
  }
  
#if ECHO_TO_SERIAL  
  Serial.println(Ethernet.localIP()); 
  Serial.print("Time provided by the RTC and UTP: ");  
  getRTCtime();
#endif 

#if ECHO_TO_SERIAL  
   Serial.println("Working");
#endif
  
  emonInstance.current(currentMeasurePin, CALIBRATION);    // sets the Pin from and the calibration to read current Values.
  
  
  digitalWrite(ledPin, HIGH);    // to test if all goes Ok and the device is properly setup and ready to loop
  delay(2000);
  digitalWrite(ledPin, LOW);     
  delay(2000);
  digitalWrite(ledPin, HIGH);    
  delay(2000);
  digitalWrite(ledPin, LOW);     
  
}

/***********
  LOOP
************/
void loop() {
  
  RTC.getTime();
  if(dayOfWeek != RTC.dow){
    memset(dateDB, '\0', 50);
    strcpy(dateDB, printDate());
    memset(timeDB, '\0', 20);
    strcpy(timeDB, printTime());
    POSTrequest(organisationID, DEVICE_TYPE, dateDB, timeDB, "RESET", "0", "0", "-");
    delay(100);
    wdt_reset();
    delay(100);
    wdt_enable(WDTO_15MS);
    while(1);
  }
  //wdt_reset();                           // Watch dog code to detect if arduino is blocked anytime
  delay(25);
  float currentToMeasure = emonInstance.calcIrms(EMON_INSTANCE_VALUE);
  Serial.println(currentToMeasure);
  if(isStable == false && currentToMeasure < 0.40){  // wait until the current sensor is stable.
    isStable = true;
    delay(200);
  }
  if(isStable){
    controlCoffeMade(currentToMeasure);            // to read the the RMS current flow [0..30A]
    if(cardDetected && digitalRead(cardInField) == 0){
      delay(200);
      if(digitalRead(cardInField) == 0){
        cardDetected = false;
        if(strcmp(tagValue, tagValueAux) != 0){
          postCoffeeCup();
        }
        memset(tagValueAux, '\0', 12);
        strcpy(tagValueAux, tagValue);
        memset(tagValue, '\0', 12);
      }
    }
    if (Serial1.available() > 0) {
      rfidReadMug();
    }
  }
}
