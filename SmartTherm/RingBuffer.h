#include <Arduino.h>
#include "SensorData.h"

#ifndef RINGBUFFER_H
  #define RINGBUFFER_H

class RingBuffer{
  private:
    byte firstBufIndex = 0;
    byte lastBufIndex=0;// индексы в кольцевом буфере
    SensorData RingBuf[30];
    byte bufCount = 0;
  public:
    bool BufHasData();
    byte nextBufIndex(byte i);
    byte prevBufIndex(byte i);
    void push(SensorData value);
    SensorData pop();
    void CancelPopData();
    unsigned char getCount();
};

#endif  
