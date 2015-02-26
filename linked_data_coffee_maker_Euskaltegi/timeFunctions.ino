
/** 
  This functions retrieves the UnixTime from a server specified in the SD config file.
  If it fails, the time is set form the RTC or ultimately set as 01/01/2000
**/
void getUnixTime(){
  
  int udpTimes = 0;
  
  udp.begin(UDP_PORT);
  IPAddress ntpToGetTime = getNTPAddress();
  //IPAddress ntpToGetTime(10,12,0,6);

  delay(100);
  while(!udp.available()) {
    sendNTPpacket(ntpToGetTime);  // send an NTP packet to a time server
    delay(1000);                // wait to see if a reply is available
    udpTimes++;
    if(udpTimes > 10)
      break;
  }
  if ( udp.parsePacket() ) {  
    udp.read(pb,NTP_PACKET_SIZE);  // read the packet into the buffer
    unixTime = getTimefromNTP(pb); //if the time provided by the UTP server is different to yours, please add or subtract the corresponding hours in secs, e.g. unixTime+=3600; 
    //unixTime+=3600; 
  }
  else
  {
    delay(5000);
    Serial.println("no-time obtained");
    numOfUnixTime++;
    if (numOfUnixTime > 2){
      RTC.getTime();
      if(RTC.year < 2014){
        unixTime = TIME30YEARS;
      }
      else{
        rtcOnTime = true;
        dayOfWeek = RTC.dow;
      }
      
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
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(pb,NTP_PACKET_SIZE);
  udp.endPacket(); 
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
  Returns the date in the format yyyy-mm-dd
**/
char* printDate()
{
    char datestr[30];
    RTC.getTime();
    
    sprintf(datestr, "%04d-%02d-%02d", RTC.year, RTC.month, RTC.day); 
    return datestr;
}

/**
  Returns the time in the format hh:mm:ss
**/
char* printTime()
{
    char datestr[8];
    RTC.getTime();
    
    sprintf(datestr, "%02d:%02d:%02d", RTC.hour, RTC.minute, RTC.second);  
    return datestr;
}

/**
This funtions updates the RTC clock time. 
In our case it is set from the unixtime retrieved from a NTP server.
**/
void setRTCTime(){
  
  RTC.setRAM(0, (uint8_t *)&startAddr, sizeof(uint16_t));// Store startAddr in NV-RAM address 0x08  
  timeIsSet = 0xffff;
  RTC.setRAM(54, (uint8_t *)&timeIsSet, sizeof(uint16_t));  
  RTC.getRAM(54, (uint8_t *)&timeIsSet, sizeof(uint16_t));
  if (timeIsSet != 0xaa55)
  {
    RTC.stopClock();
        
    RTC.fillByTime2000(unixTime-TIME30YEARS);
    
    RTC.setTime();
    timeIsSet = 0xaa55;
    RTC.setRAM(54, (uint8_t *)&timeIsSet, sizeof(uint16_t));
    RTC.startClock();
    RTC.getTime();
    dayOfWeek = RTC.dow;
    
  }
}

/**
This function shows the current date, time and day of the week retrieved from the RTC clock.
**/
void getRTCtime(){
  
  RTC.getTime();
  if (RTC.hour < 10)                    // correct hour if necessary
  {
    Serial.print("0");
    Serial.print(RTC.hour, DEC);
  } 
  else
  {
    Serial.print(RTC.hour, DEC);
  }
  Serial.print(":");
  if (RTC.minute < 10)                  // correct minute if necessary
  {
    Serial.print("0");
    Serial.print(RTC.minute, DEC);
  }
  else
  {
    Serial.print(RTC.minute, DEC);
  }
  Serial.print(":");
  if (RTC.second < 10)                  // correct second if necessary
  {
    Serial.print("0");
    Serial.print(RTC.second, DEC);
  }
  else
  {
    Serial.print(RTC.second, DEC);
  }
  Serial.print(" ");
  if (RTC.day < 10)                    // correct date if necessary
  {
    Serial.print("0");
    Serial.print(RTC.day, DEC);
  }
  else
  {
    Serial.print(RTC.day, DEC);
  }
  Serial.print("-");
  if (RTC.month < 10)                   // correct month if necessary
  {
    Serial.print("0");
    Serial.print(RTC.month, DEC);
  }
  else
  {
    Serial.print(RTC.month, DEC);
  }
  Serial.print("-");
  Serial.print(RTC.year, DEC);          // Year need not to be changed
  Serial.print(" ");
  switch (RTC.dow)                      // Friendly printout the weekday
  {
    case 1:
      Serial.print("MON");
      break;
    case 2:
      Serial.print("TUE");
      break;
    case 3:
      Serial.print("WED");
      break;
    case 4:
      Serial.print("THU");
      break;
    case 5:
      Serial.print("FRI");
      break;
    case 6:
      Serial.print("SAT");
      break;
    case 7:
      Serial.print("SUN");
      break;
  }
  Serial.println();
}
