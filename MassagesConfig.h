/*
$Id: MassagesConfig.h
 */

#ifndef MASSAGES_CONFIG_H
#define MASSAGES_CONFIG_H


#define MAX_MESSAGES 30

#define MAX_SEMAPHORES 10


typedef enum  {
    COMMAND_NO_CMD = 0,
    COMMAND_ENTER,
    COMMAND_EXIT,
	COMMAND_GET_STATUS,
	COMMAND_SET_LINK,
	COMMAND_RESET,
	COMMAND_PWRDN,
	COMMAND_PWRDN_TIMER,
	COMMAND_RESET_DEVICE,
	COMMAND_DEVICE_SLEEP,
	COMMAND_PRINT_PROMT,
	COMMAND_HELP,
	COMMAND_SEND_DATA_GSM,
	COMMAND_SEND_DATA_GSM_GPS_WAIT,	
	COMMAND_MODEM_ON,
	COMMAND_GET_DEBUG_INFO,	
	COMMAND_EVENT_CHANGE_R,
	COMMAND_SET,
	COMMAND_GET,
    COMMAND_UNKNOWN
} COMMANDS;


typedef enum  {	
    SEM_DO_SEND_LONG_STRING = 0, // перелаем строку
    SEM_CONSOL_EN, // включена ли консоль микроконтроллера, если выключена, то пр€мой доступ к модему
    SEM_CONSOL_BUSY, // автоматика зан€ла консоль, ручной ввод заблокировать
    SEM_SEND_STR_SUCC, // прошла ли запись 
    SEM_WAIT_PWD, // ќжидает выключени€
    SEM_DO_MEAS_R,
    SEM_DO_MEAS_T,
    SEM_DO_MEAS_U,
    SEM_UNKNOWN
} SEM;


#endif  //HARDWARE_PROFILE_H
