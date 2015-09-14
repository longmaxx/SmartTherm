#include <DS1307.h>
#include <OneWire.h>
#include "SensorData.h"
#include "RingBuffer.h"
#define bufSize (5)

OneWire ds(10);
byte scratchpad[12];
float lastTemperatureC;

DS1307 RTC(A4, A5);
Time lastRefreshDT;

RingBuffer RB;// Ring buffer object

boolean flag_NeedSend = false;
boolean flag_NeedRefreshData = true;
boolean flag_SendData = false;


unsigned int DataRefreshIntervalMs = 10000;//ms
unsigned int LastMillisVal=0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  RTC.halt(false);
  //RTC.setTime(22, 48, 00);    
  //RTC.setDate(27, 8, 2015);
  Serial.println("Setup");
  
  
}
void loop ()
{
  if (flag_NeedRefreshData){// пора обновлять температурные данные
    readDS18B20Scratchpad();
    lastTemperatureC = getTemperatureCelsium();
    setLastRefreshDateTime();
    saveTemperatureToRAM();
    PrintOutData();
    flag_NeedSend = true;
    flag_NeedRefreshData = false;
    //delay(1000);
    LastMillisVal = millis();
  }
  CheckRefreshInterval();
  ReadSerialCmd();
  SendData();
}

void ReadSerialCmd()
{
  int i = 0;
  byte a;
  while( Serial.available() && i< 99) {
        a = Serial.read();
        i++;
        switch (a){
          case 1:
            flag_SendData = true;
            Serial.println("Cmd ReadData");
            break;
        }
  }
}

void SendData()
{
  if (!(flag_SendData&&flag_NeedSend))
    return;
  Serial.println("\r\n====SendingData===;");
  while(RB.BufHasData()){
    SensorData val = RB.popTData();
    Serial.println(String(val.Timestamp.hour) + ":" + String(val.Timestamp.min) + ":" + String(val.Timestamp.sec) + " >> " +String(val.Temperature));  
  }
  
  Serial.println("====End Data====");
  flag_NeedSend = false;
  flag_SendData = false;
}

//выставляем флаг обновления данных если прошел интервал ожидания
void CheckRefreshInterval()
{
  if ((millis()-LastMillisVal)>DataRefreshIntervalMs)
    flag_NeedRefreshData = true;
  else
    flag_NeedRefreshData = false;  
}

void setLastRefreshDateTime()
{
  lastRefreshDT = RTC.getTime();
}


void saveTemperatureToRAM()
{
  //реализация кольцевого буфера для хранения температурных данных в RAM  
  SensorData dt;
  dt.Timestamp = lastRefreshDT;
  dt.Temperature = lastTemperatureC;
  RB.pushTData(dt);
}

void PrintOutData(){
  Serial.print("  Temperature = ");
  Serial.print(lastTemperatureC);
  Serial.print(" Celsius,\r\n");
  // Send date
  Serial.print(RTC.getDateStr());
  Serial.print(" -- ");
  // Send time
  Serial.println(RTC.getTimeStr());
}
void readDS18B20Scratchpad()
{
  byte i;
  ds.reset();
  ds.write(0xCC);//skip rom
  //ds.reset();
  ds.write(0x44); // start conversion
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
   
  ds.reset();
  ds.write(0xCC);//skip rom
  //ds.reset();    
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    scratchpad[i] = ds.read();
  }
}

float getTemperatureCelsium()
{
  byte type_s = false;
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (scratchpad[1] << 8) | scratchpad[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (scratchpad[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - scratchpad[6];
    }
  } else {
    byte cfg = (scratchpad[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
  
}

void PrintMessage(String val)
{
  Serial.print("Message: <");
  Serial.print(val);
  Serial.println(">");
}

