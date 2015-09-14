#include "RingBuffer.h"

////=================== Кольцевой буфер FILO Begin=========================================

byte RingBuffer::nextBufIndex(byte i)
{
  if (i == (sizeof(this->RingBuf)-1)){
    // крайнее правое положение
    return 0;
  }else{
    return ++i;
  }
  
}


void RingBuffer::pushTData(SensorData value)
{
  // сохраняет данные в конец кольцевого буфера
  //сдвигаемся на следующую ячейку в буфере
  RingBuf[lastBufIndex] = value;
  //PrintMessage("Before "+(String)firstBufIndex + "," + (String)lastBufIndex);
  lastBufIndex = nextBufIndex(lastBufIndex);
  //lastBufIndex++;
  //PrintMessage("After "+(String)firstBufIndex + "," + (String)lastBufIndex);
  // если начало и конец совпали - сдвигаем начало на 1
  if (firstBufIndex == lastBufIndex){
    //PrintMessage("Data buffer is full. Old data will be lost.");
    firstBufIndex = nextBufIndex(firstBufIndex);
  }
  //PrintMessage("Push data. "+(String)firstBufIndex + "," + (String)lastBufIndex);
  
  
}


SensorData RingBuffer::popTData()
{
  // извлекает последнее значение из кольцевого буфера
  if (BufHasData()){
    byte i = firstBufIndex;
    firstBufIndex = nextBufIndex(firstBufIndex);
    return RingBuf[i];
  } else{
    //PrintMessage("Cannot pop data. Buffer is empty.");
  }
}
//
bool RingBuffer::BufHasData()
{
  //возвращает количество непрочитаных данных в буфере
  return firstBufIndex != lastBufIndex;
}
//
////=================== Кольцевой буфер End=========================================

