/** 
  write word to EEPROM 
**/
void EEPROM_writeint(int address, int value) {
  EEPROM.write(address,highByte(value));
  EEPROM.write(address+1 ,lowByte(value));
}

/**
  read double word from EEPROM, give starting address
**/
unsigned long EEPROM_readlong(int address) {
  //use word read function for reading upper part
  unsigned long dword = EEPROM_readint(address);
  //shift read word up
  dword = dword << 16;
  // read lower word from EEPROM and OR it into double word
  dword = dword | EEPROM_readint(address+2);
  return dword;
}

/**
  write long integer into EEPROM
**/
void EEPROM_writelong(int address, unsigned long value) {
  //truncate upper part and write lower part into EEPROM
  EEPROM_writeint(address+2, word(value));
  //shift upper part down
  value = value >> 16;
  //truncate and write
  EEPROM_writeint(address, word(value));
}

/**
  write an integer into EEPROM
**/
unsigned int EEPROM_readint(int address) {
  unsigned int word = word(EEPROM.read(address), EEPROM.read(address+1));
  return word;
} 


/**
  print a char * stored in Flash Memory (instead of SRAM) to the Serial output
**/
void prinFromMemorySerial(const char *str){
  
  char *mem = (char *)str;
  int i = 0;
  
  memset(consumptionTypeDB, '\0', 20);
  while (pgm_read_byte(mem) != 0x00){ /* Comparamos con \0, un terminador */
      consumptionTypeDB[i++] = (char)pgm_read_byte(mem++);
  }

  #if ECHO_TO_SERIAL         
    Serial.print(consumptionTypeDB);
    Serial.print('|');
  #endif
}

void initializeConfigFile(){
  
  SPI.begin();
  if (!SD.begin(SD_SELECT))
    while (1)
#if ECHO_TO_SERIAL 
      Serial.println("SD.begin() failed");
#endif
  if (!ini.open()) {
#if ECHO_TO_SERIAL     
    Serial.print("Ini file ");
    Serial.print(filename);
    Serial.println(" does not exist");
#endif    
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
    while (1);  // Cannot do anything else
  }
  
  if (ini.getValue("organisation", "locationID", bufferINIfile, bufferLen)) { 
      memcpy(organisationID, 
    }

}
