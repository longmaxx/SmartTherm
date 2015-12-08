#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H
#endif
class WebMngr{
  public:
  void (*dbgOutput)(String msg);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  
  void Setup_Hardware();
  boolean ListWifiAPs();
  bool ConnectWifi(String sNetName,String sPassword);
  bool SendPostRequest(String sUrl, String sBody);
  bool InternetAccess();
  boolean  wifiCmd(char cmd[], int timeout, char answer[]);
};

