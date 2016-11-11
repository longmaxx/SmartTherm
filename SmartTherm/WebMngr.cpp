#include "WebMngr.h"

WebMngr::WebMngr(Stream &wifiSer,Stream &dbgSer): _wifiSerial(wifiSer),_dbgSerial(dbgSer)
{
 // wifiSerial = wifiSer;
  //dbgSerial = dbgSer;
  _wifiSerial.setTimeout(5000);
}

bool WebMngr::WifiAPConnected(String sAPName)
{
  _wifiSerial.flush();
  _wifiSerial.println(F("AT+CWJAP_CUR?"));
  if (_wifiSerial.find("\n+CWJAP_CUR:\"")){
    String curAPName = _wifiSerial.readStringUntil('"');
    if (curAPName != sAPName){
      PrintMessage("WifiAPConnected: AP is wrong:"+curAPName);
      return false;  
    }
    
  }else{
    PrintMessage(F("WifiAPConnected: NoCmdResponse"));
    return false;
  }
  PrintMessage(F("WifiAPConnected: AP is ok"));
  return true;
}

bool WebMngr::ConnectWifi(String sNetName,String sPassword)
{
  if (!WifiAPConnected(sNetName)){
    ATCmd(F("AT+CWQAP"),5000,sOK);//disconnect from any AP
    ATCmd(F("AT+CWMODE_CUR=1"),2000,sOK);
    String cmd="AT+CWJAP_CUR=\"";
    cmd+=sNetName;
    cmd+="\",\"";
    cmd+=sPassword;
    cmd+="\"";
    _wifiSerial.println(cmd);
    delay(5000);
    
    if((_wifiSerial.find("WIFI CONNECTED"))&&(_wifiSerial.find("WIFI GOT IP")&&(_wifiSerial.find("\nOK")))){
      PrintMessage(F("ConnectWifi: OK"));
      return true;
    }else{
      PrintMessage(F("ConnectWifi: Fail"));
      return false;
    }
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
  //delay(timeout);
  if(_wifiSerial.find(answer)) {
    //PrintMessage(F("WifiCmd = True"));
    return true;
  } else {
    //PrintMessage(F("WifiCmd = False"));
    //PrintMessage("|");
    return false;
  }
}

bool WebMngr::SendGetRequest(String &sUrl)
{
  PrintMessage(sUrl);
  
  String msgBegin = F("GET /");
  String msgEnd = F(" HTTP/1.1\r\nHost:192.168.1.100:80\r\n\r\n");  
  bool res = false;
  if(cmdConnectionOpenTCP("192.168.1.100",80)){
    cmdSendData(msgBegin);
    cmdSendData(sUrl);
    cmdSendData(msgEnd);
    res = true;
  }
  cmdConnectionClose();
  return res;
}

bool WebMngr::cmdSendData(String data)
{
  bool res= false;
  int len = data.length();
  
  _wifiSerial.flush();
  _wifiSerial.print(F("AT+CIPSEND="));
  _wifiSerial.println(len);

  if(WaitStrSerial(">",5000)) {
    _wifiSerial.print(data);
    res = WaitStrSerial("SEND OK",500);
  }
  return res;
}

bool WebMngr::cmdConnectionOpenTCP(String serverIP, int port)
{
  String sCmdOpenTCP;
  sCmdOpenTCP.concat(F("AT+CIPSTART=\"TCP\",\""));
  sCmdOpenTCP.concat(serverIP);
  sCmdOpenTCP.concat(F("\","));
  sCmdOpenTCP.concat(port);
  return ATCmd(sCmdOpenTCP,5000,sOK);
}

bool WebMngr::cmdConnectionClose()
{
  _wifiSerial.flush();
  _wifiSerial.println(F("AT+CIPCLOSE\r\n"));
  delay(2000);
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
         PrintMessage(F("WaitStrSerial_true"));
        return true;
      }
      a='\0';
    }
    notExpired = (end1>millis());
  }
  PrintMessage(F("WaitStrSerial_false"));
  return false;
}

void WebMngr::PrintMessage(String val)
{
  _dbgSerial.print(F("WebMngr: <"));
  _dbgSerial.print(val);
  _dbgSerial.println(F(">"));
}

