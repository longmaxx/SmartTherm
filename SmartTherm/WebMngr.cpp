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
  Serial.print(cmd);
  Serial.println();
  delay(timeout);
  if(Serial.find(answer)) {
    this->dbgOutput(answer);
    return true;
  } else {
    this->dbgOutput("errCmd:|");
    this->dbgOutput(answer);
    this->dbgOutput("|");
    return false;
  }
}

bool WebMngr::SendGetRequest(String sUrl, String sBody)
{
 wifiCmd("AT+CIPSTART=\"TCP\",\"192.168.0.115\",80",1000,"OK");
  if(Serial.find("Error"))
    return false;
  char cmd[] = "GET / HTTP/1.0\r\n\r\n";
  this->dbgOutput("AT+CIPSEND=");
  //this->dbgOutput(sizeof(cmd));
  Serial.print("AT+CIPSEND=");
  Serial.println(strlen(cmd));
  delay(1000);
  if(Serial.find(">")) {
    this->dbgOutput(">");
  } else {
    Serial.println("AT+CIPCLOSE");
    delay(1000);
    return false;
  }
  Serial.println(cmd);
  delay(2000);
  //Serial.find("+IPD");
  String c = "";
  while (Serial.available()) {
    c += Serial.read();
    /*this->dbgOutput.write(c);*/
  }
  this->dbgOutput(c);
  return true;
}

