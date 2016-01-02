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
    this->dbgOutput("Can not connect to the WiFi.");
    return false;
  }
}

bool WebMngr::InternetAccess()
{
  Serial.flush();
  Serial.println( 'AT+PING="ya.ru"' );
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
  Serial.flush();
  Serial.print("AT+CIPSEND=");
  Serial.println(msgBegin.length() + sUrl.length() + msgEnd.length());
  boolean res= false;
  if(this->WaitStrSerial(">",2000)) {
    Serial.print(msgBegin);
    Serial.print(sUrl);
    Serial.print(msgEnd);
    bool res = this->WaitStrSerial("success",15000);
  }  
  //Serial.println("aftersend\r\n");
  Serial.println("AT+CIPCLOSE");
  Serial.flush();
  return res;
}

bool WebMngr::WaitStrSerial(char strEtalon[],int timeout)
{
  unsigned long millis1 = millis();
  unsigned long end1 = millis()+(unsigned long)timeout;// possibletroubles with millis overflow
  unsigned long notExpired = true;
  unsigned char index = 0;
  unsigned char maxIndex = strlen (strEtalon);
  char a;
  this->dbgOutput( String(maxIndex));
  while (notExpired){
    //this->dbgOutputChr("ITR");
    if (Serial.available()>0){
      a = Serial.read();
      this->dbgOutputCh(a);
      char b = strEtalon[index];
      if (a == b){
        //this->dbgOutputChr("Compare");
        //this->dbgOutputCh(a);
       // this->dbgOutputCh(b);
        //this->dbgOutput((String)index);
        index++;
      }else{
        //this->dbgOutput("IndexReset");
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
   unsigned long millis2 = millis();
   this->dbgOutputChr("WaitStrSerial_false");
   char buf[10];
   this->dbgOutputChr(ltoa(millis1, buf, 10));
   this->dbgOutputChr(ltoa(millis2, buf, 10));
   this->dbgOutputChr(ltoa(end1, buf, 10));
  return false;
}


