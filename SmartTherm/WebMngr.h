#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H
#endif
class WebMngr{
  public:
  void (*dbgOutput)(char msg[]);
  
  void Setup_Hardware();
//  String ListWifiNets();
  bool ConnectWifi(String sNetName,String sPassword);
  bool SendPostRequest(String sUrl, String sBody);
//  bool IsWiFiConnected();
  bool internetAccess();
  boolean  wifiCmd(char cmd[], int timeout, char answer[]);
};

