#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef USERCMDMNGR_H
  #define USERCMDMNGR_H

  #define bufLen        (32)
  #define commandsCount (4)

  #define CMD_I_HELLO   (1)
  #define CMD_I_SETTIME (2)
  #define CMD_I_GETTIME (3)
  #define CMD_I_GETTEMP (4)
class UserCmdMngr{
  private: unsigned char lastFoundCmd;  
  private: SoftwareSerial* SPort;
  private: char buf[bufLen];//буфер для сохранения данных команды из порта
  private: unsigned char bufIndex;
  private: const char* commands[commandsCount] = {"hello","settime","gettime","gettemp"};
  
  public: UserCmdMngr();
  public: void Init(SoftwareSerial* pSWSP);
  public: void SerialPortLoop();
  public: unsigned char PopLatestParsedCmd();
  private: unsigned char parseCmdName();// возвращает индекс команды или 0
  
};
#endif  

