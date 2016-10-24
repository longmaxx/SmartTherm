#include <WiFiEsp.h>
#include <DS1307.h>
#include <SoftwareSerial.h>
#include "DS18B20.h"
#include "LCDMngr.cpp"
#include "SensorData.h"
#include "RingBuffer.h"
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
  D3 - LCD
  D4 - LCD
  D5 - LCD
  D6 - LCD
  D7 - LCD
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

//LCDMngr lcd(7,6,5,3,4);
SoftwareSerial ExtSerial(10,11);// debug serial port
OneWire OneWirePort(12);
DS18B20 DS(&OneWirePort);

DS1307 RTC(18, 19);
#define RTC_TIME_ZONE_ADDR (0x08)

Time lastRefreshDT;// время последнего снятия данных

float lastTemperatureC;

WiFiEspClient EspClient;
String WifiAP_Name;
String WifiAP_Pwd;
char server[] = "192.168.1.100";

String sDeviceName;// = "Nano1";
signed char nTimeZone = 0;// значение временной зоны. хранится в пользовтельских регистрах модуля часов



UserCmdMngr CmdMngr1;// класс обрабатывающий пользовательские команды через SoftwareSerial
RingBufferForData RB;// Ring buffer class object в этот кольцевой буфер складываем температурные данные, которые потом будет отправлять на веб сервер.
EEPROMMngr EEManager;// EEPROM actions

boolean flag_NeedSend = false;// есть несохраненные данные
boolean flag_NeedRefreshData   = true;// пора обновлять данные с датчиков
boolean flag_ESP_NeedConfigure = true;// фдаг выставляется в случае каких-либо проблем при отсылке данных на сервер
boolean flag_ESP_Wifi_Connected = false;// проверяется в главном цикле 
boolean flag_runMainProgram = true;

#define DataRefreshIntervalMs  (60000)
unsigned long LastMillisVal=0;

void setup() {
//  lcd.begin();
  //lcd.clear();
  flag_runMainProgram = true;
  CmdMngr1.Init(&ExtSerial);
  Serial.begin(9600);
  WiFi.init(&Serial);
  
  ExtSerial.begin(9600);
  //RTC.halt(false);
  ExtSerial.println(F("Setup"));
  LoadDataFromEEPROM();
  LoadTimeZoneValue();
  DS.setTemperatureResolution();
  char apName[20];
  WifiAP_Name.toCharArray(apName,20);
  char ApPwd[20];
  WifiAP_Pwd.toCharArray(ApPwd,20);
  WiFi.begin(apName,ApPwd);
}

void loop ()
{
  CmdMngr1.SerialPortLoop();
  ExecuteUserCmdIfNeeded();
  if (flag_runMainProgram){
    if (flag_ESP_NeedConfigure){
      //ConfigureESPWifi();//если необъодимо - переподключаем вайфай
    }  
    CheckRefreshInterval();// проверяем не пора ли обновлять данные и выставляем флаг
  
    if (flag_NeedRefreshData){// пора обновлять температурные данные
      RefreshDataActions();
      //DrawLCD();
    }
    if (!flag_ESP_NeedConfigure){
      SendData();
    }
  }
}

/*void DrawLCD()
{
  lcd.clear();
  DrawLCD_Screen1();
}


void DrawLCD_Screen1()
{
  // Wifi status
  lcd.setCursor(0,0);
  lcd.writeStr(F("WiFi: "));
  if (flag_ESP_Wifi_Connected){
    lcd.writeStr(F("OK"));
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
  lcd.writeStr(RTC.getTimeStr());
  lcd.setCursor(0,3);
  lcd.writeStr(RTC.getDateStr());
  //Time
  lcd.setCursor(0,4);
  lcd.writeStr(WifiAP_Name);
}*/


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
  lastTemperatureC = DS.getTemperatureCelsium();
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


boolean SendData_Http(SensorData data)
{
  String sUrl = "TMon/index.php?r=temperatures/commit&devicename=" + sDeviceName + 
                "&celsium=" + (String)(data.Temperature) +
                "&date=" + (String)data.Timestamp.year + firstZero(data.Timestamp.mon) + firstZero(data.Timestamp.date) + firstZero(data.Timestamp.hour) + firstZero(data.Timestamp.min) + firstZero(data.Timestamp.sec) +
                getStrQueryTimeZone(nTimeZone);//"TZ" + nTimeZone;
  ExtSerial.print(F("Send HttpRequest Url:"));
  ExtSerial.println(sUrl);
    if (EspClient.connect(server, 80)) {
    ExtSerial.println("Connected to server");
    // Make a HTTP request
    EspClient.print("GET");
    EspClient.print(sUrl); 
    EspClient.println("HTTP/1.1");
    EspClient.println("Host: hui");
    EspClient.println("Connection: close");
    EspClient.println();
  }

  
  return true;
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
      case CMD_I_HELP:
        Cmd_PrintHelp();  
    }
  }
}

void Cmd_PrintHelp()
{
  ExtSerial.println(F("Available commands:"));
  CmdMngr1.PrintAvailableCommands();
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
 
 ExtSerial.println(F("Year:"));
 ExtSerialClear();
 lastRefreshDT.year = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.year);
 
 ExtSerial.println(F("Month:"));
 ExtSerialClear();
 lastRefreshDT.mon = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.mon);
 
 ExtSerial.println(F("Day:"));
 ExtSerialClear();
 lastRefreshDT.date = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.date);
 
 ExtSerial.println(F("Hour:"));
 ExtSerialClear();
 lastRefreshDT.hour  = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.hour);
 
 ExtSerial.println(F("Minute:"));
 ExtSerialClear();
 lastRefreshDT.min = ReadIntSerial();
 ExtSerial.println (lastRefreshDT.min);
 
 ExtSerial.println(F("Second:"));
 ExtSerialClear();
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
  RTC.poke(RTC_TIME_ZONE_ADDR,nTZ);
  ExtSerial.print(F("savedZone:"));
  ExtSerial.println((signed char)RTC.peek(RTC_TIME_ZONE_ADDR));
  LoadDataFromEEPROM();
 }
 
 RTC.setDate(lastRefreshDT.date,lastRefreshDT.mon,lastRefreshDT.year);
 RTC.setTime  (lastRefreshDT.hour,lastRefreshDT.min,lastRefreshDT.sec);
 Cmd_GetDate();
 ExtSerial.println(F("End Date Setting"));
 //ExtSerial.println(nTimeZone);
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
  ExtSerialClear();
  String sName = ReadStrSerial();

  ExtSerial.println (F("Enter Wifi Password"));
  ExtSerialClear();
  String sPwd = ReadStrSerial();
  
  ExtSerial.print (sName);
  ExtSerial.print ("/");
  ExtSerial.println(sPwd);
  ExtSerial.println (F("Save it?(y/n)"));
  ExtSerialClear();
  String ans = "n";
  ans = ReadStrSerial();
  if ( ans == "y"){
    WifiAP_Name = sName;
    WifiAP_Pwd = sPwd;
    EEManager.setWifiName(sName);
    EEManager.setWifiPwd(sPwd);
    ExtSerial.println(F("OK"));
  }else{
    ExtSerial.println(F("Canceled"));
    ExtSerial.println(ans);
  }
}

void Cmd_SetDeviceName()
{
  ExtSerial.println(F("Enter device name"));
  ExtSerialClear();
  String sName = ReadStrSerial();

  ExtSerial.println(F("Save it?(1/0)"));
  ExtSerialClear();
  int ans = 0;
  ans = ReadIntSerial();
  if ( ans == 1){
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

void ExtSerialClear()
{
  ExtSerial.flush();
  ExtSerial.read();
}

int ReadIntSerial()
{
  ExtSerial.setTimeout(15000);
  //return ExtSerial.parseInt();
  return ExtSerial.readStringUntil('\r').toInt();
}

String ReadStrSerial()
{
  ExtSerial.setTimeout(15000);
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
