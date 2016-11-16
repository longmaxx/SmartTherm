#include "DS18B20.h"

DS18B20::DS18B20(OneWire& port):ds(port)
{
}

float DS18B20::getTemperatureCelsium()
{
  readScratchpad();
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

void DS18B20::setTemperatureResolution()
{
    ds.reset();
    ds.write(CMD_DS_SKIP_ROM); // skip ROM
    ds.write(CMD_DS_WRITE_SCRATCHPAD);///write scratchpad
    ds.write(0x00);//TH
    ds.write(0x00);//TL
    ds.write(0b01011111);//prefs
}
void DS18B20::readScratchpad()
{
  byte i;
  ds.reset();
  ds.write(CMD_DS_SKIP_ROM);
  //ds.reset();
  ds.write(CMD_DS_START_CONVERSION); // start conversion
  delay(400);     // wait conversion
  // we might do a ds.depower() here, but the reset will take care of it.
   
  ds.reset();
  ds.write(CMD_DS_SKIP_ROM);//skip rom
  //ds.reset();    
  ds.write(CMD_DS_READ_SCRATCHPAD);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    scratchpad[i] = ds.read();
  }
}

