#include "WebMngr.h"
#include <avr/wdt.h>
WebMngr::WebMngr(Stream &wifiSer,Stream &dbgSer): _wifiSerial(wifiSer),_dbgSerial(dbgSer)
{
  flushTimeout();
 // _wifiSerial.setTimeout(5000);
  //ATCmd("ATE0",1000,sOK);
}

void WebMngr::setATE(bool val)
{
  if (val){
    ATCmd("ATE1",1000,sOK);
  }else{
    ATCmd("ATE0",1000,sOK);
  }
}
void WebMngr::flushTimeout()
{
  _wifiSerial.setTimeout(2000);
}

bool WebMngr::WifiAPConnected(String sAPName)
{
  freeSerialBuf();
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
    freeSerialBuf();
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

boolean  WebMngr::ATCmd(String cmd, unsigned int timeout, char answer[])
{
  freeSerialBuf();
  _wifiSerial.println(cmd);
  flushTimeout();
  unsigned long nEndTime = millis()+(unsigned int)timeout;
  bool res = false;
//  _dbgSerial.println(cmd);
//  _dbgSerial.println(millis());
//  _dbgSerial.print("Timeout:");
//  _dbgSerial.println(timeout);
//  _dbgSerial.print("End:");
//  _dbgSerial.println(nEndTime);
  do{
//     _dbgSerial.println("CMD IT");
    wdt_reset();
    res = WaitStrSerial(answer, 1000);
//    _dbgSerial.println(millis());
  }
  while ((millis() < nEndTime) && (!res));
    
  return res;
  //int timeoutOld = _wifiSerial.getTimeout();
  //return _wifiSerial.find(answer);
}

bool WebMngr::cmdSendData(String data)
{
  bool res= false;
  int len = data.length();
  freeSerialBuf();
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
   PrintMessage(F("OpenTCP"));
  freeSerialBuf();
  String sCmdOpenTCP = "";
  sCmdOpenTCP.concat(F("AT+CIPSTART=\"TCP\",\""));
  sCmdOpenTCP.concat(serverIP);
  sCmdOpenTCP.concat(F("\","));
  sCmdOpenTCP.concat(port);
  bool res = ATCmd(sCmdOpenTCP,3000,sOK);
  PrintMessage((String)res);
  return res;
}

bool WebMngr::cmdConnectionClose()
{
  _wifiSerial.println(F("AT+CIPCLOSE\r\n"));
  delay(1000);
 freeSerialBuf();
  return true;
}

bool WebMngr::WaitStrSerial(char strEtalon[],int timeout)
{
  //unsigned long millis1 = millis();
//  String buffDbgCopy = "";
  unsigned long end1 = millis()+(unsigned long)timeout;// possibletroubles with millis overflow
  bool notExpired = true;
  unsigned char index = 0;
  unsigned char maxIndex = strlen (strEtalon);
  char a;
  while (notExpired){
    while (_wifiSerial.available()>0){
      a = _wifiSerial.read();
//      buffDbgCopy += a;
      if (strEtalon[index] == a){
        index++;
      }else{
        index = 0;
      }
      if (index == (maxIndex)){
         //PrintMessage(F("WaitStrSerial_true"));
//         _dbgSerial.print("RecSyms:");
//         _dbgSerial.println(buffDbgCopy);
        return true;
      }
      a='\0';
    }
    notExpired = (end1>millis());
  }
//  _dbgSerial.print(F("WaitStrSerial_false:"));
//  _dbgSerial.println((String)strEtalon);

//  _dbgSerial.print("RecSyms:");
//  _dbgSerial.println(buffDbgCopy);
  return false;
}

void WebMngr::PrintMessage(String val)
{
  _dbgSerial.print(F("WebMngr: <"));
  _dbgSerial.print(val);
  _dbgSerial.println(F(">"));
}

void WebMngr::freeSerialBuf()
{
  while(_wifiSerial.available()){
    _wifiSerial.read();
  }
}

