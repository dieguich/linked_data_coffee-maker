
/**
**/
void setApplianceState(){
  if(RTC.hour >= 7 && RTC.hour <= 20){
    if(RTC.hour == 7 && hourNow != 7){
      hourNow = RTC.hour;
      isStandBy = (prediArray[hourCount] == '1') ? true : false;
      hourCount++;
      Serial.println("Son las 7");
      Serial.print("La prediccion es");
      Serial.println(isStandBy); 
    }
    if(RTC.hour != hourNow){
      hourNow = RTC.hour;
      isStandBy = (prediArray[hourCount] == '1') ? true : false;
      hourCount++;           
      Serial.print("Hemos cambiado de hora son las: ");
      Serial.println(hourNow);      
      Serial.print("La prediccion es");
      Serial.println(isStandBy);       
    }
    if(RTC.hour == 20 && hourCount != 0){
      hourCount = 0;
      hourNow   = 0;
      isStandBy = false;
      Serial.println("Son las 20h");
      Serial.print("La prediccion es");
      Serial.println(isStandBy); 
    }
  }
}


/**
**/
void setNewWeekPrediction(){
  char urlGET[50] = URL_PREDICTIONS;
  
  strcat(urlGET, organisationID);
  int statusCode = client.get(urlGET, &response);
  Serial.print("Status code from server: ");
  Serial.println(statusCode);
  Serial.print("Response body from server: ");
  Serial.println(response);
  writePredictionsFile();
}



/**
**/
void getPredictionByDay(uint8_t dayOfWeek){
  
  IniFile    ini(predictFileName);
  char   buffAux[300];
  
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
  
  if (ini.getValue("Predictions", getDayOfWeek(dayOfWeek), bufferINIfile, bufferLen)) { 
      strcpy(prediArray, bufferINIfile);
      Serial.println(prediArray);
  }
  else{
    digitalWrite(ledPin, HIGH);
  }
}
