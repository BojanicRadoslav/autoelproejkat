#ifndef GENERAL_H
#define GENERAL_H



/**
* Mode select message:
* MSG_LEN | APP_ID | MODE_ID | EOL |
* 
* Sensor message:
* MSG_LEN | APP_ID | SENS_ID | DATA_MSB | DATA_LSB | EOL |
*/
#define E_OK								TRUE
#define E_NOT_OK							FALSE
#define SHIFT8								(0x08u)
#define ANALOG_OUTPUT_LEN					(0x08u)
/* Indicator of end of a message */
#define EOL									(0xFFu)
/* MAX length of a msg buffer */
#define BUFF_MAX_LEN						(32u)
#define RECV_COUNTER_PAYLOAD_LEN_Pos		(0x00u)
#define RECV_COUNTER_APP_ID_Pos				(0x01u)
#define RECV_COUNTER_PAYLPAD_MIN_Pos		(0x02u)
#define RECV_COUNTER_PAYLPAD_MAX_Pos		RECV_COUNTER_PAYLPAD_MIN_Pos + BUFF_MAX_LEN
/* Application Id macros*/
#define APP_MODE_SELECT_APP_ID				(0x00u)
#define APP_SENSOR_CONTROL_APP_ID			(0x01u)

/* Mode select macros*/
#define MODESEL_MSG_LEN						(0x01u)

#define MODESEL_MONITOR_MODE_ID				(0x00u)
#define MODESEL_DRIVE_MODE_ID				(0x01u)
#define MODESEL_SPEED_MODE_ID				(0x02u)

/* Positions in received msg buffer*/
#define MODESEL_MODE_Pos					(0x00u)

/*End of mode select macros*/


/* Sensor control macros */
#define SENSCTRL_MSG_LEN					(0x03u)

#define SENSCTRL_COOL_LIQ_TEMP_SENS_ID		(0x00u)
#define SENSCTRL_AIR_INTAKE_SENS_ID			(0x01u)
#define SENSCTRL_RPM_SENS_ID				(0x02u)
#define SENSCTRL_ENG_LOAD_SENS_ID			(0x03u)
#define SENSCTRL_ACC_PEDAL_SENS_ID			(0x04u)

/* Map sensor macros */
#define SENS_COOL_LIQ_TEMP_MIN				(0u)
#define SENS_COOL_LIQ_TEMP_MAX				(120u)

#define SENS_AIR_INTAKE_MIN					(0u)
#define SENS_AIR_INTAKE_MAX					(300u)

#define SENS_RPM_SENS_MIN					(0u)
#define SENS_RMP_SENS_MAX					(10000u)

#define SENS_ENG_LOAD_MIN					(0u)
#define SENS_ENG_LOAD_MAX					(1000u)

#define SENS_ACC_PEDAL_MIN					(0u)
#define SENS_ACC_PEDAL_MAX					(100u)
/*End of map sensor macros*/

/* Positions in received msg buffer*/
#define SENSCTRL_SENS_ID_Pos				(0x00u)
#define SENSCTRL_DATA_MSB_Pos				(0x01u)
#define SENSCTRL_DATA_LSB_Pos				(0x02u)

/* Critical mode macros */
#define CRITICALMODE_COOL_LIQ_TEMP_CRITICAL	(100u)
#define CRITICALMODE_AIR_INTAKE_CRITICAL	(200u)
#define	CRITICALMODE_RPM_CRITICAL			(7000u)
#define CRITICALMODE_ENG_LOAD_CRITICAL		(500u)
#define CRITICALMODE_BLINK_FEQ				(200u) //200ms
#define CRITICALMODE_NOT_ACTIVE				(0u)
#define CRITICALMODE_ACTIVE					(1u)
/* End of critical mode macros*/

/*End of Sensor control macros*/

/* 7-SEG NUMBER DATABASE - ALL HEX DIGITS */
static const char hexnum[] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07,
								0x7F, 0x6F, 0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71 };

typedef uint8_t StdReturn_Type;

/*Enum for display mode */
typedef enum Mode
{
	modeMonitor, //show cool liq temp and air intake sensors
	modeDrive, //show rpm and engine load sensors
	modeSpeed, //show rpm and Acc pedal sensor
	numOfModes,
	modeUnused = 0xFEu,
	modeInvalid = 0xFFu,
} mode_t;

/* Enum for error type */
typedef enum Err
{
	errNoError,
	errAppId,
	errSensType,
	errModeType,
	errInvalidMsgLen,
	errUnusedError,
	numOfErrors,
	errUnknownError = 0xFFu
} errorType_t;

/*Structure that stores recv data message from Unicom*/
typedef struct DataMessage
{
	uint8_t length;
	uint8_t appId;
	uint8_t payload[BUFF_MAX_LEN];
} Message;

typedef struct Display7SegMessage
{
	mode_t mode_e;
	errorType_t error_e;
} Display7SegMessage_s;

/* Enum for sensor type */
typedef enum Sensor
{
	sensCoolLiq,
	sensAirIntake,
	sensRpm,
	sensEngLoad,
	sensAccPedal,
	numOfSensors,
	sensUnused = 0xFE,
	sensInvalid = 0xFFu
} sensorType_t;

/* Strucure that stores data for analog output */
typedef struct AnalogMsg
{
	mode_t mode;
	sensorType_t sensor_type;
	uint16_t sensor_val;
}AnalogOutputMessage_t;

SemaphoreHandle_t General_AnalotOutput_BinarySemaphore;
SemaphoreHandle_t General_Display7Seg_BinarySemaphore;
QueueHandle_t General_SendTo7segDataQueue;
QueueHandle_t General_AnalogOutputDataQueue;
QueueHandle_t General_recvDataQueue;

/**
Function is called as a part of cyclic job routine every 500ms 
Function checks queue for newly received data and handle proccessing
*/
extern void General_ProcessData(void);

/**
Function is called when Mode Select message is received from the unicom
Input parms:
uint8* payload_u8 -> payload of the message
uint8 payloadLen_u8 -> Lenght of the payload, should be exactly MODESEL_MSG_LEN
Output parms:
None
*/
extern void General_ModeSel_MsgReceived(uint8_t* payload_u8, uint8_t payloadLen_u8);

/**
Function is called when Mode Select message is received from the unicom
Input parms:
uint8* payload_u8 -> payload of the message
uint8 payloadLen_u8 -> Lenght of the payload, should be exactly MODESEL_MSG_LEN
Output parms:
None
*/
extern void General_SenseCtrl_MsgReceived(uint8_t* payload_u8, uint8_t payloadLen_u8);

/**
Function is called when data needs to be sent to 7Seg display
Input parms:
mode_t mode_e -> Mode to be displayed to 7Seg display
Output parms:
None
*/
extern void General_SendMessageTo7Seg(Display7SegMessage_s msg_s);

/**
Function is called valid sensor data is sent from unicom
Function is used to prepare AnalogOutput message to be sent to analog output
Input parms:
AnalogOutputMessage_t* msg_t -> Pointer to a AnalogOutputMessage to set requred data
mode_t mode_e -> Display mode type
sensorType_t sensType_e -> Sensor type
uint16_t sensVal_u16 -> Sensor value
Output parms:
StdReturn_Type RetVal -> E_NOT_OK if message is invalid, E_OK otherwise
*/
extern StdReturn_Type General_PrepareAnalogMessage(AnalogOutputMessage_t* msg_t, mode_t mode_e, sensorType_t sensType_e, uint16_t sensVal_u16);

/**
Function is called when message needs to be sent to the AnalogOutput
Input parm:
AnalogOutputMessage_t msg_s -> Message to be sent
Output parm:
None
*/
extern void General_SendMessageToAnalogOutput(AnalogOutputMessage_t msg_s);

/**
Function is called when error is happend
Function send provided error code to 7Seg display
Input parms:
errorType_t error_e -> Error that happend 
Output parms:
None
*/
extern void General_ErrorHandler(errorType_t error_e);

/**
Function is called when data needs to be sent to 7Seg Display
Function is used to prepare 7Seg message to be sent to 7Seg task
Input parms:
Display7SegMessage_s* msg_t -> Pointer to a Display7SegMessage to set requred data
errorType_t erroe_e -> Error code
mode_t mode_e -> Display mode type
Output parms:
StdReturn_Type RetVal -> E_NOT_OK if message is invalid, E_OK otherwise
*/
extern StdReturn_Type General_Prepare7SegMessage(Display7SegMessage_s* msg_t, errorType_t error_e, mode_t mode_e);

/**
Function is called in case of an error(ref errorType_t enum)
Function prepares Display7Seg message and send it to 7Seg task
Input parms:
errorType_t error_e -> Error code
Output parms:
None
*/
extern void General_SendErrorCodeTo7Seg(errorType_t error_e);

/**
Function is called when valid sensor value is received
Function maps uin16_t data into range of physical sensor value(ref map sensor macros)
Input parms:
sensorType_t sensor_e -> Type of a sensor
uint16_t val_16 -> received value to be mapped
Output parms:
uint16_t retVal_u16 -> mapped value(range depend on sensor)
*/
extern uint16_t General_MapSensorToPhysical(sensorType_t sensor_e, uint16_t val_u16);

/**
Function is called when valid sensor value is received
Function maps uin16_t data into range of AnalogOutput display
Input parms:
uint16_t val_16 -> received value to be mapped
Output parms:
uint16_t retVal_u16 -> mapped value
*/
extern uint16_t General_MapSensorToAnalogDisplay(uint16_t val_u16);

/**
Function checks if sensor value is critical
Input parms:
sensorType_t sensor_e -> Type of a sensor
uint16_t val_u16 -> Received sensor value
Output parm:
StdReturn_Type retVal_u8 -> CRITICALMODE_NOT_ACTIVE if mode is not critical, CRITICALMODE_ACTIVE otherwise
*/
extern StdReturn_Type General_CheckCriticalMode(sensorType_t sensor_e, uint16_t val_u16);


#endif // !GENERAL_H