#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef USERCMDMNGR_H
  #define USERCMDMNGR_H

  #define bufLen (20)
  #define commandsCount (1)
class UserCmdMngr{
  private: SoftwareSerial* SPort;
  private: char buf[bufLen];// буфер для сохранения данных команды из порта
  private: unsigned char bufIndex;
  private: const char commands[commandsCount] = {"Hello"};

  public: UserCmdMngr();
  public: void Init(SoftwareSerial* pSWSP);
  public: void SerialPortLoop();
  private: unsigned char UserCmdMngr::parseCmdName();// возвращает индекс команды или 0
  private: void Cmd_Hello();
};
#endif  

