#ifndef MAIN_APPLICATION_H
#define MAIN_APPLICATION_H



/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH (0)

	/* TASK PRIORITIES */
#define	TASK_SERIAL_SEND_PRI		( tskIDLE_PRIORITY + 2 )
#define TASK_SERIAl_REC_PRI			( tskIDLE_PRIORITY + 3 )
#define	SERVICE_TASK_PRI		( tskIDLE_PRIORITY + 1 )

#define DISPLAY_7SEG_LEN			(6u)

#define LED_BAR_VAL0					(0u)
#define LED_BAR_VAL1					(1u)
#define LED_BAR_VAL2					(1u << 1u) | LED_BAR_VAL1
#define LED_BAR_VAL3					(1u << 2u) | LED_BAR_VAL2
#define LED_BAR_VAL4					(1u << 3u) | LED_BAR_VAL3
#define LED_BAR_VAL5					(1u << 4u) | LED_BAR_VAL4
#define LED_BAR_VAL6					(1u << 5u) | LED_BAR_VAL5
#define LED_BAR_VAL7					(1u << 6u) | LED_BAR_VAL6
#define LED_BAR_VAL8					(1u << 7u) | LED_BAR_VAL7

uint8_t ledBarValue[9u] = { LED_BAR_VAL0, LED_BAR_VAL1, LED_BAR_VAL2, LED_BAR_VAL3, LED_BAR_VAL4, 
							LED_BAR_VAL5, LED_BAR_VAL6, LED_BAR_VAL7, LED_BAR_VAL8};

/* TASKS: FORWARD DECLARATIONS */
void led_bar_tsk(void* pvParameters);
void SerialSend_Task(void* pvParameters);
void SerialReceive_Task(void* pvParameters);

/**
Task for 7Seg display
*/
void Display7Seg_Task(void* pvParameters);

/**
Task for analog output display
*/
void AnalogOutput_Task(void* pvParameters);


/** 
* The 500ms cyclic job for processing the data
*/
void CyclicJob_500ms(void* pvParameters);

/**
* The cyclic task for critical mode indication
*/
void CriticalModeTask(void* pvParameters);

/**
Function is called inside analog output task
Function set given value to the specific location at sensor memory data
Input parms:
sensorType_t sensType_e -> Type of a sensor
uint16_t sensValue_u16 -> Value of a sensor
uint16_t* sensData_u16 -> Pointer to the sensor memory 
Output parms:
None
*/
void setSensorValue(sensorType_t sensType_e, uint16_t sensValue_u16, uint16_t* sensData_u16);

/**
Function is called inside of 7Seg receive task
Function sets received error code to 7Seg display
Input parms:
errorType_t error_e -> Received error code
Output parms:
None
*/
void ErrorHandler(errorType_t error_e);

/**
Function is called inside of AnalogOutput task
Function checks received mode and set corresponding outputs
Input parms:
mode_t mode_e -> Received display mode
uint16* sensorData_u16 -> Pointer to sensor memory data
Output parms:
None
*/
void AnalogOutputHandler(mode_t mode_e, uint16_t* sensorData_u16);

/**
Function is called inside AnalogOutputHandler after valid message is received in AnalogOutput task
Function sets both columns on AnalogOutput led bar
Input parms:
uint16_t col1_u16 -> First column value
uint16_t col2_u16 -> Second column value
Output parms:
None
*/
void LedBarSetOutputs(uint16_t col1_u16, uint16_t col2_u16);

/* TRASNMISSION DATA - CONSTANT IN THIS APPLICATION */
const char trigger[] = "XYZ";
unsigned volatile t_point;

/* RECEPTION DATA BUFFER */
#define R_BUF_SIZE (32)
uint8_t r_buffer[R_BUF_SIZE];
unsigned volatile r_point;

/* GLOBAL OS-HANDLES */
SemaphoreHandle_t LED_INT_BinarySemaphore;
SemaphoreHandle_t TBE_BinarySemaphore;
SemaphoreHandle_t RXC_BinarySemaphore;
QueueHandle_t CriticalModeQueue;
TimerHandle_t per_TimerHandle;



#endif // !MAIN_APPLICATION_H