#include "WebMngr.h"

void WebMngr::Setup_Hardware()
{
  Serial.begin(115200);
  Serial.setTimeout(5000);
}

bool WebMngr::WifiAPConnected(String sAPName)
{
  Serial.flush();
  Serial.println(F("AT+CWJAP_CUR?"));
  if (Serial.find("\n+CWJAP_CUR:\"")){
    String curAPName = "";
    char s = Serial.read();
    byte i = 0;
    while ((s!='"')&&(i<10)){
      curAPName = curAPName + s;
      s = Serial.read();
      i++;
    }
    if (curAPName != sAPName){
      this->dbgOutput("Wifi: AP is wrong:"+curAPName);
      return false;  
    }
    
  }else{
    this->dbgOutput(F("Wifi: NoCmdResponse"));
    return false;
  }
  this->dbgOutput(F("Wifi: AP is ok"));
  return true;
}

bool WebMngr::ConnectWifi(String sNetName,String sPassword)
{
  if (!this->WifiAPConnected(sNetName)){
    this->ATCmd(F("AT+CWQAP"),5000,sOK);//disconnect from any AP
    this->ATCmd(F("AT+CWMODE_CUR=1"),2000,sOK);
    String cmd="AT+CWJAP_CUR=\"";
    cmd+=sNetName;
    cmd+="\",\"";
    cmd+=sPassword;
    cmd+="\"";
    Serial.println(cmd);
    delay(5000);
    
    if((Serial.find("WIFI CONNECTED"))&&(Serial.find("WIFI GOT IP")&&(Serial.find("\nOK")))){
      this->dbgOutput(F("OK, Connected to WiFi."));
      return true;
    }else{
      this->dbgOutput(F("Can not connect to WiFi."));
      return false;
    }
  }  
}

//bool WebMngr::InternetAccess()
//{
//  if(ATCmd(F("AT+PING=\"ya.ru\""),5000,sOK)){
//    this->dbgOutput(F("OK, Ping internet."));
//    return true;
//  }else{
//    this->dbgOutput(F("Fail ping internet."));
//    return false;
//  }
//}

//boolean WebMngr::ListWifiAPs(){
//  return this->ATCmd(F("AT+CWLAP"),5000,sOK);
//}

boolean  WebMngr::ATCmd(String cmd, int timeout, char answer[])
{
  Serial.flush();
  Serial.println(cmd);
  //delay(timeout);
  if(Serial.find(answer)) {
    this->dbgOutput(F("WifiCmd = True"));
    return true;
  } else {
    this->dbgOutput(F("WifiCmd = False"));
    //this->dbgOutput("|");
    return false;
  }
}

bool WebMngr::SendGetRequest(String sUrl)
{
  String msgBegin = "GET /";
  String msgEnd = " HTTP/1.1\r\nHost:192.168.1.100:80\r\n\r\n";  
  if(!ATCmd(F("AT+CIPSTART=\"TCP\",\"192.168.1.100\",80"),5000,sOK)){
    Serial.println(F("AT+CIPCLOSE"));
    return false;
  }
  Serial.flush();
  Serial.print(F("AT+CIPSEND="));
  Serial.println(msgBegin.length() + sUrl.length() + msgEnd.length());
  boolean res= false;
  //delay(5000);
  if(this->WaitStrSerial(">",5000)) {
    Serial.print(msgBegin);
    Serial.print(sUrl);
    Serial.flush();
    Serial.print(msgEnd);
    res = this->WaitStrSerial("success",5000);
    this->WaitStrSerial("CLOSED",15000);
   }  
  Serial.flush();
  Serial.println(F("AT+CIPCLOSE\r\n"));
  delay(2000);
  Serial.flush();
  return res;
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
    while (Serial.available()>0){
      a = Serial.read();
      if (strEtalon[index] == a){
        index++;
      }else{
        index = 0;
      }
      if (index == (maxIndex)){
         this->dbgOutputChr("WaitStrSerial_true");
        return true;
      }
      a='\0';
    }
    notExpired = (end1>millis());
  }
  this->dbgOutputChr("WaitStrSerial_false");
  return false;
}


