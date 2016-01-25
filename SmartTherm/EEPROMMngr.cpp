#include "EEPROMMngr.h"

String EEPROMMngr::getWifiName(){
  unsigned char i = WIFI_ID_EEPROMADDR;
  String buf = "";
  boolean endFound = false;
  while (i<(WIFI_ID_EEPROMADDR+WIFI_ID_MAXLEN)){
    char b = EEPROM.read(i);
    if (b == '\0'){
      endFound = true;
      break;
    }else{
      buf+=b;
    }
    i++;
  }
  if (!endFound){
    buf = "";
  }
  return buf;
}

boolean EEPROMMngr::setWifiName(String sWifiName){
  if (sWifiName.length() > (WIFI_ID_MAXLEN-1)){
    return false;
  }
  unsigned char i = 0;
  while (i<(WIFI_ID_MAXLEN)){
    EEPROM.update((i+WIFI_ID_EEPROMADDR),sWifiName[i]);
    i++;  
  }
  EEPROM.update((i+WIFI_ID_EEPROMADDR),'\0');
  return true;
}

