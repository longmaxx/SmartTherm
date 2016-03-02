#include <Arduino.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H

class WebMngr{
  char sOK[3] = "OK";
  public:
  void (*dbgOutput)(String msg);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  void (*dbgOutputChr)(char msg[]);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  void (*dbgOutputCh)(char msg);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  
  void Setup_Hardware();
  //boolean ListWifiAPs();
  bool WifiAPConnected(String sAPName);
  bool ConnectWifi(String sNetName,String sPassword);
  bool SendGetRequest(String sUrl);
  //bool InternetAccess();
  boolean  ATCmd(String cmd, int timeout, char answer[]);

  private: bool WaitStrSerial(char strEtalon[],int timeout);
};
#endif
