#include <avr/pgmspace.h>
#include <avr/wdt.h> 
#include <Udp.h>
#include <RTClib.h>
#include <IniFile.h>
#include <string.h>

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
  uint8_t myIP[4]      = {};
  uint8_t mySubnet[4]  = {};
  uint8_t myGateway[4] = {};
  uint8_t myDNS[4]     = {};
} NetAddresses;

byte mac[6] = {};
NetAddresses myAddresses;


/** Variables for accessing to INI file **/
const char *filename   = "/conf.ini";
const size_t bufferLen = BUFFER_LENGTH;
char bufferINIfile[bufferLen];

IniFile ini(filename);

/*  Current */
uint8_t        currentMeasurePin = A1;    // pin to measure the current flow
uint8_t        buttonState       = 0;

/* Time working */
unsigned long currentTime        = 0;        //  to store the current UnixTime in each loop iteration (base: UnixTime + time consumed)
boolean       isStable           = false;

/* Enter a MAC address and IP address for your controller below.
 The IP address will be dependent on your local network:*/
 
/****** TO FILL ******
 ****** ------- ******/
//byte mac[]     = {  };  // Type your MAC address inside the curly brackets as following : 0x90, 0xA2, 0xDA, 0x0E, 0x94, 0x69 

/****** TO FILL ******
 ****** ------- ******/
 

/*Related with NTP server and UDP setup for TxRx */
unsigned long UnixTime          = 0;           // variable to store the Unix NTP time retrieved form NTP server
unsigned long referenceUnixTime = 0;           // variable to store the reference Unix NTP time stored in EEPROM (0)

/****** TO FILL ******
 ****** ------- ******/
IPAddress timeServer(129, 6, 15, 28);            // By default Nist server. If you prefer a different UTP server (local or country) change it e.g.: 129, 6, 15, 28


byte pb[NTP_PACKET_SIZE];                      // buffer to hold incoming and outgoing packets 
EthernetUDP Udp;                               // A UDP instance to let us send and receive packets over UDP


EnergyMonitor emonInstance;                    // A emonLib instance to read current Values fron current CT sensor
RestClient client = RestClient(IP_DB_SERVER, PORT_DB_SERVER);  //Rest Client 

/*Energy counters*/
float eCoffees    = 0.0;     // energy Consumption making coffees
float ePeaks      = 0.0;     // energy Consumption by Peaks
float eStartTimes = 0.0;     // energy Consumption by Start Times

/* Counters */
uint8_t nSTimes      = 0;           // number of Start Time peaks per day
uint8_t nCoffees     = 0;           // number of coffees per day
uint8_t nSByPeaks    = 0;           // number of stand by peaks per day
uint8_t nLoopPower   = 0;           // iteration variable


/* Time working */
unsigned long referenceInMillis = 0;        // to store the time stamp when the Energy post was done
unsigned long timeOn            = 0;        // the time that the coffee machine was operating (One hot drink!)
unsigned int  nDays             = 0;        //  number of days the Arduino is active retrying data.


/* Testers */
boolean isStartTime   = false;   // to know if the detected peak belong to [StartTime or Stand-by]
boolean prevWasCoffee = false;   // test if the last peak was a coffe (to distinguish between coffee and peak)
boolean wasOff        = true;    // test if the coffe maker was previously switch off.
boolean isFirstDay    = true;    // variable to know if it's the first day

/*Energy counters*/
float auxEnergy   = 0.0;     // aux to obtain an Energy Consumption making coffees

/* Time working */
unsigned long countStart        = 0;        // to count when a peak has started

/* NoSQL DataBase */
String response   = "";
char   postData[400];
char   bufferNoSQL[30];
char   consumptionSecsDB[10]; 
char   consumptionTypeDB[20];

/* RFID tags */
char tagValue[10];
long timeRfidDetected = 0;

/* LEDs to know the status */
uint8_t readyPin = 7;
uint8_t postPin  = 3;

// Organization's abbreviation: e.g.  "UDEUSTO", "UPM", "UGENT", etc.
char organisationID[20];

/**********
  SETUP
***********/
void setup() {
   
  Serial3.begin(9600);
  wdt_disable(); 
  
#if ECHO_TO_SERIAL  
  Serial.begin(9600); 
  Serial.println("Start");
#endif
   
  /* Configure all of the SPI select pins as outputs and make SPI
   devices inactive, otherwise the earlier init routines may fail
   for devices which have not yet been configured.*/
  pinMode(SD_SELECT, OUTPUT);
  digitalWrite(SD_SELECT, HIGH);       // disable SD card
  pinMode(ETHERNET_SELECT, OUTPUT);
  digitalWrite(ETHERNET_SELECT, HIGH); // disable Ethernet 
  
  pinMode(readyPin, OUTPUT);
  pinMode(postPin,  OUTPUT);
  pinMode(currentMeasurePin, INPUT);   // sets the analog pin as input (current measure throug the coffe machine plug -mains)*/    
  digitalWrite(readyPin,  LOW);
  digitalWrite(postPin,   LOW);    
  
  initializeConfigFile();
  setNetworkAddresses();
  getUnixTime();
  
#if ECHO_TO_SERIAL  
  Serial.println(Ethernet.localIP()); 
  Serial.print("UnixTime provided by the UTP: ");  
  Serial.println(UnixTime);
  Serial.print("UnixTime converted in human readable time: ");    
  Serial.println(printDateTime(UnixTime));
#endif 

  referenceUnixTime = UnixTime;
  
#if ECHO_TO_SERIAL  
   Serial.println("Working");
#endif
  
  emonInstance.current(currentMeasurePin, CALIBRATION); // sets the Pin from and the calibration to read current Values.
  referenceInMillis = millis();                         // timeStamp to know when the current program start.
  wdt_enable(WDTO_8S);
  
  digitalWrite(readyPin, HIGH);
  while(1);
}

/***********
  LOOP
************/
void loop() {
   
  wdt_reset(); 
  currentTime = UnixTime + ((millis()-referenceInMillis)/1000);
  delay(25);
  
  float currentToMeasure = emonInstance.calcIrms(EMON_INSTANCE_VALUE);
  if (isStable == false && currentToMeasure < 0.1){
    isStable = true;
  }
  if (isStable){
   controlCoffeMade(currentToMeasure); // to read the the RMS current flow [0..30A]
   if (Serial3.available() > 0) {
     rfidReadMug();
   }
   else if(!prevWasCoffee && (millis() > timeRfidDetected + 120000)){
     memset(tagValue, '\0', 10);
   }
  }
}
