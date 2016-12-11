#include <Arduino.h>
#include <SoftwareSerial.h>
#ifndef USERCMDMNGR_H
  #define USERCMDMNGR_H

  #define bufLen        (32)

  #define commandsCount (8)
  #define CMD_I_HELP (1)
  #define CMD_I_INFO (2)
  #define CMD_I_SETTIME (3)
  #define CMD_I_GETTEMP (4)
  #define CMD_I_TOGGLE_RUN (5)
  #define CMD_I_SETWIFI (6)
  #define CMD_I_SETNAME (7)
  #define CMD_I_SETHOST (8)


class UserCmdMngr{
  private: const char sCmdStartWord[4] = {'C','M','D',':'};
  private: unsigned char lastFoundCmdID;  
  private: Stream &_SPort;
  private: char buf[bufLen];//буфер для сохранения данных команды из порта
  private: unsigned char bufIndex;
  private: signed char findCharArrayInBuffer(char* buf, unsigned char buffLen, const char* arr, unsigned char arrLen);
  private: unsigned char getStrArrLen(const char* arr);
  private: bool findCmdStartStr();
  public: const char* commands[commandsCount+1] = { "@",//0 placeholder
                                                  "?",//1
                                                  "info",//2
                                                  "settime",//3
                                                  "gettemp",//4
                                                  "mode",//5
                                                  "setwifi",//6
                                                  "setname",//7
                                                  "sethost"//8
                                                 };
  
  public: UserCmdMngr(Stream &pSWSP);
  public: void SerialPortLoop();
  public: unsigned char PopLatestParsedCmd();
  private: unsigned char parseCmdName();// возвращает индекс команды или 0
  public: void PrintAvailableCommands(); 
  
};
#endif  

