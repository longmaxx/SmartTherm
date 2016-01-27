#include "EEPROMMngr.h"

String EEPROMMngr::getWifiName(){
  return this->getStringValue(WIFI_ID_EEPROMADDR,WIFI_ID_MAXLEN);
}

boolean EEPROMMngr::setWifiName(String sWifiName){
  return this->setStringValue(sWifiName,WIFI_ID_EEPROMADDR,WIFI_ID_MAXLEN);
}

String EEPROMMngr::getWifiPwd(){
  return this->getStringValue(WIFI_PWD_EEPROMADDR,WIFI_PWD_MAXLEN);
}

boolean EEPROMMngr::setWifiPwd(String sWifiPwd){
  return this->setStringValue(sWifiPwd,WIFI_PWD_EEPROMADDR,WIFI_PWD_MAXLEN);
}

String EEPROMMngr::getDeviceName(){
  return this->getStringValue(DEVICE_NAME_EEPROMADDR,DEVICE_NAME_MAXLEN);
}

boolean EEPROMMngr::setDeviceName(String sName){
  return this->setStringValue(sName,DEVICE_NAME_EEPROMADDR,DEVICE_NAME_MAXLEN);
}


String EEPROMMngr::getStringValue(unsigned char baseAddr, unsigned char maxLen){
  unsigned char i = baseAddr;
  String buf = "";
  boolean endFound = false;
  while (i<(baseAddr+maxLen)){
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

boolean EEPROMMngr::setStringValue(String sValue,unsigned char baseAddr,unsigned char maxLen){
  if (sValue.length() > (maxLen-1)){
    return false;
  }
  unsigned char i = 0;
  while (i<(maxLen)){
    EEPROM.update((i+baseAddr),sValue[i]);
    i++;  
  }
  EEPROM.update((i+baseAddr),'\0');
  return true;
}

