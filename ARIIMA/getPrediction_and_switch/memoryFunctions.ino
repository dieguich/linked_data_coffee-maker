void initializeConfigFile(){
  
  SPI.begin();
  if (!SD.begin(SD_SELECT)){
    while (1){
#if ECHO_TO_SERIAL 
      Serial.println("SD.begin() failed");
#endif
      digitalWrite(ledPin, HIGH);
    }
  }
  if (!ini.open()) {
#if ECHO_TO_SERIAL     
    Serial.print("Ini file ");
    Serial.print(filename);
    Serial.println(" does not exist");
#endif    
    digitalWrite(ledPin, HIGH);
    // Cannot do anything else
    while (1);
  }
#if ECHO_TO_SERIAL     
  Serial.println("Ini file exists");
#endif

// Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(bufferINIfile, bufferLen)) {
#if ECHO_TO_SERIAL         
    Serial.print("ini file ");
    Serial.print(ini.getFilename());
    Serial.print(" not valid: ");
#endif
    digitalWrite(ledPin, HIGH);
    while (1);  // Cannot do anything else
  }
  
  if (ini.getValue("organisation", "locationID", bufferINIfile, bufferLen)) { 
      strcpy(organisationID, bufferINIfile);
  }
  else{
    digitalWrite(ledPin, HIGH);
  }
}



void writePredictionsFile(){
  
  if (ini.open()) {
    ini.close();
    delay(1000);
  }
  if (SD.exists((char *)predictFileName)) {
    Serial.println("it exists");
    SD.remove((char *)predictFileName);
    delay(1000);
  }
  myFile = SD.open(predictFileName, FILE_WRITE);
  
  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to File...");
    
    printInfoInPrediFile();
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening File");
  }  
  readPredictionFile();
}

void readPredictionFile(){
  
  // re-open the file for reading:
  myFile = SD.open(predictFileName);
  if (myFile) {
    Serial.println("File:");
   
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
        Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening File");
  }
}


void printInfoInPrediFile(){
  char   bufferNoSQL[300];
  for(int i = 0; i < 3; i++){
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(pred_info[i])));
    //Serial.println(bufferNoSQL);
    myFile.println(bufferNoSQL);
  }
  //Serial.println(response.length());
  for(int i = 0; i<5; i++){
    int initial = i*12;
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(pred_info[i+3])));
    char charBuf[30];
    String respAux = response.substring(initial, initial+12);
    //Serial.println(respAux.length());
    respAux.toCharArray(charBuf, 30);
    //Serial.println(charBuf);
    strcat(bufferNoSQL, charBuf);
    myFile.println(bufferNoSQL);
    memset(bufferNoSQL, '\0', 300);
  }
  for(int i = 8; i < 10; i++){
    strcpy_P(bufferNoSQL, (char*)pgm_read_word(&(pred_info[i])));
    //Serial.println(bufferNoSQL);
    myFile.println(bufferNoSQL);
  }
#if ECHO_TO_SERIAL   
  Serial.println("done predictions");
#endif  
  memset(bufferNoSQL, '\0', 300);
}

/** Extract from SD the IP address to query for retrieving NTP date**/
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

