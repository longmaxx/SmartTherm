#include <Arduino.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H

class WebMngr{
  char sOK[3] = {'O','K','\0'};
  public: WebMngr( Stream &wifiSer,Stream &dbgSer);
  public:
      void Setup_Hardware();
      //boolean ListWifiAPs();
      bool WifiAPConnected(String sAPName);
      bool ConnectWifi(String sNetName,String sPassword);
      bool SendGetRequest(String &sUrl);
      //bool InternetAccess();
      boolean  ATCmd(String cmd, int timeout, char answer[]);
  private: 
      bool WaitStrSerial(char strEtalon[],int timeout);
      Stream& _dbgSerial;
      Stream& _wifiSerial;     
      void PrintMessage(String val);
      //void PrintMessage(char val);
      //void PrintMessage(char val[]);
  
};
#endif
