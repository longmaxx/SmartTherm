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
   // this->dbgOutput("Cmd ok");
    return true;
  } else {
    //this->dbgOutput("CmdFail");
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
  delay(2000);
  if(!Serial.find(">")) {
    //this->dbgOutput("Not found \">\" sign");
    Serial.println("AT+CIPCLOSE");
    delay(1000);
    return false;
  }
  Serial.print(msgBegin);
  Serial.print(sUrl);
  Serial.print(msgEnd);
  delay(5000);
  boolean res = Serial.find("\"status\":\"success\"");
  Serial.println("AT+CIPCLOSE");
  return res;
}

