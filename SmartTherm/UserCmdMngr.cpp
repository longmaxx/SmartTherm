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
  boolean bEndFound = false;
  //this->bufIndex = 0;
  //считываем  данные из порта пока они есть или до заполнения буфера
  while (this->SPort->available()>0){
    char curByte;
    curByte = this->SPort->read();
    if (curByte == '\r'){
      this->buf[this->bufIndex++]='\0';
      bEndFound = true;
      break;
    }else{
      this->buf[this->bufIndex++]=curByte;
    }
  }
  // проверяем буфер на наличие команды
  if ((bEndFound)&&
      (this->buf[0] == 'C')&&
      (this->buf[1] == 'M')&&
      (this->buf[2] == 'D')&&
      (this->buf[3] == ':')
                            ){
    unsigned char cmd = this->parseCmdName();
    if (cmd == 1){
      this->Cmd_Hello();
    }
  }
}

unsigned char UserCmdMngr::parseCmdName()
{
  #define ST_NOTFOUND (0)
  #define ST_OVERFLOW (-2)
 
  #define CMD_BUF_BASE (4)
  
  signed char foundState[commandsCount];
  for (unsigned char k = 0;k<commandsCount;k++ ){
    foundState[k]=0;// init values
  }
  for (unsigned char i=CMD_BUF_BASE;i<this->bufIndex;i++){
    //TODO
    // пробегаем по массиву команд и смотрим не совпадает ли байт с текущим
    for (unsigned char c = 0;c<commandsCount;c++){
     //const char* cmd_name= this->commands[c];
     
      if (foundState[c] != ST_OVERFLOW){
        if (this->commands[c][i-CMD_BUF_BASE] == this->buf[i]){
          //байт совпал
          foundState[c]++;
        //}else if (cmd_name[c][i-CMD_BUF_BASE] == '\0'){
        //  foundState[c] = ST_OVERFLOW;
        }else {
          foundState[c] = ST_OVERFLOW;
        }
      }  
    }
  }
  // считаем что получилось по поиску
  for (unsigned char k = 0;k<commandsCount;k++ ){
    if (foundState[k]!=ST_OVERFLOW)
      return k;
  }
  return 0;
}

void UserCmdMngr::Cmd_Hello()
{
  this->SPort->println("OK");  
}


