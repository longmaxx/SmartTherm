#ifndef _EEPROMMNGR_H
#include <Arduino.h>
#include <EEPROM.h>
#define _EEPROMMNGR_H

class EEPROMMngr{
  #define WIFI_ID_EEPROMADDR (0)
  #define WIFI_ID_MAXLEN (20)
  
  #define WIFI_PWD_EEPROMADDR (WIFI_ID_EEPROMADDR + WIFI_ID_MAXLEN)
  #define WIFI_PWD_MAXLEN (20)
    
  #define DEVICE_NAME_EEPROMADDR (WIFI_PWD_EEPROMADDR + WIFI_PWD_MAXLEN)
  #define DEVICE_NAME_MAXLEN (10)

  public:
    String getWifiName();
    //String getWifiPwd();
    //String getDeviceName();
    //String getTermometerID();

    boolean setWifiName(String sWifiName);
    //boolean setWifiPwd(String sPwd);
    //boolean setDeviceName(String sDevName);
    //boolean setTermometerID(String ID);
};


#endif
