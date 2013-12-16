
/*  Current */
boolean        currentIsFlowing  = false; // to know if the previous state of the current measure

/* Time working */
unsigned long lastPeak          = 0;        // this variable is to store when the last peak has been triggered (aim: figure out what kind of peak is [peak, coffee or S. Time.]) 
unsigned long timeCount         = 0;        // the timestamp to store the begining of a peak, coffee or S. Time.
unsigned long totalTimeOn       = 0;        // the time that the coffee machine was operating (during the cicle of 24h)

/* Vars to store info in the DDBB*/
char  dateDB[50];
char  timeDB[20];
char  consumptionWhDB[10];

/** 
  This function controls when the coffee machine is operating. Not matter if is a peak, start time or coffee.
  Inside this, the function writes in the SD the current measured values and the relative time-stamp when those are sensed
  Finally it writes the type of peak detected and the time spend reading such a peak.
**/
void controlCoffeMade(float readCurrentValue){  

  // if the machine is working (hot water) and it was idle in the previous loop.
  if((readCurrentValue > 0.2)){
    if ((!currentIsFlowing)){ 
      timeCount = millis();  // to calculate the time used to make the current coffee.
      if(lastPeak == 0 || (timeCount-lastPeak)/1000 > 240){        
        isStartTime = true;
        wasOff      = true;        
        countStart  = timeCount + 120000;                   // during 120 seconds each peak that occurs will be considered as StartTime
      }
      lastPeak = millis();
      
      memset(dateDB, '\0', 50);
      strcpy(dateDB, printDate(referenceUnixTime));
   #if ECHO_TO_SERIAL
      Serial.print(dateDB);
      Serial.print('|');
   #endif
      memset(timeDB, '\0', 20); //before was 10
      strcpy(timeDB, (char *)printTime(referenceUnixTime+((millis()-referenceInMillis)/1000)));
   #if ECHO_TO_SERIAL
      Serial.print(timeDB);      // to know the current day relative timeStamp. [0..86400]
      Serial.print('|');    
   #endif
      currentIsFlowing = true; //The state of the machine shift to working
    }
    auxEnergy += (readCurrentValue*220);
    nLoopPower++;
  }
  
  // if the machine is idle and it was working in the previous loop.  
  if((readCurrentValue <= 0.12) && (currentIsFlowing)){ 
      delay(1000);                     // wait a second to know if is a false stop 
      if (emonInstance.calcIrms(EMON_INSTANCE_VALUE) <= 0.12){        
        currentIsFlowing = false;               // the state of the machine shift to idle
        timeOn = millis()-timeCount;        // compute the time used to prepare a coffee (2secons to establish the current to 0 after a hot drink made) 
        totalTimeOn  += timeOn;                 // sumatory of partial times. It will be used afterwards to sumarize the whole energy consumption during a day
        setType();
        memset(consumptionWhDB, '\0', 10);
        floatToString(consumptionWhDB, ((auxEnergy/nLoopPower)*(timeOn/3600000.0)), 2, 3);        
       #if ECHO_TO_SERIAL                                 
          Serial.print(consumptionWhDB);    
          Serial.println("|");
       #endif
        if(prevWasCoffee){ 
          digitalWrite(readyPin, LOW);
          digitalWrite(postPin,  HIGH);
          delay(2000);
          digitalWrite(readyPin, HIGH);
          digitalWrite(postPin,  LOW);          
          
          POSTrequest(ID, DEVICE_TYPE, dateDB, timeDB, consumptionTypeDB, consumptionSecsDB, consumptionWhDB, tagValue);
        }
        else{
          digitalWrite(readyPin, LOW);
          digitalWrite(postPin,  HIGH);
          delay(2000);
          digitalWrite(readyPin, HIGH);
          digitalWrite(postPin,  LOW);                    
          POSTrequest(ID, DEVICE_TYPE, dateDB, timeDB, consumptionTypeDB, consumptionSecsDB, consumptionWhDB, "-");
        }
        auxEnergy = 0.0;              
        timeOn  = timeCount = nLoopPower = 0;
      }
      else{
        currentIsFlowing = true;       // it was not a coffe (fake)
      } 
  }     
  delay(100);
}
