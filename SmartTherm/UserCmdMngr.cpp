 #include "UserCmdMngr.h"

UserCmdMngr::UserCmdMngr(SoftwareSerial& pSWSP): _SPort(pSWSP) 
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
    if (findCmdStartStr())
    {
      lastFoundCmdID = parseCmdName();
      if (lastFoundCmdID == 0){
        _SPort.println(F("Error: Unknown command!"));  
      }  
      bufIndex=0;
    }  
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
  
  signed char foundState[commandsCount];
  for (unsigned char k = 0;k<commandsCount;k++ ){
    signed char fRes = findCharArrayInBuffer(&buf[CMD_BUF_BASE], bufLen-CMD_BUF_BASE, commands[k],getStrArrLen(commands[k]));
    if (fRes !=-1)
      return k+1;
  }
  return 0;
//  for (unsigned char i=CMD_BUF_BASE;i<bufIndex;i++){
//    //TODO
//    // пробегаем по массиву команд и смотрим не совпадает ли байт с текущим
//    for (unsigned char c = 0;c<commandsCount;c++){
//     
//      if (foundState[c] != ST_NOTMATCH){
//        if (commands[c][i-CMD_BUF_BASE] == buf[i]){
//          //байт совпал
//          foundState[c]++;
//        }else {
//          foundState[c] = ST_NOTMATCH;
//        }
//      }  
//    }
//  }
//  // считаем что получилось по поиску
//  for (unsigned char k = 0;k<commandsCount;k++ ){
//    if (foundState[k]!=ST_NOTMATCH){
//      //_SPort.print("Command detected");
//      return k+1;// zero mean not found, so begin count from 1
//    }  
//  }
//  //_SPort.println("Parse failed");
//  return 0;
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
}

unsigned char UserCmdMngr::PopLatestParsedCmd()
{
  unsigned char val = lastFoundCmdID;
  lastFoundCmdID = 0;
  return val;
}

void UserCmdMngr::PrintAvailableCommands()
{
  for (unsigned char i=0;i<commandsCount;i++)
  {
    _SPort.println(commands[i]);
  }
}



