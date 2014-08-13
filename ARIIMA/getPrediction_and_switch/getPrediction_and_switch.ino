#include <avr/pgmspace.h>
#include <IniFile.h>
#include <RTClib.h>

#include <DS1307new.h>  //DS1307 Module for RTC
#include <Ethernet.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

#include "RestClient.h"
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
boolean  rtcOnTime = false;             // to know if the Arduino was previously set On time or not
uint16_t lastAddr;                      // new address for storing in NV-RAM
uint8_t  dayOfWeek;

/** Variables for accessing to INI file **/
const char *filename        = "/config.ini";   // the name of the config file. Stored in "/"
const      size_t bufferLen = BUFFER_LENGTH;
char       bufferINIfile[bufferLen];
IniFile    ini(filename);                      // object to access the config file

/** Variables for accessing to Precitions file **/
File myFile;
const char *predictFileName = "/predic.ini";   // the name of the config file. Stored in "/"


/* Variables related with NTP server and UDP setup for TxRx */
unsigned long unixTime      = 0;               // variable to store the Unix NTP time retrieved form NTP server
byte          pb[NTP_PACKET_SIZE];             // buffer to hold incoming and outgoing packets 
int           numOfUnixTime = 0;               // used to know if the proccess of getting UnixTime fails more than this variable.
EthernetUDP   udp;                             // a UDP instance to let us send and receive packets over UDP

/* Variables to get response from Predictions DataBase */
String response   = "";          // value returned by the server.

/* LEDs to know the status */
uint8_t ledPin       = STATUS_PIN; // pin for feedback

/* Info from ini file*/
char   organisationID[20];       // organization's abbreviation: e.g.  "UDEUSTO", "UPM", "UGENT", etc.
char   prediArray[20];       // organization's abbreviation: e.g.  "UDEUSTO", "UPM", "UGENT", etc.

/* variables to Control when switch On or OFF the appliance */
long      timeWithoutConsumption = 0;
boolean   isStandBy              = false;
uint8_t   hourCount              = 0;
uint8_t   hourNow                = 0;
const int switchPin              = SWITCH_PIN; // the number of the LED pin
const int relayPin               = RELAY_PIN;
int       relayActivated         = false;
int       inStandbyTime          = false;
int       switchState;                         // ledState used to set the LED
int       readSwitch;



//Setup
void setup() {
  /*Code to Reset the Ethernet uControler*/
  pinMode(47,OUTPUT);  //RESET PIN
  digitalWrite(47, LOW);
  delay(500);
  digitalWrite(47, HIGH); 
  delay(500);
  
 #if ECHO_TO_SERIAL  
  Serial.begin(9600); 
  Serial.println("Start");
 #endif
  
  pinMode(SD_SELECT,       OUTPUT);
  pinMode(ETHERNET_SELECT, OUTPUT);
  pinMode(ledPin,          OUTPUT);
  pinMode(CS_MEGA,         OUTPUT);
  pinMode(switchPin,        INPUT); 
  pinMode(relayPin,        OUTPUT);
  
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
  Serial.print("Time provided by the RTC and UTP: ");  
  getRTCtime();
#endif 
  delay(1000);
  
  if(RTC.dow == 1){            // Every Monday set a new week prediction
    setNewWeekPrediction();
    delay(1000);
  }
  readPredictionFile();
  getPredictionByDay(RTC.dow); // Get the prediction for the curremt day
  
}

//loop
void loop(){
  RTC.getTime();
  setApplianceState(); 
  
  switchState = digitalRead(switchPin);
  controlRelay();

}

