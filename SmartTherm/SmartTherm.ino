#include <DS1307.h>
#include <OneWire.h>
#include <SoftwareSerial.h>
#include "SensorData.h"
#include "RingBuffer.h"
#include "WebMngr.h"
#include "UserCmdMngr.h"
#include "EEPROMMngr.h"


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
#define RTC_TIME_ZONE_ADDR (0x08)

Time lastRefreshDT;// время последнего снятия данных
byte scratchpad[12];
float lastTemperatureC;

String WifiAP_Name;// = "KotNet";
String WifiAP_Pwd;//  = "MyKotNet123";

String sDeviceName;// = "Nano1";
signed char nTimeZone = 0;

UserCmdMngr CmdMngr1;// класс обрабатывающий пользовательские команды через SoftwareSerial
RingBuffer RB;// Ring buffer class object в этот кольцевой буфер складываем температурные данные, которые потом будет отправлять на веб сервер.
WebMngr ESPMod;// Wifi class object
EEPROMMngr EEManager;// EEPROM actions

boolean flag_NeedSend = false;// есть несохраненные данные
boolean flag_NeedRefreshData   = true;// надо обноалять данные с датчиков
boolean flag_ESP_NeedConfigure = true;// фдаг выставляется в случае каких-либо проблем при отсылке данных на сервер
boolean flag_runMainProgram = true;

#define DataRefreshIntervalMs  (60000)
unsigned long LastMillisVal=0;

void setup() {
  flag_runMainProgram = true;
  CmdMngr1.Init(&ExtSerial);
  ExtSerial.begin(9600);
  ESPMod.dbgOutput = PrintMessage;
  ESPMod.dbgOutputCh = PrintMessageCh;
  ESPMod.dbgOutputChr = PrintMessageChr;
  //RTC.halt(false);
  ExtSerial.println(F("Setup"));
  LoadDataFromEEPROM();
  LoadTimeZoneValue();
  //delay(5000);
}

void loop ()
{
  CmdMngr1.SerialPortLoop();
  ExecuteUserCmdIfNeeded();
  if (flag_runMainProgram){
    if (flag_ESP_NeedConfigure){
      ConfigureESPWifi();//если необъодимо - переподключаем вайфай
    }  
    CheckRefreshInterval();// проверяем не пора ли обновлять данные и выставляем флаг
  
    if (flag_NeedRefreshData){// пора обновлять температурные данные
      RefreshDataActions();
    }
    SendData();
  }
  
}

void LoadDataFromEEPROM()
{
  String tmp = EEManager.getWifiName();
  if (tmp.length()>0){
    WifiAP_Name = tmp;
  }

  tmp = "";
  tmp = EEManager.getWifiPwd();
  if (tmp.length()>0){
    WifiAP_Pwd = tmp;
  }

  tmp = "";
  tmp = EEManager.getDeviceName();
  if (tmp.length()>0){
    sDeviceName = tmp;
  }
}

void LoadTimeZoneValue(){
  nTimeZone = RTC.peek(RTC_TIME_ZONE_ADDR);
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
      flag_ESP_NeedConfigure = true;
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


void saveTemperatureToRAM(){
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
void readDS18B20Scratchpad(){
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
  //ESPMod.wifiCmd("ATE0",1000,"OK");
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
  String sUrl = "TMon/index.php?r=temperatures/commit&devicename=" + sDeviceName + "&celsium=" + (String)(data.Temperature) + "&date=" + (String)data.Timestamp.year + firstZero(data.Timestamp.mon) + firstZero(data.Timestamp.date) + firstZero(data.Timestamp.hour) + firstZero(data.Timestamp.min) + firstZero(data.Timestamp.sec) + "TZ" + nTimeZone;
  ExtSerial.print(F("Send HttpRequest Url:"));
  ExtSerial.println(sUrl);
  return ESPMod.SendGetRequest(sUrl);
}

String firstZero(int val)
{
  if (val<10){
    return "0"+(String)val;  
  }else{
    return (String)val;
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
      case CMD_I_TOGGLE_RUN:
        Cmd_ToggleRunProgram();
        break;
      case CMD_I_SETWIFI:
        Cmd_SetWifiPreferences();
        break; 
      case CMD_I_SETNAME:
        Cmd_SetDeviceName();
        break; 
      case CMD_I_INFO:
        Cmd_PrintDeviceInfo();
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
 ExtSerial.println (lastRefreshDT.year);
 ExtSerial.flush();
 ExtSerial.println(F("Month:"));
 lastRefreshDT.mon = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.mon);
 ExtSerial.flush();
 ExtSerial.println(F("Day:"));
 lastRefreshDT.date = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.date);
 ExtSerial.flush();
 ExtSerial.println(F("Hour:"));
 lastRefreshDT.hour  = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.hour);
 ExtSerial.flush();
 ExtSerial.println(F("Minute:"));
 lastRefreshDT.min = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.min);
 ExtSerial.flush();
 ExtSerial.println(F("Second:"));
 lastRefreshDT.sec = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.sec);
 //просим ввести часовой пояс и сохраняем его 
 ExtSerial.flush();
 ExtSerial.println(F("TimeZone(+3):"));
 String sTimeZone = ReadStrSerial();
 signed char nTimeZone = 255;
 if ((sTimeZone[0] == '-')){
  nTimeZone = sTimeZone.substring(1).toInt();   
  nTimeZone = -nTimeZone;
 }else{
  nTimeZone = sTimeZone.toInt();   
 }
 if (nTimeZone != 255){
  RTC.poke(RTC_TIME_ZONE_ADDR,nTimeZone);
  ExtSerial.print("savedZone:");
  ExtSerial.println((signed char)RTC.peek(RTC_TIME_ZONE_ADDR));
 }
 RTC.setDate(lastRefreshDT.date,lastRefreshDT.mon,lastRefreshDT.year);
 RTC.setDate(lastRefreshDT.hour,lastRefreshDT.min,lastRefreshDT.sec);
 Cmd_GetDate();
 ExtSerial.println(F("End Date Setting"));
 ExtSerial.println(nTimeZone);
}

void Cmd_GetDate()
{
  lastRefreshDT = RTC.getTime();
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
  ExtSerial.print(F("TZ"));
  ExtSerial.print(nTimeZone);
}

void Cmd_ToggleRunProgram()
{
  flag_runMainProgram = !flag_runMainProgram;
  ExtSerial.print (F("Cmd ToggleProgramRun"));
  ExtSerial.println(flag_runMainProgram);  
}

void Cmd_SetWifiPreferences()
{
  ExtSerial.println (F("Enter Wifi name"));
  ExtSerial.flush();
  String sName = ReadStrSerial();

  ExtSerial.println (F("Enter Wifi Password"));
  ExtSerial.flush();
  String sPwd = ReadStrSerial();
  
  ExtSerial.print (sName);
  ExtSerial.print ("/");
  ExtSerial.println(sPwd);
  ExtSerial.println (F("Save it?(1/0)"));
  ExtSerial.flush();
  int ans = 0;
  ans = ReadIntSerial();
  if ( ans == 1){
    WifiAP_Name = sName;
    WifiAP_Pwd = sPwd;
    EEManager.setWifiName(sName);
    EEManager.setWifiPwd(sPwd);
    ExtSerial.println(F("OK"));
  }else{
    ExtSerial.println(F("Canceled"));
  }
}

void Cmd_SetDeviceName()
{
  ExtSerial.println(F("Enter device name"));
  ExtSerial.flush();
  String sName = ReadStrSerial();

  ExtSerial.println(F("Save it?(1/0)"));
  ExtSerial.flush();
  int ans = 0;
  ans = ReadIntSerial();
  if ( ans == 1){
    sDeviceName = sName;
    EEManager.setDeviceName(sName);
    ExtSerial.println(F("OK"));
  }else{
    ExtSerial.println(F("Canceled"));
  }
}

void Cmd_PrintDeviceInfo()
{
  ExtSerial.println(F("<Device info>"));
  //name
  ExtSerial.print(F("Device name: "));
  ExtSerial.println(sDeviceName);
  //Wifi info
  ExtSerial.print(F("Wifi AP Name: "));
  ExtSerial.println(WifiAP_Name);
  ExtSerial.print(F("Wifi connected: "));
  ExtSerial.println(BoolToStr(!flag_ESP_NeedConfigure));
  //Last temperature
  ExtSerial.print(F("Temperature data: "));
  ExtSerial.print(lastTemperatureC);
  ExtSerial.print("C");

  ExtSerial.println(F("\r\nOK"));
}
//====================END Commands===================================

int ReadIntSerial()
{
  ExtSerial.setTimeout(15000);
  return ExtSerial.parseInt();
}

String ReadStrSerial()
{
  ExtSerial.setTimeout(15000);
  return ExtSerial.readStringUntil(0x0D);
}

String BoolToStr (boolean val)
{
  return val ? F("true"): F("false");
}

