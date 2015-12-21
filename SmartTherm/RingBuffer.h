#include <Arduino.h>

#ifndef SENSORDATA_H
  #include "SensorData.h"
#endif  

class RingBuffer{
  private:
    byte firstBufIndex = 0;
    byte lastBufIndex=0;// индексы в кольцевом буфере
    SensorData RingBuf[30];
  public:
    bool BufHasData();
    byte nextBufIndex(byte i);
    byte prevBufIndex(byte i);
    SensorData popTData();
    void CancelPopData();
    void pushTData(SensorData value);
};


