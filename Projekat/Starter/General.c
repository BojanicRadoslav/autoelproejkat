#include <stdint.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include "queue.h"
#include "semphr.h"
#include "General.h"



static void sensSetVal(uint8_t sensorId_u8, uint16_t sensorVal_u16)
{
	sensorType_t sensor_e = sensInvalid;
	AnalogOutputMessage_t analogOutputMsg_s = { 0u };
	switch (sensorId_u8)
	{
	case SENSCTRL_COOL_LIQ_TEMP_SENS_ID:
	{
		sensor_e = sensCoolLiq;
		break;
	}
	case SENSCTRL_AIR_INTAKE_SENS_ID:
	{
		sensor_e = sensAirIntake;
		break;
	}
	case SENSCTRL_RPM_SENS_ID:
	{
		sensor_e = sensRpm;
		break;
	}
	case SENSCTRL_ENG_LOAD_SENS_ID:
	{
		sensor_e = sensEngLoad;
		break;
	}
	case SENSCTRL_ACC_PEDAL_SENS_ID:
	{
		sensor_e = sensAccPedal;
		break;
	}
	default:
		//Error Handling
		break;
	}

	if (General_PrepareAnalogMessage(&analogOutputMsg_s, modeUnused, sensor_e, sensorVal_u16))
	{
		//send message to AnalogOutput
		General_SendMessageToAnalogOutput(analogOutputMsg_s);
	}
	else
	{
		printf("Sensor data invalid\n");
		General_SendErrorCodeTo7Seg(errSensType);
	}
}

void General_ProcessData()
{
	Message recvMsg = { 0u };

	if (xQueueReceive(General_recvDataQueue,
		&(recvMsg),
		(TickType_t)10) == pdPASS)
	{
		switch (recvMsg.appId)
		{
			case APP_MODE_SELECT_APP_ID:
			{
				General_ModeSel_MsgReceived(recvMsg.payload, recvMsg.length);
				break;
			}
			case APP_SENSOR_CONTROL_APP_ID:
			{
				General_SenseCtrl_MsgReceived(recvMsg.payload, recvMsg.length);
				break;
			}
			default:
			{
				//Error handling
				printf("App ID invalid\n");
				General_SendErrorCodeTo7Seg(errAppId);
				break;
			}
		}
	}


	
}

void General_ModeSel_MsgReceived(uint8_t* payload_u8, uint8_t payloadLen_u8)
{
	printf("Mode select message received\n");

	errorType_t error_e = errNoError;
	Display7SegMessage_s display7SegMsg_s = { 0u };
	uint8_t mode_u8 = modeUnused;

	if (MODESEL_MSG_LEN == payloadLen_u8)
	{
		AnalogOutputMessage_t analogOutputMsg_s = { 0u };
		mode_u8 = payload_u8[MODESEL_MODE_Pos];

		mode_u8 = (numOfModes > mode_u8) ? (mode_t)mode_u8 : modeInvalid;


		//send message to AnalogOutput
		
		if (General_PrepareAnalogMessage(&analogOutputMsg_s, mode_u8, sensUnused, 0u))
		{
			General_SendMessageToAnalogOutput(analogOutputMsg_s);
		}
		else
		{
			error_e = errModeType;
			printf("Error: Invalid mode\n");
		}
		
	}
	else
	{
		printf("Error: received message len %d but expected %d\n", payloadLen_u8, MODESEL_MSG_LEN);
		error_e = errInvalidMsgLen;
	}

	//send message to 7seg display
	General_Prepare7SegMessage(&display7SegMsg_s, error_e, mode_u8);
	General_SendMessageTo7Seg(display7SegMsg_s);
}

void General_SenseCtrl_MsgReceived(uint8_t* payload_u8, uint8_t payloadLen_u8)
{
	printf("Mode sensor control message received\n");
	if (SENSCTRL_MSG_LEN == payloadLen_u8)
	{
		uint16_t sensVal = (uint16_t)(payload_u8[SENSCTRL_DATA_MSB_Pos] << SHIFT8) | (uint16_t)payload_u8[SENSCTRL_DATA_LSB_Pos];
		sensSetVal(payload_u8[SENSCTRL_SENS_ID_Pos], sensVal);
	}
	else
	{
		printf("Error: received message len %d but expected %d\n", payloadLen_u8, SENSCTRL_MSG_LEN);
		General_SendErrorCodeTo7Seg(errInvalidMsgLen);
	}
}

void General_SendMessageTo7Seg(Display7SegMessage_s msg_s)
{
	xQueueSend(General_SendTo7segDataQueue, (void*)&msg_s, (TickType_t)10);
	xSemaphoreGive(General_Display7Seg_BinarySemaphore); //release 7Seg semaphore
}

void General_SendMessageToAnalogOutput(AnalogOutputMessage_t msg_s)
{
	xQueueSend(General_AnalogOutputDataQueue, (void*)&msg_s, (TickType_t)10);
	xSemaphoreGive(General_AnalotOutput_BinarySemaphore); //release AnalogOutput semaphore
}

StdReturn_Type General_Prepare7SegMessage(Display7SegMessage_s* msg_t, errorType_t error_e, mode_t mode_e)
{
	if (modeInvalid == mode_e)
	{
		error_e = errModeType;
	}

	msg_t->error_e = error_e;
	msg_t->mode_e = mode_e;

	return E_OK;
}

StdReturn_Type General_PrepareAnalogMessage(AnalogOutputMessage_t* msg_t, mode_t mode_e, sensorType_t sensType_e, uint16_t sensVal_u16)
{
	StdReturn_Type retVal_u8 = E_NOT_OK;
	if ((modeInvalid != mode_e) && (sensInvalid != sensType_e))
	{
		msg_t->mode = mode_e;
		msg_t->sensor_type = sensType_e;
		msg_t->sensor_val = sensVal_u16;
		retVal_u8 = E_OK;
	}

	return retVal_u8;
}

void General_SendErrorCodeTo7Seg(errorType_t error_e)
{
	Display7SegMessage_s display7SegMsg_s = { 0u };
	General_Prepare7SegMessage(&display7SegMsg_s, error_e, modeUnused);
	General_SendMessageTo7Seg(display7SegMsg_s);
}

uint16_t General_MapSensorToPhysical(sensorType_t sensor_e, uint16_t val_u16)
{
	uint16_t deltaVal_u16 = 0u;
	uint16_t resolution_u16 = 0u;
	uint16_t retVal_u16 = 0u;
	switch (sensor_e)
	{
		case sensCoolLiq:
		{	
			deltaVal_u16 = (uint16_t)SENS_COOL_LIQ_TEMP_MAX - (uint16_t)SENS_COOL_LIQ_TEMP_MIN;
			break;
		}
		case sensAirIntake:
		{
			deltaVal_u16 = (uint16_t)SENS_AIR_INTAKE_MAX - (uint16_t)SENS_AIR_INTAKE_MIN;
			break;
		}
		case sensRpm:
		{
			deltaVal_u16 = (uint16_t)SENS_RMP_SENS_MAX - (uint16_t)SENS_RPM_SENS_MIN;
			break;
		}
		case sensEngLoad:
		{
			deltaVal_u16 = (uint16_t)SENS_ENG_LOAD_MAX - (uint16_t)SENS_ENG_LOAD_MIN;
			break;
		}
		case sensAccPedal:
		{
			deltaVal_u16 = (uint16_t)SENS_ACC_PEDAL_MAX - (uint16_t)SENS_ACC_PEDAL_MIN;
			break;
		}
		default:
		{
			General_SendErrorCodeTo7Seg(errSensType);
			break;
		}
	}

	if (0u != deltaVal_u16)
	{
		resolution_u16 = (uint16_t)UINT16_MAX / deltaVal_u16;
		if (0u != resolution_u16)
		{
			retVal_u16 = val_u16 / resolution_u16;
		}
	}

	return retVal_u16;
}

uint16_t General_MapSensorToAnalogDisplay(uint16_t val_u16)
{
	uint16_t retVal_u16 = 0u;
	uint16_t resolution_u16 = UINT16_MAX / ANALOG_OUTPUT_LEN;
	if(0u != resolution_u16)
	{
		retVal_u16 = val_u16 / resolution_u16;
	}

	return retVal_u16;
}

StdReturn_Type General_CheckCriticalMode(sensorType_t sensor_e, uint16_t val_u16)
{
	uint16_t phyVal_u16 = General_MapSensorToPhysical(sensor_e, val_u16);
	uint16_t criticalVal_u16 = 0u;
	StdReturn_Type retVal_u8 = CRITICALMODE_NOT_ACTIVE;
	switch (sensor_e)
	{
		case sensCoolLiq:
		{
			criticalVal_u16 = (uint16_t)CRITICALMODE_COOL_LIQ_TEMP_CRITICAL;
			break;
		}
		case sensAirIntake:
		{
			criticalVal_u16 = (uint16_t)CRITICALMODE_AIR_INTAKE_CRITICAL;
			break;
		}
		case sensRpm:
		{
			criticalVal_u16 = (uint16_t)CRITICALMODE_RPM_CRITICAL;
			break;
		}
		case sensEngLoad:
		{
			criticalVal_u16 = (uint16_t)CRITICALMODE_ENG_LOAD_CRITICAL;
			break;
		}
		default:
		{
			General_SendErrorCodeTo7Seg(errSensType);
			break;
		}
	}

	if (phyVal_u16 > criticalVal_u16)
	{
		retVal_u8 = CRITICALMODE_ACTIVE;
	}

	return retVal_u8;
}

