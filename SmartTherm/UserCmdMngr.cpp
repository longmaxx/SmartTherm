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
  boolean bEOLFound = false;
  //this->bufIndex = 0;
  //считываем  данные из порта пока они есть или до заполнения буфера
  while (this->SPort->available()>0){
    char curByte;
    curByte = this->SPort->read();
    if (curByte == '\n'){
      this->buf[this->bufIndex++]='\0';
      bEOLFound = true;
      break;
    }else{
      //this->SPort->write(curByte);
      if (curByte != '\r'){
        this->buf[this->bufIndex++]=curByte;
      }  
    }
    if (this->bufIndex == bufLen){// переполнение буфера ожидания команд
      this->bufIndex=0;
      this->SPort->println(F("<ERROR>"));
      this->SPort->flush();
      return;
    }
  }
  // проверяем буфер на наличие команды
  if ((bEOLFound)&&
      (this->buf[0] == 'C')&&
      (this->buf[1] == 'M')&&
      (this->buf[2] == 'D')&&
      (this->buf[3] == ':')
  ){
                              
    this->lastFoundCmd = this->parseCmdName();
    if (this->lastFoundCmd == 0){
      this->SPort->println(F("Error: Unknown command!"));  
    }  
  }
  if(bEOLFound){
    this->bufIndex=0;
  }
}

unsigned char UserCmdMngr::parseCmdName()
{
  #define ST_NEVERFOUND (0)
  #define ST_NOTMATCH (-2)
  #define CMD_BUF_BASE (4)
  
  signed char foundState[commandsCount];
  for (unsigned char k = 0;k<commandsCount;k++ ){
    foundState[k]=ST_NEVERFOUND;// init values
  }
  for (unsigned char i=CMD_BUF_BASE;i<this->bufIndex;i++){
    //TODO
    // пробегаем по массиву команд и смотрим не совпадает ли байт с текущим
    for (unsigned char c = 0;c<commandsCount;c++){
     
      if (foundState[c] != ST_NOTMATCH){
        if (this->commands[c][i-CMD_BUF_BASE] == this->buf[i]){
          //байт совпал
          foundState[c]++;
        }else {
          foundState[c] = ST_NOTMATCH;
        }
      }  
    }
  }
  // считаем что получилось по поиску
  for (unsigned char k = 0;k<commandsCount;k++ ){
    if (foundState[k]!=ST_NOTMATCH){
      //this->SPort->print("Command detected");
      return k+1;// zero mean not found, so begin count from 1
    }  
  }
  //this->SPort->println("Parse failed");
  return 0;
}

unsigned char UserCmdMngr::PopLatestParsedCmd()
{
  unsigned char val = this->lastFoundCmd;
  this->lastFoundCmd = 0;
  return val;
}

void UserCmdMngr::PrintAvailableCommands()
{
  for (unsigned char i=0;i<commandsCount;i++)
  {
    this->SPort->println(this->commands[i]);
  }
}



