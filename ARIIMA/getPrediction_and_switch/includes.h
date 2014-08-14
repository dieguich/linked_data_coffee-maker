/****** DEFINES *******/
#define NTP_PACKET_SIZE         48      // NTP time stamp is in the first 48 bytes of the message
#define UDP_PORT                8888    // local port to listen for UDP packets
#define TIME30YEARS             946684800 // To compute the current date and time when the RTC is updated.

#define SD_SELECT               4                 // The select pin used for the SD card
#define CS_MEGA                 53                // The CS pin in MEGa is 53 in UNO 10
#define BUFFER_LENGTH           80        // The length of the buffer to read from the SD

#define ETHERNET_SELECT         10                // The select pin used for the SD card
#define IP_DB_SERVER            "130.206.138.42"  //default IP server to store coffe maker's data
#define PORT_DB_SERVER          80
#define DEVICE_TYPE             "COFFEE-MAKER"    // the type of the eco-aware device
#define URL_PREDICTIONS         "/ecoserver/prediction/"

#define STATUS_PIN              13
#define ECHO_TO_SERIAL          1                 // echo data to serial port (DEBUG mode)

#define SWITCH_PIN              7
#define RELAY_PIN               2

const prog_char firstLine[]   PROGMEM = "#Preditions by day of the week";
const prog_char secondtLine[] PROGMEM = "; The data is filled every Monday 00:00:01 am";
const prog_char thirdtLine[]  PROGMEM = "[Predictions]";
const prog_char monLine[]     PROGMEM = "MON = ";
const prog_char tueLine[]     PROGMEM = "TUE = ";
const prog_char wedLine[]     PROGMEM = "WED = ";
const prog_char thuLine[]     PROGMEM = "THU = ";
const prog_char friLine[]     PROGMEM = "FRI = ";
const prog_char satLine[]     PROGMEM = "SAT = 000000000000";
const prog_char sunLine[]     PROGMEM = "SUN = 000000000000";

const char * const pred_info[] PROGMEM = 	 
{   
  firstLine,
  secondtLine,
  thirdtLine,
  monLine,  
  tueLine,
  wedLine,
  thuLine,
  friLine,
  satLine,
  sunLine
};

/************************************   
 * Specific Methods declared in this sketch
 ************************************/
 
 
