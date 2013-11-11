
int numOfUnixTime = 0;
/** 
  send an NTP request to the time server at the given address 
  NTP requests are to port 123
**/
void getUnixTime(){
  
  int udpTimes = 0;
  
  Udp.begin(UDP_PORT);
  
  while(!Udp.available()) {
    sendNTPpacket(timeServer);  // send an NTP packet to a time server
    delay(1000);                // wait to see if a reply is available
    udpTimes++;
    if(udpTimes > 10)
      break;
  }
  if ( Udp.parsePacket() ) {  
    Udp.read(pb,NTP_PACKET_SIZE);  // read the packet into the buffer
    UnixTime = getTimefromNTP(pb); //if the time provided by the UTP server is different to yours, please add or subtract the corresponding hours in secs, e.g. UnixTime+=3600; 
    //UnixTime+=3600; 
  }
  else
  {
    delay(5000);
    Serial.println("no-time obtained");
    numOfUnixTime++;
    if (numOfUnixTime > 2){
      UnixTime = referenceUnixTime - DAY_IN_SECONDS;
    }
    else{
      getUnixTime();
    }
  }
}


/** 
  send an NTP request to the time server at the given address 
  NTP requests are to port 123
**/

unsigned long sendNTPpacket(IPAddress& address)
{
  
  memset(pb, 0, NTP_PACKET_SIZE);  // Initialize values needed to form NTP request

  pb[0] = 0b11100011;   // LI, Version, Mode
  pb[1] = 0;            // Stratum, or type of clock
  pb[2] = 6;            // Polling Interval
  pb[3] = 0xEC;         // Peer Clock Precision
                        // 8 bytes of zero for Root Delay & Root Dispersion
  pb[12]  = 49; 
  pb[13]  = 0x4E;
  pb[14]  = 49;
  pb[15]  = 52;
 		    
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:         
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(pb,NTP_PACKET_SIZE);
  Udp.endPacket(); 
}


/** 
  get UTC time from NTP Server and conver it in epoch time
**/


unsigned long getTimefromNTP(byte* pb){
  
    // NTP contains four timestamps with an integer part and a fraction part
    // we only use the integer part here
    //unsigned long t1, t2, t3, t4;
    const unsigned long seventyYears = 2208988800UL;
    unsigned long t4 = 0;
    //t1 = t2 = t3 = t4 = 0;
    
    for (int i=0; i< 4; i++)
    {
      t4 = t4 << 8 | pb[40+i];
    }
    
    t4 -= seventyYears;

     // Adjust timezone and DST... in my case add two hours
    // or work in UTC?
    t4 += (1 * 3600L);     // Notice the L for long calculations!!
    t4 += 1;               // adjust the delay(1000) at begin of loop!
    
    return t4;
}

/**
  Returns the date and time in the format yyyy-mm-dd hh:mm:ss
**/
String printDateTime(DateTime t)
{
    char datestr[24];
    
    sprintf(datestr, "%04d-%02d-%02d  %02d:%02d:%02d  ", t.year(), t.month(), t.day(), t.hour(), t.minute(), t.second());  
    return datestr;
}

/**
  Returns the date in the format dataset/yyyymmdd.txt where dataset is the folder where the txt are stored and yyyymmdd is the
  date when the file was created
**/
char* printDate(DateTime t)
{
    char datestr[30];
    
    sprintf(datestr, "%04d-%02d-%02d", t.year(), t.month(), t.day()); 
    return datestr;
}

/**
  Returns the time in the format hh:mm:ss
**/
char* printTime(DateTime t)
{
    char datestr[8];
    
    sprintf(datestr, "%02d:%02d:%02d", t.hour(), t.minute(), t.second());  
    return datestr;
}


/**
  Function used to convert from the time when the program started to the first time to a relative to the current day 
  (in seconds: oo:oo:oo == 0, o1:oo:oo == 3600 ans so forth)
**/

unsigned int printRelativeUnixTime(unsigned long timeToConvert){
  return ((timeToConvert-referenceInMillis)/1000);
}
