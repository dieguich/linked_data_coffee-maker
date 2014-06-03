
/**
 This function initializes the SD to read from the config file. Further, it reads from the file the organisation ID-
**/
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
