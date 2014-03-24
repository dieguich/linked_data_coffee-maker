

/** **/
IPAddress getNTPAddress(){
  
  IPAddress ntpAddress;
  uint8_t   ntpTimeServer[4];
  
  memset(bufferINIfile, '\0', strlen(bufferINIfile));
#if ECHO_TO_SERIAL         
   Serial.print("NTP: ");  
#endif
  if (ini.getValue("NTPserver", "ntpIP", bufferINIfile, bufferLen)) {
      copyToStruct(ntpTimeServer); 
  }
#if ECHO_TO_SERIAL       
    Serial.println();
#endif 
  ntpAddress =  ntpTimeServer;
  return ntpAddress;
}


/*** 
This function allows to setup the configuration of the network. 
MAC is always retrieved and the other IPs are gotten if dhcp service is not enabled.
***/
void setNetworkAddresses(){
  
  bool dhcpEnabled;                    // variable where result of the data searched will be stored
  bool found = ini.getValue("network", "dhcpEnabled", bufferINIfile, bufferLen, dhcpEnabled); 
  
  IPAddress ip;
  IPAddress gateway;
  IPAddress netmask;
  IPAddress dnsServer;
   
  if (ini.getValue("network", "mac", bufferINIfile, bufferLen)) {
#if ECHO_TO_SERIAL       
    Serial.print("MAC address: ");   
#endif    
    for (int x = 1; x <= 6; x++) {
      mac[x] = strtoul(subStr(bufferINIfile, ":", x),0,16);
#if ECHO_TO_SERIAL         
      Serial.print(mac[x]);
      Serial.print(":");
#endif      
    }
#if ECHO_TO_SERIAL       
    Serial.println();
#endif     
  }
  else{
     digitalWrite(ledPin, HIGH);
  }

  if (found && (dhcpEnabled == 0 || dhcpEnabled == false)) {
#if ECHO_TO_SERIAL      
    Serial.println("**************DHCP is not enabled*******************");
#endif
    if (ini.getValue("static", "ip", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.myIP); 
      //IPAddress ip(myAddresses.myIP);  
      ip = myAddresses.myIP;
    }else{
       digitalWrite(ledPin, HIGH);
    }
    if (ini.getValue("static", "gateway", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.myGateway);
      gateway = myAddresses.myGateway;
    }else{
       digitalWrite(ledPin, HIGH);
    }
    if (ini.getValue("static", "netmask", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.mySubnet);
      netmask = myAddresses.mySubnet;
    }else{
       digitalWrite(ledPin, HIGH);
    }
    if (ini.getValue("static", "dnsServer", bufferINIfile, bufferLen)) {
        //Serial.print("section 'static' has an entry 'dnsServer' with value ");
       copyToStruct(myAddresses.myDNS);
       dnsServer = myAddresses.myDNS;
    }else{
       digitalWrite(ledPin, HIGH);
    }
    Ethernet.begin(mac, ip, dnsServer, gateway, netmask);
  }
  else{
    if(Ethernet.begin(mac)==0){
    #if ECHO_TO_SERIAL       
      Serial.println("Failed to configure Ethernet using DHCP");
    #endif      
    }
    else{
    #if ECHO_TO_SERIAL       
      Serial.println("Ethernet started");
    #endif     
    }
  } 
}



/*****
Function which is used to do a POST request to a remote server where the JSON sent is stored. The data that are passed to the function
correspond with the energy consumption associated with each peak.
*****/
void POSTrequest(char* deviceID, char* device_type, char* date, char* time, char* consumption_type, char* consumption_time_in_secs, char* energy_consumption_Wh, char* coffeeMugID){

    char   bufferNoSQL[30];       // temporal buffer to store the data form memory   
    
    memset(postData, '\0', 400);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[0])));
    strcat(postData, bufferNoSQL);
    strcat(postData, deviceID);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[1])));
    strcat(postData, bufferNoSQL);
    strcat(postData, device_type);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[2])));
    strcat(postData, bufferNoSQL);
    strcat(postData, date);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[3])));
    strcat(postData, bufferNoSQL);
    strcat(postData, time);    
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[4])));
    strcat(postData, bufferNoSQL);
    strcat(postData, consumption_type);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[5])));
    strcat(postData, bufferNoSQL);
    strcat(postData, consumption_time_in_secs);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[6])));
    strcat(postData, bufferNoSQL);
    strcat(postData, energy_consumption_Wh);
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[7])));
    strcat(postData, bufferNoSQL);
    strcat(postData, coffeeMugID);    
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(json_properties[8])));
    strcat(postData, bufferNoSQL);
    

    int statusCode = client.post("/ecoserver", postData, &response);
    //int statusCode = 1;
    #if ECHO_TO_SERIAL  
      Serial.println(postData);
      Serial.print("Status code from server: ");
      Serial.println(statusCode);
      Serial.print("Response body from server: ");
      Serial.println(response);
    #endif
   
    memset(postData, '\0',    400);
    memset(bufferNoSQL, '\0', 30);
    response = "";  
}

