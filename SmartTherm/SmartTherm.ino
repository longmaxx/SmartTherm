#include <DS1307.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include "SensorData.h"
#include "RingBuffer.h"
#include "WebMngr.h"
#include "UserCmdMngr.h"


/*
  NANO3.0 pins
  Left
  Tx (D0)  - ESP USART
  Rx (D1)  - ESP USART
  Reset
  Gnd
  D2
  D3
  D4
  D5
  D6
  D7
  D8
  D9
  D10   - ExtSerial
  D11   - ExtSerial
  D12- OneWire

  Right
  VIN
  Gnd
  Reset
  5V
  A0(14)
  A1(15)
  A2(16)
  A3(17)
  A4(18)  - RTC i2c
  A5(19)  - RTC i2c
  A6
  A7
  ARef
  3v3 out
  D13 (LED)

*/

SoftwareSerial ExtSerial(10,11);// debug serial port
OneWire ds(12);
DS1307 RTC(18, 19);

Time lastRefreshDT;// время последнего снятия данных
byte scratchpad[12];
float lastTemperatureC;

const String WifiAP_Name = "KotNet";
const String WifiAP_Pwd  = "MyKotNet123";

const String sDeviceName = "Nano1";


UserCmdMngr CmdMngr1;// класс обрабатывающий пользовательские команды через SoftwareSerial
RingBuffer RB;// Ring buffer class object в этот кольцевой буфер складываем температурные данные, которые потом будет отправлять на веб сервер.
WebMngr ESPMod;// Wifi class object

boolean flag_NeedSend = false;// есть несохраненные данные
boolean flag_NeedRefreshData   = true;// надо обноалять данные с датчиков
boolean flag_ESP_NeedConfigure = true;// фдаг выставляется в случае каких-либо проблем при отсылке данных на сервер

#define DataRefreshIntervalMs  (60000)
unsigned long LastMillisVal=0;

void setup() {
  CmdMngr1.Init(&ExtSerial);
  ExtSerial.begin(9600);
  ESPMod.dbgOutput = PrintMessage;
  ESPMod.dbgOutputCh = PrintMessageCh;
  ESPMod.dbgOutputChr = PrintMessageChr;
  //RTC.halt(false);
  ExtSerial.println(F("Setup"));
 
}

void loop ()
{
  if (flag_ESP_NeedConfigure){
    ConfigureESPWifi();//если необъодимо - переподключаем вайфай
  }  
  CheckRefreshInterval();// проверяем не пора ли обновлять данные и выставляем флаг
  if (flag_NeedRefreshData){// пора обновлять температурные данные
    RefreshDataActions();
  }
  SendData();
  CmdMngr1.SerialPortLoop();
}

void RefreshDataActions()
{
  ExtSerial.println(F("Refreshing Data!"));
  readDS18B20Scratchpad();
  lastTemperatureC = getTemperatureCelsium();
  setLastRefreshDateTime();
  saveTemperatureToRAM();
  PrintOutData();
  flag_NeedSend = true;
  flag_NeedRefreshData = false;
  LastMillisVal = millis();
}

void SendData()
{
  boolean bSendSuccessful = true;
  if (!flag_NeedSend)
    return;
  ExtSerial.println(F("\r\n====SendingData===;"));
  while(RB.BufHasData()){
    SensorData val = RB.popTData();// забираем из буфера данные
    // передаем их в отладочный сериал
    ExtSerial.println(String(val.Timestamp.hour) + ":" + String(val.Timestamp.min) + ":" + String(val.Timestamp.sec) + " >> " +String(val.Temperature));  
    // отсылаем данные по HTTP
    if (!SendData_Http(val)){
      ExtSerial.println(F("Cancel POP data"));
      RB.CancelPopData();
      bSendSuccessful = false;
      break;
    }
  }
  
  ExtSerial.println(F("====End Data===="));
  if (bSendSuccessful){
    flag_NeedSend = false;
  }
}

//выставляем флаг обновления данных если прошел интервал ожидания
void CheckRefreshInterval()
{
  unsigned long m = millis();
  unsigned long delta;
  if (m<LastMillisVal){
    delta =  (0xFFFFFFFF-LastMillisVal)+m;
  }else{
    delta = m-LastMillisVal;
  }
  if (delta>DataRefreshIntervalMs)
    flag_NeedRefreshData = true;
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
  ExtSerial.print(F("  Temperature = "));
  ExtSerial.print(lastTemperatureC);
  ExtSerial.print(F(" Celsius,\r\n"));
  // Send date
  ExtSerial.print(lastRefreshDT.year);
  ExtSerial.print(F("-"));
  ExtSerial.print(lastRefreshDT.mon);
  ExtSerial.print(F("-"));
  ExtSerial.print(lastRefreshDT.date);
  ExtSerial.print(F(" -- "));
  // Send time
  ExtSerial.print(lastRefreshDT.hour);
  ExtSerial.print(F(":"));
  ExtSerial.print(lastRefreshDT.min);
  ExtSerial.print(F(":"));
  ExtSerial.println(lastRefreshDT.sec);
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
  ExtSerial.print(F("Message: <"));
  ExtSerial.print(val);
  ExtSerial.println(F(">"));
}

void PrintMessageCh(char val)
{
  ExtSerial.print(F("MessageCh: <"));
  ExtSerial.print(val);
  ExtSerial.println(F(">"));
}

void PrintMessageChr(char val[])
{
  ExtSerial.print(F("MessageCh: <"));
  ExtSerial.print(val);
  ExtSerial.println(F(">"));
}

void ConfigureESPWifi()
{
  ESPMod.Setup_Hardware();
  delay(5000);
  ESPMod.wifiCmd("ATE0",1000,"OK");
  if (!ESPMod.ConnectWifi(WifiAP_Name,WifiAP_Pwd)){
    //ESPMod.ListWifiAPs();
    return;
  }  
  //проверяем пинг
  //if (!ESPMod.InternetAccess()){
  //  return;
 // }
  flag_ESP_NeedConfigure = false;
}

boolean SendData_Http(SensorData data)
{
  String sUrl = "TMon/index.php?route=t/commit&devicename=" + sDeviceName + "&celsium=" + (String)((int)data.Temperature) + "&date=" + (String)data.Timestamp.year + firstZero(data.Timestamp.mon) + firstZero(data.Timestamp.date) + firstZero(data.Timestamp.hour) + firstZero(data.Timestamp.min) + firstZero(data.Timestamp.sec);
  ExtSerial.print(F("Send HttpRequest Url:"));
  ExtSerial.println(sUrl);
  return ESPMod.SendGetRequest(sUrl);
}

String firstZero(int val)
{
  if (val<10){
    return "0"+(String)val;  
  }else{
    return (String)val    ;
  }
}



//======================COMMANDS=====================================
void ExecuteUserCmdIfNeeded()
{
  unsigned char cmd = CmdMngr1.PopLatestParsedCmd();
  if (cmd>0){
    switch (cmd){
      case CMD_I_HELLO:
        Cmd_Hello();
        break;
      case CMD_I_SETTIME:
        Cmd_SetDate();
        break;
      case CMD_I_GETTIME:
        Cmd_GetDate();
        break;    
    }
  }
}



void Cmd_Hello()
{
  ExtSerial.println(F("Hello OK"));  
}

void Cmd_SetDate()
{
 ExtSerial.println(F("Cmd Set Date"));
 unsigned int value;
 //unsigned char Month,Day,Hour,Minute,Second;   
 ExtSerial.println(F("Enter date\time values"));
 ExtSerial.flush();
 ExtSerial.println(F("Year:"));
 lastRefreshDT.year = ReadIntSerial();
 ExtSerial.println(F("Month:"));
 lastRefreshDT.mon = ReadIntSerial();
 ExtSerial.println(F("Day:"));
 lastRefreshDT.date = ReadIntSerial();
 ExtSerial.println(F("Hour:"));
 lastRefreshDT.hour  = ReadIntSerial();
 ExtSerial.println(F("Minute:"));
 lastRefreshDT.min = ReadIntSerial();
 ExtSerial.println(F("Second:"));
 lastRefreshDT.sec = ReadIntSerial();

 /*if ((lastRefreshDT.year>0)&&
     (lastRefreshDT.year<3000)&&
     (lastRefreshDT.mon>0)&&
     (lastRefreshDT.mon<12)&&
     (lastRefreshDT.date>0)&&
     (lastRefreshDT.date<31)&&
     (lastRefreshDT.hour>0)&&
     (lastRefreshDT.hour<23)&&
     (lastRefreshDT.min>0)&&
     (lastRefreshDT.min<59)&&
     (lastRefreshDT.sec>0)&&
     (lastRefreshDT.sec<59)
    ){*/
  ExtSerial.println(F("DataEntered OK"));      
 //}
}

void Cmd_GetDate()
{
  lastRefreshDT = RTC.getTime();
  ExtSerial.println(F(""));
  ExtSerial.println(F("Date/Time:"));
  // Send date
  ExtSerial.print(lastRefreshDT.year);
  ExtSerial.print(F("-"));
  ExtSerial.print(lastRefreshDT.mon);
  ExtSerial.print(F("-"));
  ExtSerial.print(lastRefreshDT.date);
  ExtSerial.print(F(" -- "));
  // Send time
  ExtSerial.print(lastRefreshDT.hour);
  ExtSerial.print(F(":"));
  ExtSerial.print(lastRefreshDT.min);
  ExtSerial.print(F(":"));
  ExtSerial.println(lastRefreshDT.sec); 
}
//====================END Commands===================================

int ReadIntSerial()
{
  ExtSerial.setTimeout(30000);
  return ExtSerial.parseInt();
}

