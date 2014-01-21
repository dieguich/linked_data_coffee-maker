
/*** ***/
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

  if (found && (dhcpEnabled == 0 || dhcpEnabled == false)) {
#if ECHO_TO_SERIAL      
    Serial.println("**************DHCP is not enabled*******************");
#endif
    if (ini.getValue("static", "ip", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.myIP); 
      //IPAddress ip(myAddresses.myIP);  
      ip = myAddresses.myIP;
    }
    if (ini.getValue("static", "gateway", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.myGateway);
      gateway = myAddresses.myGateway;
    }
    if (ini.getValue("static", "netmask", bufferINIfile, bufferLen)) {
      copyToStruct(myAddresses.mySubnet);
      netmask = myAddresses.mySubnet;
    }
    if (ini.getValue("static", "dnsServer", bufferINIfile, bufferLen)) {
        //Serial.print("section 'static' has an entry 'dnsServer' with value ");
       copyToStruct(myAddresses.myDNS);
       dnsServer = myAddresses.myDNS;
    }
    Ethernet.begin(mac, ip, dnsServer, gateway, netmask);
  }
  else{
    Ethernet.begin(mac);
  }
  
}



/*****
CouchDB
*****/
void POSTrequest(char* deviceID, char* device_type, char* date, char* timeSecs, char* consumption_type, char* consumption_time_in_secs, char* energy_consumption_Wh, char* coffeeMugID){

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
    strcat(postData, timeSecs);    
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
    

    int statusCode = client.post("/", postData, &response);
    //int statusCode = 1;
    #if ECHO_TO_SERIAL  
      Serial.println(postData);
      Serial.print("Status code from server: ");
      Serial.println(statusCode);
      Serial.print("Response body from server: ");
      Serial.println(response);
    #endif
   
    memset(postData, '\0', 400);
    memset(bufferNoSQL, '\0', 30);
    response = "";  
}

