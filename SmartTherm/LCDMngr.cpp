#ifndef _LCDMNGR_H
#define _LCDMNGR_H
#include <PCD8544.h>
//#include <charset.cpp>
class LCDMngr : public PCD8544 
{
  public:
  LCDMngr(unsigned char sclk, unsigned char sdin, unsigned char dc, unsigned char rst, unsigned char sce):PCD8544(sclk, sdin, dc, rst, sce){;};
  
  void writeStr(String str)
  {
    int len = str.length();
    for (int i=0; i<len; i++)
    {
      write(str.charAt(i));
    }
  }
};
#endif
