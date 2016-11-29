#include <Arduino.h>
#include <EEPROM.h>
#ifndef _EEPROMMNGR_H
  #define _EEPROMMNGR_H

class EEPROMMngr{
  #define WIFI_ID_EEPROMADDR (0)
  #define WIFI_ID_MAXLEN (20)
  
  #define WIFI_PWD_EEPROMADDR (WIFI_ID_EEPROMADDR + WIFI_ID_MAXLEN)
  #define WIFI_PWD_MAXLEN (20)
    
  #define DEVICE_NAME_EEPROMADDR (WIFI_PWD_EEPROMADDR + WIFI_PWD_MAXLEN)
  #define DEVICE_NAME_MAXLEN (10)

  #define HOST_IP_EEPROMADDR (DEVICE_NAME_EEPROMADDR + DEVICE_NAME_MAXLEN)
  #define HOST_IP_MAXLEN (13)

  #define HOST_PORT_EEPROMADDR (HOST_IP_EEPROMADDR + HOST_IP_MAXLEN)
  #define HOST_PORT_MAXLEN (sizeof(int))

  
  public:
    String getWifiName();
    String getWifiPwd();
    String getDeviceName();
    //String getTermometerID();
    String getHostIP();
    int getHostPort();

    boolean setWifiName(String sWifiName);
    boolean setWifiPwd(String sPwd);
    boolean setDeviceName(String sDevName);
    //boolean setTermometerID(String ID);
    boolean setHostIP(String ip);
    boolean setHostPort(int port);
  private:  String  getStringValue(unsigned char baseAddr,unsigned char maxLen);
            boolean setStringValue(String sValue,unsigned char baseAddr,unsigned char maxLen)  ;
};


#endif
