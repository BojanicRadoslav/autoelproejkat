# autoelproejkat
System support 2 messages, message for mode select and message for setting sensor value
Supported hardware: 
  LED analog VU meter
  7Seg display
  
Available sensors:
  Cooling Liquid Temperature sensor
  Air Intage Temperature sensor
  Rpm sensor
  Engine load sensor 
  Acceleration Pedal sensor
  
  Sensor values are stored in 16bit memory per sensor and mapped to physical values:
  Cooling Liquid Temperature sensor
    Max value: 120
    Min value: 0
    Resolution: 0.0018 degrees
    Critical value: 100 degrees
    
  Air Intage Temperature sensor:
    Max value: 300
    Min value: 0
    Resolution: 0.0045 degrees
    Critical value: 200 degrees
    
  
  Rpm sensor sensor:
    Max value: 0
    Min value: 10000
    Resolution: 0.27 Rpm
    Critical value: 7000 Rpm
    
  Engine load sensor :
    Max value: 0
    Min value: 1000
    Resolution: 0.027 Nm
    Critical value: 500 Nm
    
   Acceleration Pedal sensor :
    Max value: 0
    Min value: 100
    Resolution: 0.0015 percents
    Critical value: None
    
Critical mode support:
  When corresponding sensor values are critical analog LED bar starts blinking
  
Supported modes are:
  Monitor mode
  Drive mode
  Speed mode
  
Monitor mode:
 System displays values of a sensors:
 LED bar row 1: Cooling Liquid Temperature sensor
 LED bar row 2: Air Intage Temperature sensor

Drive mode:
 System displays values of a sensors:
 LED bar row 1: Rpm sensor
 LED bar row 2: Engine load sensor 
 
 
Speed mode:
 System displays values of a sensors:
 LED bar row 1: Rpm sensor
 LED bar row 2: Acceleration Pedal sensor
 
7Seg message codes:
  
  Modes(positions 0 and 1):
  0x00 -> Monitor mode
  0x01 -> Drive mode
  0x02 -> Speed mode
  0xFF -> Invalid mode
  
  Error codes(positions 4 and 6):
  0x00 -> No Error
  0x01 -> Invalid app ID
  0x02 -> Invalid sensor type
  0x03 -> invalid mode type
  0x04 -> unvalid message length
  0xFF -> Invalid error code
  
Critical mode:


Message catalog:
  ModeSelectMsg
  | len | appId | mode | EOL |
  byte 0 -> message length, must be 0x01
  byte 1 -> Application ID, must be 0x00
  byte 2 -> Selected system mode
    0x00      -> Monitor mode
    0x01      -> Drive mode
    0x02      -> Speed mode
    0x03-0xFF -> Invalid mode

  byte 3 -> EOL sign, must be 0xFF
  
  Set sensor value message
  | len | appId | sensId | sensValMSB | sensValLSB | EOL
  byte 0 -> message length, must be 0x03
  byte 1 -> Application ID, must be 0x01
  byte 2 -> Type of a sensor
    0x00      -> Cooling Liquid Temperature sensor
    0x01      -> Air Intage Temperature sensor
    0x02      -> Rpm sensor
    0x03      -> Engine load sensor 
    0x04      -> Acceleration Pedal sensor
    0x05-0xFF -> Invalid sensor type
 byte3 -> sensor value MSB, goes from 0x00-0xFF
 byte4 -> sensor value LSB, goes from 0x00-0xFF
 byte5 -> EOL, must be 0xFF
 
 Testing system:
  Setting mode:
  In unicom send hex message \01\00\00\ff\ (3. byte indicates desired mode is Monitor mode)
  \05\00\01\ff will result with msg len invalid error
  \01\00\08\ff will result with mode type invalid error
  
  Setting sensor val:
  \03\01\00\01\ff\ff -> 00 indicates requred sensor is Cooling Liquid Temperature sensor, 01ff is 16bit value of a sensor
  \07\01\00\00\00\ff will result with msg len invalid error
  \03\01\09\fe\ab\ff will result with sensor type invalid error
  
