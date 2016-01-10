#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef USERCMDMNGR_H
  #define USERCMDMNGR_H

  #define bufLen (32)
  #define commandsCount (2)
class UserCmdMngr{
  private: SoftwareSerial* SPort;
  private: char buf[bufLen];// буфер для сохранения данных команды из порта
  private: unsigned char bufIndex;
  private: const char* commands[commandsCount] = {"hello","settime"};

  public: UserCmdMngr();
  public: void Init(SoftwareSerial* pSWSP);
  public: void SerialPortLoop();
  private: unsigned char parseCmdName();// возвращает индекс команды или 0
  private: void Cmd_Hello();
};
#endif  

