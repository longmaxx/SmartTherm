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

byte RingBuffer::prevBufIndex(byte i)
{
  if (i == 0){
    // крайнее левое положение
    return (sizeof(this->RingBuf)-1);
  }else{
    return --i;
  }
}


void RingBuffer::pushTData(SensorData value)
{
  // сохраняет данные в конец кольцевого буфера
  //сдвигаемся на следующую ячейку в буфере
  RingBuf[this->lastBufIndex] = value;
  //PrintMessage("Before "+(String)this.firstBufIndex + "," + (String)this.lastBufIndex);
  this->lastBufIndex = nextBufIndex(this->lastBufIndex);
  //lastBufIndex++;
  //PrintMessage("After "+(String)this.firstBufIndex + "," + (String)this.lastBufIndex);
  // если начало и конец совпали - сдвигаем начало на 1
  if (this->firstBufIndex == this->lastBufIndex){
    //PrintMessage("Data buffer is full. Old data will be lost.");
    this->firstBufIndex = nextBufIndex(this->firstBufIndex);
  }
  //PrintMessage("Push data. "+(String)firstBufIndex + "," + (String)lastBufIndex);
  
  
}


SensorData RingBuffer::popTData()
{
  // извлекает последнее значение из кольцевого буфера
  if (BufHasData()){
    byte i = this->firstBufIndex;
    this->firstBufIndex = nextBufIndex(this->firstBufIndex);
    return RingBuf[i];
  } else{
    //PrintMessage("Cannot pop data. Buffer is empty.");
  }
}

void RingBuffer::CancelPopData()
{
  this->firstBufIndex = prevBufIndex(this->firstBufIndex);
}
//
bool RingBuffer::BufHasData()
{
  //возвращает количество непрочитаных данных в буфере
  return firstBufIndex != lastBufIndex;
}
//
////=================== Кольцевой буфер End=========================================

