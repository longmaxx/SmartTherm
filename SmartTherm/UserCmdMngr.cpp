#include "UserCmdMngr.h"

UserCmdMngr::UserCmdMngr()
{
  
}

void UserCmdMngr::Init(SoftwareSerial* pSWSP)
{
  this->SPort = pSWSP;
}


void UserCmdMngr::SerialPortLoop()
{
  boolean bExit = false;
  this->bufIndex = 0;
  if (this->SPort->find("CMD:")){// если нашли признак начала команды
    while(!bExit){
      char curByte;
      if (this->SPort->available()>0){
        curByte = this->SPort->read();
        if ((curByte == '\r')||(this->bufIndex == (bufLen-1))){
          this->buf[this->bufIndex]='\0';
          break;
        }else{
          this->buf[this->bufIndex]=curByte;
        }
      }
    }
    //закончили прием слова команды. обрабатываем
    unsigned char cmd = this->parseCmdName();
    if (cmd == 1){
      this->Cmd_Hello();
    }
    
  }
}

unsigned char UserCmdMngr::parseCmdName()
{
  unsigned char foundState[commandsCount];
  for (unsigned char i=0;i<this->bufIndex;i++){
    //TODO
    // пробегаем по массиву команд и смотрим не совпадает ли байт с текущим
    for (unsigned char c = 0;c<commandsCount;c++){
  
    }
  }
}

void UserCmdMngr::Cmd_Hello()
{
  this->SPort->println("OK");  
}


