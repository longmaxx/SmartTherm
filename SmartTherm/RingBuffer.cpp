#include "RingBuffer.h"

////=================== Кольцевой буфер FIFO Begin=========================================

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


void RingBuffer::push(SensorData value)
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
  }else{
    this->bufCount++;
  }
  //PrintMessage("Push data. "+(String)firstBufIndex + "," + (String)lastBufIndex);
  
}


SensorData RingBuffer::pop()
{
  // извлекает последнее значение из кольцевого буфера
  if (BufHasData()){
    byte i = this->firstBufIndex;
    this->firstBufIndex = nextBufIndex(this->firstBufIndex);
    this->bufCount--;
    return RingBuf[i];
  } else{
    //PrintMessage("Cannot pop data. Buffer is empty.");
  }
}

void RingBuffer::CancelPopData()
{
  this->firstBufIndex = prevBufIndex(this->firstBufIndex);
  this->bufCount++;
}
//
bool RingBuffer::BufHasData()
{
  //возвращает количество непрочитаных данных в буфере
  //return firstBufIndex != lastBufIndex;
  return this->bufCount>0;
}

unsigned char RingBuffer::getCount()
{
  return (unsigned char)this->bufCount;
}
//
////=================== Кольцевой буфер End=========================================

