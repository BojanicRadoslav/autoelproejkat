/* Standard includes. */
#include <stdio.h> 
#include <conio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"

#include "HW_access.h"
/* Application includes */
#include "General.h"
#include "main_application.h"

/* Private variables */

static uint8_t recvCounter = 0u;


/* Private functions */
/* OPC - ON INPUT CHANGE - INTERRUPT HANDLER */
static uint32_t OnLED_ChangeInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	xSemaphoreGiveFromISR(LED_INT_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}

/* TBE - TRANSMISSION BUFFER EMPTY - INTERRUPT HANDLER */
static uint32_t prvProcessTBEInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	xSemaphoreGiveFromISR(TBE_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}

/* RXC - RECEPTION COMPLETE - INTERRUPT HANDLER */
static uint32_t prvProcessRXCInterrupt(void)
{
	BaseType_t xHigherPTW = pdFALSE;

	xSemaphoreGiveFromISR(RXC_BinarySemaphore, &xHigherPTW);

	portYIELD_FROM_ISR(xHigherPTW);
}

/* PERIODIC TIMER CALLBACK */
static void TimerCallback(TimerHandle_t xTimer)
{ 
	//static uint8_t bdt = 0;
	//set_LED_BAR(2, 0x00);//sve LEDovke iskljucene
	//set_LED_BAR(3, 0xF0);// gornje 4 LEDovke ukljucene
	//
 //   set_LED_BAR(0, bdt); // ukljucena LED-ovka se pomera od dole ka gore
	//bdt <<= 1;
	//if (bdt == 0)
	//	bdt = 1;
}

static void InitAnalogOutput(void)
{
	/* Create queue for sending data to 7Seg display */
	General_AnalogOutputDataQueue = xQueueCreate(10, sizeof(AnalogOutputMessage_t)); //queue can store up to 10 messages

	/* Create task for 7Seg display */
	xTaskCreate(AnalogOutput_Task, "ANALOUT", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* Create binary semaphore for sync between data processing task and 7seg task*/
	General_AnalotOutput_BinarySemaphore = xSemaphoreCreateBinary();
}

static void reset7Seg(void)
{
	uint8_t i_u8 = 0u;
	for (i_u8 = 0u; (uint8_t)DISPLAY_7SEG_LEN > i_u8; i_u8++)
	{
		select_7seg_digit(i_u8);
		set_7seg_digit(0u);
	}
}

static void Init7Seg(void)
{
	/* Create queue for sending data to 7Seg display */
	General_SendTo7segDataQueue = xQueueCreate(10, sizeof(Display7SegMessage_s)); //queue can store up to 10 messages

	/* Create task for 7Seg display */
	xTaskCreate(Display7Seg_Task, "SEVENSEG", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* Create binary semaphore for sync between data processing task and 7seg task*/
	General_Display7Seg_BinarySemaphore = xSemaphoreCreateBinary();

	reset7Seg();
}

static void ResetLedBar(void)
{
	set_LED_BAR(0u, 0u);
	set_LED_BAR(1u, 0u);
	set_LED_BAR(2u, 0u);
}

/* MAIN - SYSTEM STARTUP POINT */
void main_demo( void )
{
	init_7seg_comm();
	init_LED_comm();
	init_serial_uplink(COM_CH);  // inicijalizacija serijske TX na kanalu 0
	init_serial_downlink(COM_CH);// inicijalizacija serijske TX na kanalu 0
	Init7Seg();
	ResetLedBar();

	/* ON INPUT CHANGE INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_OIC, OnLED_ChangeInterrupt);

	/* Create LED interrapt semaphore */
	LED_INT_BinarySemaphore = xSemaphoreCreateBinary();

	/* create a timer task */
	//per_TimerHandle = xTimerCreate("Timer", pdMS_TO_TICKS(500), pdTRUE, NULL, TimerCallback);
	//xTimerStart(per_TimerHandle, 0);

	

	/* SERIAL TRANSMITTER TASK */
	xTaskCreate(SerialSend_Task, "STx", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_SEND_PRI, NULL);

	/* SERIAL RECEIVER TASK */
	xTaskCreate(SerialReceive_Task, "SRx", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAl_REC_PRI, NULL);
	r_point = 0;

	/* Create TBE semaphore - serial transmit comm */
	TBE_BinarySemaphore = xSemaphoreCreateBinary();

	/* Create TBE semaphore - serial transmit comm */
	RXC_BinarySemaphore = xSemaphoreCreateBinary();

	/* SERIAL TRANSMISSION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_TBE, prvProcessTBEInterrupt);

	/* SERIAL RECEPTION INTERRUPT HANDLER */
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);

	/* create a led bar TASK */
	//xTaskCreate(led_bar_tsk, "ST",	configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* create cyclic 500ms task*/
	xTaskCreate(CyclicJob_500ms, "CJ", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* Create queue for sending data for proccessing*/
	General_recvDataQueue = xQueueCreate(10, sizeof(Message)); //queue can store up to 10 messages

	/* Create cyclic task for critical mode blinik */
	xTaskCreate(CriticalModeTask, "CMT", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);

	/* Create queue for sending data to CriticalModeTask */
	CriticalModeQueue = xQueueCreate(10, sizeof(uint8_t)); //queue can store up to 10 messages
	InitAnalogOutput();

	vTaskStartScheduler();

	while (1);
}

/* Public functions */
void led_bar_tsk(void* pvParameters)
{
	unsigned i;
	uint8_t d;
			while (1)
	{  
		xSemaphoreTake(LED_INT_BinarySemaphore, portMAX_DELAY);
				get_LED_BAR(1, &d);
		i = 3;
		do
		{
			i--;
			select_7seg_digit(i);
			set_7seg_digit(hexnum[d % 10]);
			d /= 10;
		} while (i > 0);
	}
}

void CyclicJob_500ms(void* pvParameters)
{
	const TickType_t xDelay = 500u / portTICK_PERIOD_MS;
	for (;;)
	{
		//printf("Hello from cyclic task\n");
		General_ProcessData();
		vTaskDelay(xDelay);
	}
}

void SerialSend_Task(void* pvParameters)
{
	t_point = 0;
	while (1)
	{

		if (t_point > (sizeof(trigger) - 1))
			t_point = 0;
		send_serial_character(COM_CH, trigger[t_point++]);
		xSemaphoreTake(TBE_BinarySemaphore, portMAX_DELAY);// kada se koristi predajni interapt
		//vTaskDelay(pdMS_TO_TICKS(100));// kada se koristi vremenski delay
	}
}

void SerialReceive_Task(void* pvParameters)
{
	uint8_t cc = 0;
	static uint8_t loca = 0;
	static Message msg = { 0u };
	while (1)
	{
		xSemaphoreTake(RXC_BinarySemaphore, portMAX_DELAY);// ceka na serijski prijemni interapt
		get_serial_character(COM_CH, &cc);//ucitava primljeni karakter u promenjivu cc

		switch (recvCounter)
		{
			case RECV_COUNTER_PAYLOAD_LEN_Pos:
			{
				msg.length = cc;
				recvCounter++;
				break;
			}
			case RECV_COUNTER_APP_ID_Pos:
			{
				msg.appId = cc;
				recvCounter++;
				break;
			}
			default:
			{
				if (0xFFu != cc)
				{
					msg.payload[recvCounter - RECV_COUNTER_PAYLPAD_MIN_Pos] = cc;
					recvCounter++;
				}
				else
				{
					if (APP_SENSOR_CONTROL_APP_ID == msg.appId)
					{
						if (5u != recvCounter)
						{
							msg.payload[recvCounter - RECV_COUNTER_PAYLPAD_MIN_Pos] = cc;
							recvCounter++;
						}
						else
						{
							if (General_recvDataQueue != NULL)
							{
								/* Send a pointer to a struct AMessage object.  Don't block if the
								queue is already full. */
								xQueueSend(General_recvDataQueue, (void*)&msg, (TickType_t)0);
							}
							recvCounter = 0;
						}
					}
					else
					{
						if (General_recvDataQueue != NULL)
						{
							/* Send a pointer to a struct AMessage object.  Don't block if the
							queue is already full. */
							xQueueSend(General_recvDataQueue, (void*)&msg, (TickType_t)0);
						}
						recvCounter = 0;
					}
				}

				break;
			}
		}


	}
}

void Display7Seg_Task(void* pvParameters)
{
	mode_t recvMode_e = modeMonitor;
	errorType_t error_e = errNoError;
	Display7SegMessage_s msg_s = { 0u };
	while (1)
	{

		xSemaphoreTake(General_Display7Seg_BinarySemaphore, portMAX_DELAY); //wait for processing task to send data
		if (xQueueReceive(General_SendTo7segDataQueue,
			&(msg_s),
			(TickType_t)10) == pdPASS)
		{
			error_e = msg_s.error_e;
			recvMode_e = msg_s.mode_e;

			ErrorHandler(error_e);
			printf("Error code: %d", error_e);
			if (errNoError == error_e)
			{

				switch (recvMode_e)
				{
				case modeDrive:
				case modeMonitor:
				case modeSpeed:
				{
					select_7seg_digit(0u);
					set_7seg_digit(hexnum[0u]);
					select_7seg_digit(1u);
					set_7seg_digit(hexnum[recvMode_e]);
					break;
				}
				case modeInvalid:
				{
					select_7seg_digit(0u);
					set_7seg_digit(hexnum[0xfu]);
					select_7seg_digit(1u);
					set_7seg_digit(hexnum[0xfu]);
					break;
				}
				default:
					//received value for unused mode, displayed mode unchanged
					break;
				}

				printf("7Seg message received:\nMode: %d\nError code: %d\n", msg_s.mode_e, msg_s.error_e);
			}
		}
	}
}

void AnalogOutput_Task(void* pvParameters)
{
	AnalogOutputMessage_t msg_s;
	mode_t displayMode_e = modeMonitor;
	sensorType_t sensorType_e = sensInvalid;
	uint16_t sensorData_u16[numOfSensors] = { 0u };
	uint8_t criticalModeActive = (uint8_t)CRITICALMODE_NOT_ACTIVE;

	while (1)
	{
		xSemaphoreTake(General_AnalotOutput_BinarySemaphore, portMAX_DELAY); //wait for processing task to send data
		if (xQueueReceive(General_AnalogOutputDataQueue,
			&(msg_s),
			(TickType_t)10) == pdPASS)
		{
			if ((modeInvalid != msg_s.mode) && (modeUnused != msg_s.mode))
			{
				displayMode_e = msg_s.mode;
			}

			if ((sensInvalid != msg_s.sensor_type))
			{
				sensorType_e = msg_s.sensor_type;
				setSensorValue(sensorType_e, msg_s.sensor_val, sensorData_u16);
			}

			printf("Analog task received data\n");
			printf("\nMessage:\n");
			printf("Mode: %d\nSensor Type: %d\nSensor Value: %d\n\n", displayMode_e, sensorType_e, msg_s.sensor_val);

			criticalModeActive = General_CheckCriticalMode(sensorType_e, msg_s.sensor_val);
			xQueueSend(CriticalModeQueue, (void*)&criticalModeActive, (TickType_t)10);
			if (CRITICALMODE_NOT_ACTIVE == criticalModeActive)
			{
				AnalogOutputHandler(displayMode_e, sensorData_u16);
			}
		}
	}
}

void setSensorValue(sensorType_t sensType_e, uint16_t sensValue_u16, uint16_t* sensData_u16)
{
	if (modeInvalid != sensType_e && modeUnused != sensType_e)
	{
		sensData_u16[sensType_e] = sensValue_u16;
	}
	else
	{
		//should not happen
	}
}

void ErrorHandler(errorType_t error_e)
{
	select_7seg_digit(4u);
	set_7seg_digit(hexnum[0u]);
	select_7seg_digit(5u);
	set_7seg_digit(hexnum[error_e]);
}

void AnalogOutputHandler(mode_t mode_e, uint16_t* sensorData_u16)
{
	uint16_t column1Val_u16 = 0u;
	uint16_t column2Val_u16 = 0u;
	switch (mode_e)
	{
		case modeMonitor:
		{
			//show Cooling Liquid temp and Air intake sensor in Monitor mode 
			column1Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensCoolLiq]);
			column2Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensAirIntake]);
			break;
		}
		case modeDrive:
		{
			//show Rpm and Engine Mode sensor in Drive mode
			column1Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensRpm]);
			column2Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensEngLoad]);
			break;
		}
		case modeSpeed:
		{
			//show Rpm and Acceleration Pedal sensor
			column1Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensRpm]);
			column2Val_u16 = General_MapSensorToAnalogDisplay(sensorData_u16[sensAccPedal]);
			break;
		}
		case modeUnused:
		{
			//ignore the case
			break;
		}
		default:
		{
			//error handling
			General_SendErrorCodeTo7Seg(errModeType);
			break;
		}
	}

	LedBarSetOutputs(column1Val_u16, column2Val_u16);
}

void LedBarSetOutputs(uint16_t col1_u16, uint16_t col2_u16)
{
	(void)set_LED_BAR(0u, ledBarValue[(uint8_t)col1_u16]);
	(void)set_LED_BAR(1u, ledBarValue[(uint8_t)col2_u16]);
}

void CriticalModeTask(void* pvParameters)
{
	const TickType_t xDelay = (uint8_t)CRITICALMODE_BLINK_FEQ / portTICK_PERIOD_MS;
	uint8_t criticalModeActive = (uint8_t)CRITICALMODE_NOT_ACTIVE;
	uint8_t blinkCounter_u8 = 0u;
	for (;;)
	{
		if (xQueueReceive(CriticalModeQueue,
			&(criticalModeActive),
			(TickType_t)10) == pdPASS)
		{
			//receive critical mode information
		}

		if (CRITICALMODE_ACTIVE != criticalModeActive)
		{
			//critical mode not active
		}
		else
		{
			// critical mode active
			blinkCounter_u8 ^= 1;
			if (blinkCounter_u8)
			{
				LedBarSetOutputs(8u, 8u);
			}
			else
			{
				LedBarSetOutputs(0u, 0u);
			}
			
		}
		//blink
		vTaskDelay(xDelay);
	}
}