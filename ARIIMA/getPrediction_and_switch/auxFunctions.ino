/**
  Funtion to split a string into data buffers depending of the token passed from value (separator)
**/
char* subStr(char* input_string, char *separator, int segment_number) {
  
  char *act, *sub, *ptr;
  static char copy[20];
  int i;
  
  strcpy(copy, input_string);
  for (i = 1, act = copy; i <= segment_number; i++, act = NULL) {
    
    sub = strtok_r(act, separator, &ptr);
    if (sub == NULL) break;
  }
  return sub;
}


/**
 Function to retrieve data from the config file stored in the SD card to a data structure.
**/
void copyToStruct(uint8_t* IPaddss){

  uint8_t bufTemp[4]; 
  for (int x = 1; x <= 4; x++) {
    bufTemp[x] = strtoul(subStr(bufferINIfile, ".", x),NULL,0);
    IPaddss[x-1] = bufTemp[x];
#if ECHO_TO_SERIAL     
    Serial.print(IPaddss[x-1]);
    Serial.print(".");
#endif
  }
#if ECHO_TO_SERIAL       
  Serial.println();
#endif
        
}


/**
**/
void controlRelay(){
    if (readSwitch != switchState){
    switchState = readSwitch;
    if (!relayActivated){
      //Activate the relay
      digitalWrite(relayPin, HIGH);
      relayActivated         = true;
      timeWithoutConsumption = millis();
      Serial.println("I am going to start running.");
    }
    Serial.println("State changed.");
    delay(100);
  }
  
  if (relayActivated && !isStandBy){
    if((millis()-timeWithoutConsumption) > 18000){
      digitalWrite(relayPin, LOW); 
      relayActivated         = false;
      timeWithoutConsumption = 0;    
    }
  }
}
