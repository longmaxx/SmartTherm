#ifndef DS1307_H
  #include <DS1307.h>
#endif  
#ifndef SENSORDATA_H
  #define SENSORDATA_H
#endif  

class SensorData {
  public:
    
    Time Timestamp;
    float Temperature;// celsium
};

