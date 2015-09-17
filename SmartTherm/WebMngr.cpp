#include "WebMngr.h"

void WebMngr::Setup_Hardware()
{
  Serial.begin(115200);
  Serial.setTimeout(5000);
}

bool WebMngr::ConnectWifi(String sNetName,String sPassword)
{
  Serial.println("AT+CWMODE=1");
  String cmd="AT+CWJAP=\"";
  cmd+=sNetName;
  cmd+="\",\"";
  cmd+=sPassword;
  cmd+="\"";
  //dbgSerial.println(cmd);
  Serial.println(cmd);
  delay(2000);
  if(Serial.find("OK")){
    this->dbgOutput("OK, Connected to WiFi.");
    return true;
  }else{
    this->dbgOutput("Can not connect to the WiFi.");
    return false;
  }
  
}
void WebMngr:internetAccess()
{
  Serial.println( ' at+ping="ya.ru"' );
  
}
