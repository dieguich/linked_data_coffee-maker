
/**
  calls the function to show and compute consumptions

**/

void accumulatedEnergyByPeak(){
  
  switch (setType()){
    case 1:{
      eCoffees += (auxEnergy/nLoopPower)*(timeOn/3600000.0);
  #if ECHO_TO_SERIAL                                             
      Serial.print("Consumed by Coffees: ");
      Serial.println(eCoffees, 2);
  #endif                    
      break;
    }
    case 2:{      
      eStartTimes += (auxEnergy/nLoopPower)*(timeOn/3600000.0);
  #if ECHO_TO_SERIAL                                                         
      Serial.print("Consumed by S. time: ");
      Serial.println(eStartTimes, 2);
  #endif            
      break;
    }
    case 3:{
      ePeaks += (auxEnergy/nLoopPower)*(timeOn/3600000.0);
  #if ECHO_TO_SERIAL                                                         
      Serial.print("Consumed by Peaks: ");
      Serial.println(ePeaks, 2);
  #endif            
      break;
    }
    //default: Serial.println("No type");
  }    
}



/**
  This function is many fold:
    * It figures out if the detected activation is a Stand By Peak, a Start Time or a Coffee.
    * Print this information in Serial output if DEBUG mode activated and to currentFile.
    * Before return the current Type, it prints the time implied in each of the detected activation.
**/
int setType(){

  uint8_t nType = 0;

  if(isStartTime){
    if(wasOff){
      wasOff = false;
      nSTimes++;
      prevWasCoffee = false;        
      nType = 2;           
      prinFromMemorySerial(sTime);         
    }
    else if(millis() < countStart){
      if(timeOn > 12000){    
        prinFromMemorySerial(coffee);
        nCoffees++;
        prevWasCoffee = true;
        isStartTime = false;
        nType = 1;
      }else{  
        prinFromMemorySerial(sTime);               
        nSTimes++;
        prevWasCoffee = false;        
        nType = 2;
      }
    }
    else{
      isStartTime = false;
      if(timeOn > 7000 && !prevWasCoffee){
        prinFromMemorySerial(coffee);
        prevWasCoffee = true;
        nCoffees++;
        nType = 1;         
      }else{      
        prinFromMemorySerial(peak);   
        nSByPeaks++;
        prevWasCoffee = false;        
        nType = 3;
      }
    }
  }
  else if(timeOn > 7000){  // if the operating time is bigger than 7secs is a coffee REMOVED:  && !prevWasCoffee
    prinFromMemorySerial(coffee);
    prevWasCoffee = true;    
    nCoffees++;
    nType = 1;
  }
  else{
    prinFromMemorySerial(peak);
    nSByPeaks++;
    prevWasCoffee = false;            
    nType = 3;
  }
  memset(consumptionSecsDB, '\0', 10);
  floatToString(consumptionSecsDB, (timeOn/1000.0), 2, 3);
#if ECHO_TO_SERIAL
  Serial.print(timeOn);
  Serial.print('|');    
#endif

 return nType;
}

/**
  Function to print error to the Serial output (if DEBUG mode is activated).
  Also sets a led to HIGH if an error has been triggered
**/
void error(char *str)
{
#if ECHO_TO_SERIAL    
  Serial.print("error: ");
  Serial.println(str);
#endif   
  delay(500);  
}

/**
  float to string conversion function that runs standalone
**/
char * floatToString(char * outstr, double val, byte precision, byte widthp){
  char temp[16];
  byte i;

  // compute the rounding factor and fractional multiplier
  double roundingFactor = 0.5;
  unsigned long mult = 1;
  for (i = 0; i < precision; i++)
  {
    roundingFactor /= 10.0;
    mult *= 10;
  }
  
  temp[0]='\0';
  outstr[0]='\0';

  if(val < 0.0){
    strcpy(outstr,"-\0");
    val = -val;
  }

  val += roundingFactor;

  strcat(outstr, itoa(int(val),temp,10));  //prints the int part
  if( precision > 0) {
    strcat(outstr, ".\0"); // print the decimal point
    unsigned long frac;
    unsigned long mult = 1;
    byte padding = precision -1;
    while(precision--)
      mult *=10;

    if(val >= 0)
      frac = (val - int(val)) * mult;
    else
      frac = (int(val)- val ) * mult;
    unsigned long frac1 = frac;

    while(frac1 /= 10)
      padding--;

    while(padding--)
      strcat(outstr,"0\0");

    strcat(outstr,itoa(frac,temp,10));
  }

  // generate space padding
  if ((widthp != 0)&&(widthp >= strlen(outstr))){
    byte J=0;
    J = widthp - strlen(outstr);
    
    for (i=0; i< J; i++) {
      temp[i] = ' ';
    }

    temp[i++] = '\0';
    strcat(temp,outstr);
    strcpy(outstr,temp);
  }
  
  return outstr;
} 

/*****
CouchDB
*****/
void POSTrequest(char* deviceID, char* device_type, char* date, char* timeSecs, char* consumption_type, char* consumption_time_in_secs, char* energy_consumption_Wh, char* coffeeMugID){

    memset(postData, '\0', 400);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[0])));
    strcat(postData, buffer);
    strcat(postData, deviceID);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[1])));
    strcat(postData, buffer);
    strcat(postData, device_type);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[2])));
    strcat(postData, buffer);
    strcat(postData, date);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[3])));
    strcat(postData, buffer);
    strcat(postData, timeSecs);    
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[4])));
    strcat(postData, buffer);
    strcat(postData, consumption_type);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[5])));
    strcat(postData, buffer);
    strcat(postData, consumption_time_in_secs);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[6])));
    strcat(postData, buffer);
    strcat(postData, energy_consumption_Wh);
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[7])));
    strcat(postData, buffer);
    strcat(postData, coffeeMugID);    
    strcpy_P(buffer, (char*)pgm_read_word(&(json_properties[8])));
    strcat(postData, buffer);
    

    int statusCode = client.post("/", postData, &response);
    #if ECHO_TO_SERIAL  
      Serial.println(postData);
      Serial.print("Status code from server: ");
      Serial.println(statusCode);
      Serial.print("Response body from server: ");
      Serial.println(response);
    #endif
   
    memset(postData, '\0', 400);
    memset(buffer, '\0', 30);
    response = "";  
}


/**
 To Read the tag RFID
**/

void rfidReadMug(){
  byte i = 0;
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesRead = 0;
  byte tempbyte = 0;
  
  if((val = Serial3.read()) == 2) {                  // check for header 
      memset(tagValue, '\0', 10);
      timeRfidDetected = millis();
      //Serial.println("RFID cleared");
      bytesRead = 0; 
      while (bytesRead < 12) {                        // read 10 digit code + 2 digit checksum
        if( Serial3.available() > 0) { 
          val = Serial3.read();
          if (bytesRead < 10)
          {
            tagValue[bytesRead] = val;
          }
          if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) { // if header or stop bytes before the 10 digit reading 
            break;                                    // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((val >= '0') && (val <= '9')) {
            val = val - '0';
          } else if ((val >= 'A') && (val <= 'F')) {
            val = 10 + val - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesRead & 1 == 1) {
            // make some space for this hex-digit by
            // shifting the previous hex-digit with 4 bits to the left:
            code[bytesRead >> 1] = (val | (tempbyte << 4));

            if (bytesRead >> 1 != 5) {                // If we're at the checksum byte,
              checksum ^= code[bytesRead >> 1];       // Calculate the checksum... (XOR)
            };
          } else {
            tempbyte = val;                           // Store the first hex digit first...
          };

          bytesRead++;                                // ready to read next digit
        } 
      } 

      // Output to Serial:

      if (bytesRead == 12) {           // if 12 digit read is complete
        tagValue[10] = '\0'; 
       
        #if ECHO_TO_SERIAL
        Serial.print("5-byte code: ");
        for (i=0; i<5; i++) {
          if (code[i] < 16) Serial.print("0");
          Serial.print(code[i], HEX);
          Serial.print(" ");
        }
        
          Serial.println();
  
          Serial.print("Checksum: ");
          Serial.print(code[5], HEX);
          Serial.println(code[5] == checksum ? " -- passed." : " -- error.");
          Serial.println();
          
            // Show the raw tag value
          Serial.print("VALUE_in_READING: ");
          Serial.println(tagValue);
        #endif
        
      }

      bytesRead = 0;
    }
}
