#include <Arduino.h>
#ifndef WEMNGR_H
  #define WEBMNGR_H

class WebMngr{
  public:
  void (*dbgOutput)(String msg);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  //void (*dbgOutputChr)(char msg[]);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса
  //void (*dbgOutputCh)(char msg);// указатель на функцию обработчик вывода в лог. заполняется после создания экземпляра класса

  
  private: bool WaitStrSerial(char strEtalon[],int timeout);
    Stream* WifiSerial;
  Send
    
};
#endif
