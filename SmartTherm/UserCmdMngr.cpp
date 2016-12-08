 #include "UserCmdMngr.h"

UserCmdMngr::UserCmdMngr(Stream& pSWSP): _SPort(pSWSP) 
{
}

void UserCmdMngr::SerialPortLoop()
{
  boolean bEOLFound = false;
  //bufIndex = 0;
  //считываем  данные из порта пока они есть или до заполнения буфера
  while (_SPort.available()>0){
    char curByte;
    curByte = _SPort.read();
    if (curByte == '\n'){
      buf[bufIndex++]='\0';
      bEOLFound = true;
      break;
    }else{
      //_SPort.write(curByte);
      if (curByte != '\r'){
        buf[bufIndex++]=curByte;
      }  
    }
    if (bufIndex == bufLen){// переполнение буфера ожидания команд
      bufIndex=0;
      _SPort.println(F("<ERROR>"));
      _SPort.flush();
      return;
    }
  }
  // проверяем буфер на наличие команды
  if (bEOLFound)
  {
    //_SPort.println("EOL found");
    if (findCmdStartStr())
    {
      lastFoundCmdID = parseCmdName();
      if (lastFoundCmdID == 0){
        _SPort.println(F("Error: Unknown command!"));  
      }  
    }  
    bufIndex=0;
  }
}

bool UserCmdMngr::findCmdStartStr()
{
  return (findCharArrayInBuffer(buf, bufLen, sCmdStartWord, sizeof(sCmdStartWord)) != -1);
   
}

unsigned char UserCmdMngr::parseCmdName()
{
  #define ST_NEVERFOUND (0)
  #define ST_NOTMATCH (-2)
  #define CMD_BUF_BASE (4)
  
  //signed char foundState[commandsCount];
  for (unsigned char k = 1;k<commandsCount+1;k++ ){
    signed char fRes = findCharArrayInBuffer(&buf[CMD_BUF_BASE], bufLen-CMD_BUF_BASE, commands[k],getStrArrLen(commands[k]));
    _SPort.flush();
    if (fRes !=-1){
      return k;
    }  
  }
  
  return 0;
}

unsigned char UserCmdMngr::getStrArrLen(const char* arr)
{
  for (unsigned char i=0;i<0xFF;i++){
    if (arr[i] == '\0')
      return i;
  }
  return 0;
}

signed char UserCmdMngr::findCharArrayInBuffer(char* buf, unsigned char buffLen, const char* arr, unsigned char arrLen)
{
  signed char tmpFirstIndex = -1;
  unsigned char tmpIndex = 0;
  for (unsigned char iBuf=0;iBuf<buffLen;iBuf++)
  {
    if (buf[iBuf] == arr[tmpIndex]){
      if (tmpIndex == 0)
      {
         tmpFirstIndex = iBuf;// запоминаем первый индекс в буфере 
      }
      tmpIndex++;
      if (tmpIndex == arrLen)// поиск строки закончен
      {
        return  tmpFirstIndex; // возвращаем индекс начала строки
      }
    }else{
      //символ не совпадает. сбрасываем поиск
      tmpIndex = 0;
    }
  }
  return -1;
}

unsigned char UserCmdMngr::PopLatestParsedCmd()
{
  unsigned char val = lastFoundCmdID;
  lastFoundCmdID = 0;
  return val;
}

void UserCmdMngr::PrintAvailableCommands()
{
  for (unsigned char i=1;i<commandsCount+1;i++)
  {
    _SPort.println(commands[i]);
  }
}



