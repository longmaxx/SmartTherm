#include <avr/wdt.h>
#include <DS1307.h>
#include <SoftwareSerial.h>
#include "DS18B20.h"
#include "LCDMngr.cpp"
#include "SensorData.h"
#include "RingBuffer.h"
#include "WebMngr.h"
#include "UserCmdMngr.h"
#include "EEPROMMngr.h"

/*
  NANO3.0 pins
  Left
  Tx (D0)  - Debug Serial
  Rx (D1)  - Debug Serial
  Reset
  Gnd
  D2
  D3 - LCD
  D4 - LCD
  D5 - LCD
  D6 - LCD
  D7 - LCD
  D8
  D9
  D10   - WifiSerial
  D11   - WifiSerial
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
  A4(18)  - RTC1 i2c
  A5(19)  - RTC1 i2c
  A6
  A7
  ARef
  3v3 out
  D13 (LED)
*/

//==Disable\Enable modules===
#define MOD_USER_CMD
#define MOD_LCD
//===========================

OneWire OneWirePort(12);
SoftwareSerial SWSerial(10,11);// debug serial port
#define ExtSerial Serial
#define WifiSerial SWSerial

#ifdef MOD_LCD
  #define DISPLAY_PAGE_DEFAULT 0
  LCDMngr lcd(7,6,5,3,4);
  int displayPage = DISPLAY_PAGE_DEFAULT;
#endif  
DS18B20 DS(OneWirePort);
DS1307 RTC1(18, 19);

#define RTC1_TIME_ZONE_ADDR (0x08)
Time lastRefreshDT;// время последнего снятия данных
signed char nTimeZone = 0;// значение временной зоны. хранится в пользовтельских регистрах модуля часов

#define DataRefreshIntervalMs  (60000)
unsigned long LastMillisVal=0;
float lastTemperatureC;

String WifiAP_Name;
String WifiAP_Pwd;
String sDeviceName;// = "nano";

String sHost = "192.168.1.100";
int nPort = 80;

//User classes
#ifdef MOD_USER_CMD
  UserCmdMngr CmdMngr1(ExtSerial);// класс обрабатывающий пользовательские команды через SoftwareSerial
#endif
RingBuffer RB;// Ring buffer class object в этот кольцевой буфер складываем температурные данные, которые потом будет отправлять на веб сервер.
WebMngr ESPMod(WifiSerial,ExtSerial);// Wifi class object
EEPROMMngr EEManager;// EEPROM actions

boolean flag_NeedSend = false;// есть несохраненные данные
boolean flag_NeedRefreshData   = true;// пора обновлять данные с датчиков
boolean flag_ESP_NeedConfigure = true;// фдаг выставляется в случае каких-либо проблем при отсылке данных на сервер
boolean flag_ESP_Wifi_Connected = false;// проверяется в главном цикле 
boolean flag_runMainProgram = true;

void setup()
{
  wdt_enable (WDTO_8S); // Для тестов не рекомендуется устанавливать значение менее 8 сек.
  #ifdef MOD_LCD
    lcd.begin();
  #endif
  //lcd.clear();
  flag_runMainProgram = true;
  WifiSerial.begin(9600);
  ExtSerial.begin(9600);
  //RTC1.halt(false);
  ExtSerial.println(F("Setup"));
  LoadDataFromEEPROM();
  LoadTimeZoneValue();
  DS.setTemperatureResolution();
  Cmd_PrintHelp();
  wdt_reset();
  ESPMod.setATE(false);
}

void loop ()
{
  wdt_reset();
  #ifdef MOD_USER_CMD
    CmdMngr1.SerialPortLoop();//check for user input
    ExecuteUserCmdIfNeeded();
  #endif
  if (flag_runMainProgram){
    if (flag_ESP_NeedConfigure){
      ConfigureESPWifi();//если необъодимо - переподключаем вайфай
      wdt_reset();
    }  
    CheckRefreshInterval();// проверяем не пора ли обновлять данные и выставляем флаг
  
    if (flag_NeedRefreshData){// пора обновлять температурные данные
      RefreshDataActions();
	  #ifdef MOD_LCD
      DrawLCD();
	  #endif
      if (!flag_ESP_NeedConfigure){
        SendData();
      }
    }
  }
}

#ifdef MOD_LCD
void DrawLCD()
{
    lcd.clear();
    switch (displayPage){
      case (DISPLAY_PAGE_DEFAULT):
        DrawLCD_Screen1();
        break;
    }
}

void DrawLCD_Screen1()
{
  // Wifi status
  lcd.setCursor(0,0);
  lcd.writeStr(F("WiFi:"));
  if (flag_ESP_Wifi_Connected){
    lcd.writeStr(WifiAP_Name);
  }else{
    lcd.writeStr(F("Fail"));
  }
  //LastTemperature
  lcd.setCursor(0,1);
  lcd.writeStr(F("LastTemp:"));
  lcd.writeStr((String)lastTemperatureC);
  //Time
  lcd.setCursor(0,2);
  lcd.writeStr(F("Time:"));
  lcd.writeStr(RTC1.getTimeStr());
  lcd.setCursor(0,3);
  lcd.writeStr(RTC1.getDateStr());
}
#endif

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

  tmp = "";
  tmp = EEManager.getHostIP();
  if (tmp.length()>0){
    sHost = tmp;
  }

  int iTmp = 0;
  iTmp = EEManager.getHostPort();
  if (iTmp>0){
    nPort = iTmp;
  }
}

void LoadTimeZoneValue()
{
  nTimeZone = RTC1.peek(RTC1_TIME_ZONE_ADDR);
}

void ConfigureESPWifi()
{
  flag_ESP_Wifi_Connected = false;
  //Serial.find("ready");
  //ESPMod.ATCmd("ATE0",1000,"OK");
  if (!ESPMod.WifiAPConnected(WifiAP_Name)){
    if (!ESPMod.ConnectWifi(WifiAP_Name,WifiAP_Pwd)){
      ExtSerial.println(F("ConnectWifi:Fail"));
      return;
    }
  }    
  flag_ESP_Wifi_Connected = ESPMod.WifiAPConnected(WifiAP_Name);
  flag_ESP_NeedConfigure = false;
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
  lastRefreshDT = RTC1.getTime();
}

void RefreshDataActions()
{
  ExtSerial.println(F("Refreshing Data!"));
  lastTemperatureC = DS.getTemperatureCelsium();
  setLastRefreshDateTime();
  if (lastRefreshDT.year == 2000){
    ExtSerial.println(F("!!Get time error."));
    return;
  }
  saveDataToRingBuffer();
  PrintOutData();
  flag_NeedSend = true;
  flag_NeedRefreshData = false;
  LastMillisVal = millis();
}

void saveDataToRingBuffer()
{
  SensorData dt;
  dt.Timestamp = lastRefreshDT;
  dt.Temperature = lastTemperatureC;
  RB.push(dt);
}

void SendData()
{
  boolean bSendSuccessful = true;
  if (!flag_NeedSend)
    return;
  ExtSerial.println(F("\r\n=SendingData="));
  while(RB.BufHasData()){
    SensorData val = RB.pop();// забираем из буфера данные
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

  ExtSerial.println(F("=End Data="));
  if (bSendSuccessful){
    flag_NeedSend = false;
  }
}

boolean SendData_Http(SensorData data)
{

  
  String sRequestUrl = F("TMon2/web/index.php?r=temperatures/commit");
  String sUrlParamDeviceName = "&device_name=" + sDeviceName;
  String sUrlParamCelsium = "&celsium=" + (String)(data.Temperature);
  String sUrlParamDateTime = "&measured_at=" + 
                              (String)data.Timestamp.year    + 
                              firstZero(data.Timestamp.mon)  + 
                              firstZero(data.Timestamp.date) + 
                              firstZero(data.Timestamp.hour) + 
                              firstZero(data.Timestamp.min)  + 
                              firstZero(data.Timestamp.sec);// +
  //                            getStrQueryTimeZone(nTimeZone);//"TZ" + nTimeZone;
  ExtSerial.println(F("Send HttpRequest"));
  ESPMod.cmdConnectionClose();
  bool res = true;
  if(ESPMod.cmdConnectionOpenTCP(sHost,nPort)){
    delay(500);
    res &= ESPMod.cmdSendData(F("GET /"));
    // url
    res &= ESPMod.cmdSendData(sRequestUrl);
    // url parameters
    wdt_reset();
    res &= ESPMod.cmdSendData(sUrlParamDeviceName);
    res &= ESPMod.cmdSendData(sUrlParamCelsium);
    res &= ESPMod.cmdSendData(sUrlParamDateTime);
    wdt_reset();
    // end http request
    res &= ESPMod.cmdSendData(F(" HTTP/1.1\r\nHost:"));
    res &= ESPMod.cmdSendData(sHost);
    res &= ESPMod.cmdSendData(":"+(String)nPort+"\r\n\r\n");
    wdt_reset();
  }else{
    res = false;
  }
  delay(3000);// time to receive data from server
  ESPMod.cmdConnectionClose();
  return res;
}

void PrintOutData()
{
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

String getStrQueryTimeZone(int nTimeZone)
{
  String res = "";
  if (nTimeZone>0) 
    res += "%2B";  
  else
    res += "-";
  if (nTimeZone<10)
    res +="0";
  res += nTimeZone;
  res +="00";
  return res;    
}

//======================COMMANDS=====================================
#ifdef MOD_USER_CMD
void ExecuteUserCmdIfNeeded()
{
  unsigned char cmd = CmdMngr1.PopLatestParsedCmd();
  if (cmd>0){
    switch (cmd){
      case CMD_I_SETTIME:
        Cmd_SetDate();
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
      case CMD_I_HELP:
        Cmd_PrintHelp();  
        break;
      case CMD_I_SETHOST:
        Cmd_SetHost();
        break;
      default:
          ExtSerial.println(F("!!No handler!"));
    }
  }
}

void Cmd_PrintHelp()
{
  ExtSerial.println(F("Available commands:"));
  CmdMngr1.PrintAvailableCommands();
}

void Cmd_SetDate()
{
 ExtSerial.println(F("Cmd Set Date"));
 unsigned int value;
 //unsigned char Month,Day,Hour,Minute,Second;   
 ExtSerial.println(F("Enter date\time values"));
 
 ExtSerial.println(F("Year:"));
 clearExtSerialBuf();
 lastRefreshDT.year = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.year);
 
 ExtSerial.println(F("Month:"));
 clearExtSerialBuf();
 lastRefreshDT.mon = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.mon);
 
 ExtSerial.println(F("Day:"));
 clearExtSerialBuf();
 lastRefreshDT.date = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.date);
 
 ExtSerial.println(F("Hour:"));
 clearExtSerialBuf();
 lastRefreshDT.hour  = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.hour);
 
 ExtSerial.println(F("Minute:"));
 clearExtSerialBuf();
 lastRefreshDT.min = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.min);
 
 ExtSerial.println(F("Second:"));
 clearExtSerialBuf();
 lastRefreshDT.sec = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.sec);
 
 //просим ввести часовой пояс и сохраняем его 
 ExtSerial.println(F("TimeZone(+3):"));
 String sTimeZone = ReadStrSerial();
 signed char nTZ = 255;
 if ((sTimeZone.charAt(0) == '-')){
  nTZ = sTimeZone.substring(1).toInt();   
  nTZ = -nTZ;
 }else{
  nTZ = sTimeZone.substring(1).toInt();   
 }
 if (nTZ != 255){
  RTC1.poke(RTC1_TIME_ZONE_ADDR,nTZ);
  ExtSerial.print(F("savedZone:"));
  ExtSerial.println((signed char)RTC1.peek(RTC1_TIME_ZONE_ADDR));
  LoadDataFromEEPROM();
 }
 
 RTC1.setDate(lastRefreshDT.date,lastRefreshDT.mon,lastRefreshDT.year);
 RTC1.setTime  (lastRefreshDT.hour,lastRefreshDT.min,lastRefreshDT.sec);
 Debug_PrintDate();
 ExtSerial.println(F("\nEnd Date Setting"));
 //ExtSerial.println(nTimeZone);
}

void Debug_PrintDate()
{
  lastRefreshDT = RTC1.getTime();
  ExtSerial.print(F("Date/Time: "));
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
  clearExtSerialBuf();
  String sName = ReadStrSerial();

  ExtSerial.println (F("Enter Wifi Password"));
  clearExtSerialBuf();
  String sPwd = ReadStrSerial();
  
  ExtSerial.print (sName);
  ExtSerial.print ("/");
  ExtSerial.println(sPwd);
  if (AskSave()){
    WifiAP_Name = sName;
    WifiAP_Pwd = sPwd;
    EEManager.setWifiName(sName);
    EEManager.setWifiPwd(sPwd);
    ExtSerial.println(F("OK"));
  }else{
    ExtSerial.println(F("Canceled"));
  }
}
boolean AskSave()
{
  ExtSerial.println(F("Save it?(1/0)"));
  clearExtSerialBuf();
  boolean res = ReadIntSerial() == 1;;
  clearExtSerialBuf();
  return res;
}

void Cmd_SetDeviceName()
{
  ExtSerial.println(F("Enter device name"));
  clearExtSerialBuf();
  String sName = ReadStrSerial();

  if (AskSave()){
    sDeviceName = sName;
    EEManager.setDeviceName(sName);
    ExtSerial.println(F("OK"));
    ExtSerial.println(sDeviceName);
  }else{
    ExtSerial.println(F("Canceled"));
  }
}

void Cmd_PrintDeviceInfo()
{
  ExtSerial.println(F("<Device info>"));
  //host
  ExtSerial.print(F("Host: "));
  ExtSerial.print(sHost);
  ExtSerial.print(":");
  ExtSerial.println(nPort);
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
  ExtSerial.println("C");
  // DATE
  Debug_PrintDate();
  ExtSerial.println(F("\r\nOK"));
}

void Cmd_SetHost()
{
  ExtSerial.println(F("Enter host IP"));
  clearExtSerialBuf();
  String ip = ReadStrSerial();
  
  ExtSerial.println(F("Enter host port"));
  clearExtSerialBuf();
  int port = ReadIntSerial();

  if (AskSave()){
    ExtSerial.println(F("TODO Check IP!!!"));
    sHost = ip;
    nPort = port;
    EEManager.setHostIP(sHost);
    EEManager.setHostPort(nPort);
  }else{
    ExtSerial.println(F("Canceled"));
  }
}
#endif //#ifdef MOD_USER_CMD
//====================END Commands===================================
void clearExtSerialBuf()
{
  while(ExtSerial.available()){
    ExtSerial.read();
  }
}

int ReadIntSerial()
{
  ExtSerial.setTimeout(5000);
  wdt_reset();
  //return ExtSerial.parseInt();
  return ExtSerial.readStringUntil('\r').toInt();
}

String ReadStrSerial()
{
  ExtSerial.setTimeout(5000);
  wdt_reset();
  return ExtSerial.readStringUntil('\r');
}

String BoolToStr (boolean val)
{
  return val ? F("true"): F("false");
}

String firstZero(int val)
{
  if (val<10){
    return "0"+(String)val;  
  }else{
    return (String)val;
  }
}



