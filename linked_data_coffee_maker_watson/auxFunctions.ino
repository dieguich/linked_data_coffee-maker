
/**
  The function shows and computes energy consumptions by each of the peak detected.
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
      printType(sTime);         
    }
    else if(millis() < countStart){
      if(timeOn > 10000){ 
        printType(coffee);        
        nCoffees++;
        prevWasCoffee = true;
        isStartTime = false;
        nType = 1;
      }else{  
        printType(sTime);               
        nSTimes++;
        prevWasCoffee = false;        
        nType = 2;
      }
    }
    else{
      isStartTime = false;
      if(timeOn > 3000 && !prevWasCoffee){   
        printType(coffee);        
        prevWasCoffee = true;
        nCoffees++;
        nType = 1;         
      }else{      
        printType(peak);   
        nSByPeaks++;
        prevWasCoffee = false;        
        nType = 3;
      }
    }
  }
  else if(timeOn > 3000){  // if the operating time is bigger than 7secs is a coffee REMOVED:  && !prevWasCoffee
    printType(coffee);
    prevWasCoffee = true;    
    nCoffees++;
    nType = 1;
  }
  else{
    printType(peak);
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



/**
 To Read the tag RFID. Set the value for storing in the DDBB
**/
void rfidReadMug(){
 
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesRead = 0;
  byte tempbyte = 0;
  
  char * tagValueTemp;
  
  if((val = Serial1.read()) == 2) {                  // check for header 
      //Serial.println("DETECTED");
      memset(tagValue, '\0', 12);
      tagValueTemp = (char*) malloc(10);
      //Serial.println("RFID cleared");
      bytesRead = 0; 
      while (bytesRead < 12) {                       // read 10 digit code + 2 digit checksum
        if( Serial1.available() > 0) { 
          val = Serial1.read();
          if (bytesRead < 10)
          {
            tagValueTemp[bytesRead] = val;
          }
          if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) {  // if header or stop bytes before the 10 digit reading 
            break;                                                          // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((val >= '0') && (val <= '9')) {
            val = val - '0';
          } else if ((val >= 'A') && (val <= 'F')) {
            val = 10 + val - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesRead & 1 == 1) {
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

      if (bytesRead == 12) {                          // if 12 digit read is complete
        tagValueTemp[10] = '\0'; 
       
      #if ECHO_TO_SERIAL
        Serial.print("VALUE_in_READING: ");
        Serial.println(tagValueTemp);
      #endif
        strcpy(tagValue, tagValueTemp);
        //Serial.print("VALUE_in_READING: ");
        //Serial.println(tagValueTemp);
        free(tagValueTemp);
        cardDetected = true;
      }
      bytesRead = 0;
    }
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
  print a char * stored in Flash Memory (instead of SRAM) to the Serial output
**/
void printType(const char *str){
 
  memset(consumptionTypeDB, '\0', 12);
  strcpy(consumptionTypeDB, str);

  #if ECHO_TO_SERIAL         
    Serial.print(consumptionTypeDB);
    Serial.print('|');
  #endif
}




