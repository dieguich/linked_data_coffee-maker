
/** 
  This function controls when the coffee machine is operating. Not matter if is a peak, start time or coffee.
  Inside this, the function writes in to a remote DDBB the current measured values and the timestamp when those are sensed
  Finally it writes the type of peak detected and the time lasted reading such a peak.
**/
void controlCoffeMade(float readCurrentValue){  

  
  if((readCurrentValue > 0.5)){  // if the machine is working (hot water) and it was idle in the previous loop.
    if ((!currentIsFlowing)){ 
      timeCount = millis();      // to calculate the time used to make the current coffee.
      if(lastPeak == 0 || (timeCount-lastPeak)/1000 > 240){        
        isStartTime = true;
        wasOff      = true;        
        countStart  = timeCount + 120000;                   // during 120 seconds each peak that occurs will be considered as StartTime
      }
      lastPeak = millis();
      
      memset(dateDB, '\0', 50);
      strcpy(dateDB, printDate());
   #if ECHO_TO_SERIAL
      Serial.print(dateDB);
      Serial.print('|');
   #endif
      memset(timeDB, '\0', 20);
      strcpy(timeDB, printTime());
   #if ECHO_TO_SERIAL
      Serial.print(timeDB);      
      Serial.print('|');    
   #endif
      currentIsFlowing = true;   //The state of the machine shift to working
    }
    auxEnergy += (readCurrentValue*220);
    nLoopPower++;
  }
  
  
  if((readCurrentValue <= 0.1) && (currentIsFlowing)){  // if the machine is idle and it was working in the previous loop.  
      delay(2500);                                      // wait a second to know if is a false stop 
      if (emonInstance.calcIrms(EMON_INSTANCE_VALUE) <= 0.1){        
        currentIsFlowing = false;               // the state of the machine shift to idle
        timeOn = millis()-timeCount-2500;            // compute the time used to prepare a coffee (2secons to establish the current to 0 after a hot drink made) 
        setType();
        memset(consumptionWhDB, '\0', 10);
        floatToString(consumptionWhDB, ((auxEnergy/nLoopPower)*(timeOn/3600000.0)), 2, 3);        
       #if ECHO_TO_SERIAL                                 
          Serial.print(consumptionWhDB);    
          Serial.println("|");
       #endif
        
        if(prevWasCoffee){
          if(strlen(tagValue) > 10){
            tagValue[10] = '\0';
          }   
          POSTrequest(organisationID, DEVICE_TYPE, dateDB, timeDB, consumptionTypeDB, consumptionSecsDB, consumptionWhDB, tagValue);
        #if ECHO_TO_SERIAL                                 
          Serial.print("send: ");
          Serial.println(tagValue);          
        #endif
        }
        else{
        #if ECHO_TO_SERIAL                                 
          Serial.print("send: ");
          Serial.println("-");
        #endif
          POSTrequest(organisationID, DEVICE_TYPE, dateDB, timeDB, consumptionTypeDB, consumptionSecsDB, consumptionWhDB, "-");
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
