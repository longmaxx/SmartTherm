#include "WebMngr.h"

void WebMngr::Setup_Hardware()
{
  Serial.begin(115200);
  Serial.setTimeout(5000);
}

bool WebMngr::ConnectWifi(String sNetName,String sPassword)
{
  Serial.println("AT+CWMODE=1");
  delay(1000);
  String cmd="AT+CWJAP_CUR=\"";
  cmd+=sNetName;
  cmd+="\",\"";
  cmd+=sPassword;
  cmd+="\"";
  Serial.println(cmd);
  delay(5000);
  if(Serial.find("OK")){
    this->dbgOutput("OK, Connected to WiFi.");
    return true;
  }else{
    //Serial.print("hui");
    this->dbgOutput("Can not connect to the WiFi.");
    return false;
  }
}

bool WebMngr::InternetAccess()
{
  Serial.println( ' at+ping="ya.ru"' );
  delay(2000);
  if(Serial.find("OK")){
    this->dbgOutput("OK, Ping internet.");
    return true;
  }else{
    this->dbgOutput("Fail ping internet.");
    return false;
  }
}

boolean WebMngr::ListWifiAPs(){
  return this->wifiCmd("AT+CWLAP",5000,"OK");
}

boolean  WebMngr::wifiCmd(char cmd[], int timeout, char answer[]) {
  this->dbgOutput(cmd);
  Serial.flush();
  Serial.println(cmd);
  //delay(timeout);
  if(Serial.find(answer)) {
    return true;
  } else {
    //this->dbgOutput(answer);
    //this->dbgOutput("|");
    return false;
  }
}

bool WebMngr::SendGetRequest(String sUrl)
{
  if(!wifiCmd("AT+CIPSTART=\"TCP\",\"192.168.1.100\",80",2000,"OK")){
    Serial.println("AT+CIPCLOSE");
    return false;
  }
  String msgBegin = "GET /";
  String msgEnd = " HTTP/1.1\r\nHost:192.168.1.100:80\r\n\r\n";  
  Serial.print("AT+CIPSEND=");
  Serial.println(msgBegin.length() + sUrl.length() + msgEnd.length());
  boolean res= false;
  delay(2000);
  if(this->WaitStrSerial(">",2000)) {
    Serial.print(msgBegin);
    Serial.print(sUrl);
    Serial.print(msgEnd);
    delay(4000);
    bool res = this->WaitStrSerial("success",5000);
  }  
  Serial.println("AT+CIPCLOSE");
  return res;
}

bool WebMngr::WaitStrSerial(char strEtalon[],int timeout)
{
  //unsigned long end1 = millis()+timeout;// possibletroubles with millis overflow
  unsigned char index = 0;
  unsigned char maxIndex = sizeof (strEtalon)-1;
  while (Serial.available() /*|| (millis()<end1)*/){
    this->dbgOutput("Iterate ");
    //this->dbgOutput((String)index);
    char a = Serial.read();
    if (a == strEtalon[index]){
      index++;
    }else{
      this->dbgOutput((String)a);
      index = 0;
    }
    if (index == (maxIndex)){
      this->dbgOutput("DbgSer_true");
      return true;
    }  
  }
  this->dbgOutput("DbgSer_false");
  return false;
}

