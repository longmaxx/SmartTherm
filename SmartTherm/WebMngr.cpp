#include "WebMngr.h"
#include <avr/wdt.h>
WebMngr::WebMngr(Stream &wifiSer,Stream &dbgSer): _wifiSerial(wifiSer),_dbgSerial(dbgSer)
{
  flushTimeout();
 // _wifiSerial.setTimeout(5000);
  //ATCmd("ATE0",1000,sOK);
}

void WebMngr::flushTimeout()
{
  _wifiSerial.setTimeout(2000);
}

bool WebMngr::WifiAPConnected(String sAPName)
{
  _wifiSerial.flush();
  _wifiSerial.println(F("AT+CWJAP_CUR?"));
  flushTimeout();
  if (_wifiSerial.find("\n+CWJAP_CUR:\"")){
    String curAPName = _wifiSerial.readStringUntil('"');
    if (curAPName != sAPName){
      PrintMessage("WifiAPConnected: AP is wrong:"+curAPName);
      return true;  
    }
  }else{
    PrintMessage(F("WifiAPConnected: NoConnect"));
    return false;
  }
  PrintMessage(F("WifiAPConnected: AP is ok"));
  return true;
}

bool WebMngr::ConnectWifi(String sNetName,String sPassword)
{
  
  if (!WifiAPConnected(sNetName)){
    ATCmd(F("AT+CWQAP"),4000,sOK);//disconnect from any AP
    ATCmd(F("AT+CWMODE_CUR=1"),3000,sOK);
    _wifiSerial.flush();
    _wifiSerial.print(F("AT+CWJAP_CUR=\""));
    _wifiSerial.print(sNetName);
    _wifiSerial.print(F("\",\""));
    _wifiSerial.print(sPassword);
    //_wifiSerial.println("\"");
     bool res = ATCmd(F("\""),20000,sOK);
     _wifiSerial.println ("ConnectWifi: "+(String)res);
     return res;
  }
  return true;  
}

//bool WebMngr::InternetAccess()
//{
//  if(ATCmd(F("AT+PING=\"ya.ru\""),5000,sOK)){
//    PrintMessage(F("OK, Ping internet."));
//    return true;
//  }else{
//    PrintMessage(F("Fail ping internet."));
//    return false;
//  }
//}

//boolean WebMngr::ListWifiAPs(){
//  return ATCmd(F("AT+CWLAP"),5000,sOK);
//}

boolean  WebMngr::ATCmd(String cmd, int timeout, char answer[])
{
  _wifiSerial.flush();
  _wifiSerial.println(cmd);
  flushTimeout();
  int nEndTime = millis()+timeout;
  bool res = false;
  while ((millis() < nEndTime) && (!res)){
    wdt_reset();
    res = _wifiSerial.find(answer);
  }
  return res;
  //int timeoutOld = _wifiSerial.getTimeout();
  //return _wifiSerial.find(answer);
}

bool WebMngr::cmdSendData(String data)
{
  bool res= false;
  int len = data.length();
  _wifiSerial.flush();
  _wifiSerial.print(F("AT+CIPSEND="));
  _wifiSerial.println(len);

  if(WaitStrSerial(">",2000)) {
    _wifiSerial.print(data);
    res = WaitStrSerial("SEND OK",500);
  }
  delay(300);// fix busy p... errors
  return res;
}

bool WebMngr::cmdConnectionOpenTCP(String serverIP, int port)
{
  String sCmdOpenTCP;
  sCmdOpenTCP.concat(F("AT+CIPSTART=\"TCP\",\""));
  sCmdOpenTCP.concat(serverIP);
  sCmdOpenTCP.concat(F("\","));
  sCmdOpenTCP.concat(port);
  return ATCmd(sCmdOpenTCP,3000,sOK);
}

bool WebMngr::cmdConnectionClose()
{
  _wifiSerial.flush();
  _wifiSerial.println(F("AT+CIPCLOSE\r\n"));
  delay(1000);
  _wifiSerial.flush();
  return true;
}

bool WebMngr::WaitStrSerial(char strEtalon[],int timeout)
{
  //unsigned long millis1 = millis();
  unsigned long end1 = millis()+(unsigned long)timeout;// possibletroubles with millis overflow
  bool notExpired = true;
  unsigned char index = 0;
  unsigned char maxIndex = strlen (strEtalon);
  char a;
  while (notExpired){
    while (_wifiSerial.available()>0){
      a = _wifiSerial.read();
      if (strEtalon[index] == a){
        index++;
      }else{
        index = 0;
      }
      if (index == (maxIndex)){
         //PrintMessage(F("WaitStrSerial_true"));
        return true;
      }
      a='\0';
    }
    notExpired = (end1>millis());
  }
  PrintMessage(F("WaitStrSerial_false"));
  //_dbgSerial.println((String)strEtalon);
  return false;
}

void WebMngr::PrintMessage(String val)
{
  _dbgSerial.print(F("WebMngr: <"));
  _dbgSerial.print(val);
  _dbgSerial.println(F(">"));
}

