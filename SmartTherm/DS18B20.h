#include <Arduino.h>
#include <OneWire.h>
#ifndef _DS18B20_H
  #define _DS18B20_H

class DS18B20{
  public:
    DS18B20(OneWire &port);
    float getTemperatureCelsium();
    void setTemperatureResolution();
  private:
    OneWire &ds;
    byte scratchpad[12];
    void readDS18B20Scratchpad();
};

#endif
