 º/****** DEFINES *******/
#define NTP_PACKET_SIZE         48      // NTP time stamp is in the first 48 bytes of the message
#define UDP_PORT                8888    // local port to listen for UDP packets
#define CALIBRATION             20.5
#define EMON_INSTANCE_VALUE     1000

#define SD_SELECT               4         // The select pin used for the SD card
#define ETHERNET_SELECT         10        // The select pin used for the SD card
#define BUFFER_LENGTH           80        // The length of the buffer to read from the SD
#define TIME30YEARS             946684800 // To compute the current date and time when the RTC is updated.

#define DEVICE_TYPE             "COFFEE-MAKER"    // the type of the eco-aware device
#define IP_DB_SERVER            "130.206.138.42"  //default IP server to store coffe maker's data
#define PORT_DB_SERVER          80

#define STATUS_PIN              13
#define MUG_IN_DEVICE_PIN       41
#define CURRENT_PIN             A15

#define ECHO_TO_SERIAL          0                 // echo data to serial port (DEBUG mode)


/* Variables to store in the Flash memory (save SRAM space): PROGMEM */
const char coffee[]          = "COFFEE";
const char peak[]            = "STANDBY";
const char sTime[]           = "START_TIME";

/* Variables to store in the NoSQL Database */

const prog_char deviceID[] PROGMEM    = "{\"deviceID\":\"";
const prog_char device_type[] PROGMEM = "\",\"device_type\":\"";
const prog_char date[] PROGMEM = "\",\"date\":\"";
const prog_char time[] PROGMEM = "\",\"time\":\"";
const prog_char consumption_type[] PROGMEM         = "\",\"consumption_type\":\"";
const prog_char consumption_time_in_secs[] PROGMEM = "\",\"consumption_time_in_secs\":";
const prog_char energy_consumption_Wh[] PROGMEM    = ",\"energy_consumption_Wh\":";
const prog_char user_ID[] PROGMEM = ",\"userID\":\"";
const prog_char endTerm[] PROGMEM = "\"}";


const char * const json_properties[] PROGMEM = 	 
{   
  deviceID,
  device_type,
  date,
  time,  
  consumption_type,
  consumption_time_in_secs,
  energy_consumption_Wh,
  user_ID,
  endTerm
};

/************************************   
 * Specific Methods declared in this sketch
 ************************************/
void          controlCoffeMade();               
void          accumulatedEnergyByPeak();        
void          setNetworkAddresses();
void          initializeConfigFile();
void          setRTCTime();
void          getRTCtime();
void          rfidReadMug();
void          copyToStruct(uint8_t* );
void          prinType(char *);
void          POSTrequest(char* , char* , char* , char* , char* , char* , char* , char* );

int           setType();

unsigned long getUnixTime(EthernetUDP udp);
unsigned long sendNTPpacket(byte *);   
unsigned long getTimefromNTP(byte* );

char*         printDate();
char*         printTime();
char*         subStr(char* , char* , int );
char*         floatToString(char* , double , byte , byte);
