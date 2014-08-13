

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
