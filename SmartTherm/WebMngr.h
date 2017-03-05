#include <Arduino.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H

class WebMngr{
  char sOK[4] = {'\n','O','K','\0'};
  public: WebMngr( Stream &wifiSer,Stream &dbgSer);
  public:
      //boolean ListWifiAPs();
      bool WifiAPConnected(String sAPName);
      bool ConnectWifi(String sNetName,String sPassword);
      bool cmdConnectionOpenTCP(String serverIP, int port);
      bool cmdConnectionClose();
      bool cmdSendData(String data);
      //bool InternetAccess();
      boolean  ATCmd(String cmd, unsigned int timeout, char answer[]);
  private: 
      void flushTimeout();
      bool WaitStrSerial(char strEtalon[],int timeout);
      Stream& _dbgSerial;
      Stream& _wifiSerial;     
      void PrintMessage(String val);
      
};
#endif
