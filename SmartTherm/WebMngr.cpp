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
    String curAPName = Serial.readStringUntil('"');
    if (curAPName != sAPName){
      this->dbgOutput("WifiAPConnected: AP is wrong:"+curAPName);
      return false;  
    }
    
  }else{
    this->dbgOutput(F("WifiAPConnected: NoCmdResponse"));
    return false;
  }
  this->dbgOutput(F("WifiAPConnected: AP is ok"));
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
    while (Serial.available()>0){
      a = Serial.read();
      if (strEtalon[index] == a){
        index++;
      }else{
        index = 0;
      }
      if (index == (maxIndex)){
         this->dbgOutput(F("WaitStrSerial_true"));
        return true;
      }
      a='\0';
    }
    notExpired = (end1>millis());
  }
  this->dbgOutput(F("WaitStrSerial_false"));
  return false;
}


