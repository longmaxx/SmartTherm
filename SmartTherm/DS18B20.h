#include <Arduino.h>
#include <OneWire.h>
#ifndef _DS18B20_H
  #define _DS18B20_H


#define CMD_DS_SKIP_ROM (0xCC)
#define CMD_DS_READ_SCRATCHPAD (0xBE)
#define CMD_DS_WRITE_SCRATCHPAD (0x4E)
#define CMD_DS_START_CONVERSION (0x44)


class DS18B20{
  public:
    DS18B20(OneWire &port);
    float getTemperatureCelsium();
    void setTemperatureResolution();
  private:
    OneWire &ds;
    byte scratchpad[12];
    void readScratchpad();
};

#endif
