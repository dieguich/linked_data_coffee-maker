
/****** DEFINES *******/
#define DAY_IN_SECONDS          86400   // seconds in s 24h day 
#define NTP_PACKET_SIZE         48      // NTP time stamp is in the first 48 bytes of the message
#define UDP_PORT                8888    // local port to listen for UDP packets
#define CALIBRATION             20.5
#define EMON_INSTANCE_VALUE     1000

/****** TO FILL ******
 ****** ------- ******/
#define ID                      ""   // Please, fill it with your organization's abbreviation: e.g.  "UDEUSTO", "UPM", "UGENT", etc.

#define DEVICE_TYPE             "COFFEE-MAKER"
#define ECHO_TO_SERIAL          1                 // echo data to serial port (DEBUG mode)
#define STATIC_IP_ADDRESS       1                 // Only if ethernet address is not DHCP enabled
#define IP_DB_SERVER            "130.206.138.42"  //default IP server to store coffe maker's data
#define PORT_DB_SERVER          57080


/* Variables to store in the Flash memory (save SRAM space): PROGMEM */
const char coffee[]        PROGMEM  = "COFFEE";
const char peak[]          PROGMEM  = "STANDBY";
const char sTime[]         PROGMEM  = "START_TIME";

/* Variables to store in the NoSQL Database */

prog_char deviceID[] PROGMEM = "{\"deviceID\":\"";
prog_char device_type[] PROGMEM = "\",\"device_type\":\"";
prog_char date[] PROGMEM = "\",\"date\":\"";
prog_char timeSecs[] PROGMEM = "\",\"time_secs\":\"";
prog_char consumption_type[] PROGMEM = "\",\"consumption_type\":\"";
prog_char consumption_time_in_secs[] PROGMEM = "\",\"consumption_time_in_secs\":";
prog_char energy_consumption_Wh[] PROGMEM = ",\"energy_consumption_Wh\":";
prog_char user_ID[] PROGMEM = ",\"userID\":\"";
prog_char endTerm[] PROGMEM = "\"}";


PROGMEM const char *json_properties[] = 	 
{   
  deviceID,
  device_type,
  date,
  timeSecs,  
  consumption_type,
  consumption_time_in_secs,
  energy_consumption_Wh,
  user_ID,
  endTerm
};



/************************************   
 * Specific Methods declared in this sketch
 ************************************/
void          controlCoffeMade();               // algorithm to measure the coffee machine current and coffe counter
void          accumulatedEnergyByPeak();        // calls the function to show and compute consumptions
//void          startEthernet(byte *mac);

int           setType();
void          error(char *);

unsigned long getUnixTime(EthernetUDP udp);
unsigned long sendNTPpacket(byte *);   
unsigned long getTimefromNTP(byte* );
unsigned int  printRelativeUnixTime(unsigned long);
String        printDateTime(DateTime);
char*         printDate(DateTime);
char*        printTime(DateTime);


void          prinFromMemorySerial(char *);
void          EEPROM_writelong(int , unsigned long );
void          EEPROM_writeint(int , int );
unsigned int  EEPROM_readint(int );
unsigned long EEPROM_readlong(int);

void          computeEnergyConsumed();          // algorithm to measure energy consumption and coffees made during a work day.

