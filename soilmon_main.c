/****************************************************************/
#include "soilmon_main.h"

/****************************************************************/
EEPROM_INIT

/****************************************************************/
volatile int sys_tick;

DATA_Struct dev_data;
DEBUG_INFO_Stract debug_data;
BYTE g_wdt_flag = WDT_FL_NO;

/****************************************************************/
void tsk_mes_T(void);
void tsk_mes_R(void);
void tsk_UsartProc(void);
void tsk_Cmd(void);
void taskConnect(void);
void taskSendData(void);
void tsk_wdt(void);
void tsk_SMS(void);
void tsk_timers(void);
void task_HLV_get_accu(void);

//volatile int ii;

/****************************************************************/
void main(void) {

    Nop();
    
    //ii = atoi(" 04.10 grn.");
    
	Nop();
	Nop();
	
    InitializeSystem();
    InitMessages();
    
	BSemOn(SEM_CONSOL_EN);    
    
	chk_eeprom_parameters();    

    ei();

    while(1) {	    
	    tsk_timers();
        tsk_mes_T();
        tsk_mes_R(); 
        task_HLV_get_accu();       
		tsk_Cmd();		
		taskSendData();		
		taskConnect();		
		tsk_SMS();
        tsk_UsartProc();        
        tsk_wdt(); 
        
        ProcessMessages();
    }
}



/****************************************************************/
// Задача по инициализации модема и отправке данных на сервер, состоящяя из нескольких команд
// Принимает сообщения
// COMMAND_SEND_DATA_GSM - полный цикл с получением данных GPS и SSD
// TSK_GSM_CHK_CONNECT_TX 

void taskConnect(void) {	

	static BYTE chk_recieve_is_ok_count;
	static BYTE conn_state;	
	static BYTE trans_attempt = 0;
	static WORD conn_atempts = 0;	
	static BYTE sender = COMMAND_NO_CMD;		
	char str[50];
	
		
	OS_TaskBegin();	
	
	SET_WDT_FLAG(WDT_FL4);
	BSemOff(SEM_CONSOL_BUSY);	
	sender = COMMAND_NO_CMD;
	
	
	if(GetMessage(COMMAND_MODEM_ON)) {	
		sender = COMMAND_MODEM_ON;
	}
	
	if(GetMessage(COMMAND_SEND_DATA_GSM)){
		sender = COMMAND_SEND_DATA_GSM;
		strcpy(dev_data.cSender, "Manual  ");
	}
	
	if(GetMessage(COMMAND_SEND_DATA_GSM_GPS_WAIT)){
		sender = COMMAND_SEND_DATA_GSM_GPS_WAIT;
	}	

		
	
	if(sender != COMMAND_NO_CMD) {
		trans_attempt = 0;
		
trans_again:
		BSemOn(SEM_CONSOL_BUSY);
		sprintf(str, "\r\ntaskConnect: %s\r\n", dev_data.cSender);
		putstr(str);		
		//sender = COMMAND_SEND_DATA_GSM_GPS_WAIT;				
		
		conn_atempts = 0;
		trans_attempt = 0;		
		conn_state = 0;
	
		do{	
			conn_atempts ++;
			sprintf(str, "taskConnect: Ask modem %ud atempt\r\n", conn_atempts);
			putstr(str);
			putstr2_slow("AT\r\n");			
			OS_Delay(500);
			// check answer
	        if(SIM908_cmd_get_OK()) {		        
	            putstr((char *)"\r\ntaskConnect: Modem is connected\r\n\r\n");
	            //gps_pwr_is_set = 0;	            
	            conn_state ++;
	            break;          
	        } else {	        	
	            putstr((char *)"\r\ntaskConnect: Modem is't connected. Do reset..\r\n\r\n");
	            SIM908_cmd_reset();	            
	            OS_Delay(20000);
				debug_data.sim_908_resets ++;
				conn_state = 2;				
	        }
  		}  while(conn_atempts < 5);
  		
  		
  		// Проверяем соединение
        if(conn_state == 0){
	        putstr((char *)"\r\ntaskConnect: Modem can't connect. Stop.\r\n\r\n");
	        BSemOff(SEM_CONSOL_BUSY);
	        // Выбегаем
	        OS_Reset();
	    } else 
	    	if(conn_state == 3){
		    	// Если только включено, включаем GPS
		    	putstr2_slow("AT+CGPSPWR=1\r\n");
		    	OS_Delay(500);
		    	putstr2_slow("AT+CGPSRST=1\r\n");
		    	OS_Delay(500);
		    	
	    	}
	    	
	    	// Пытаемся получить координаты
	    	conn_atempts = 0;
	    	do{
		    	SIM908_cmd_clear_buffer();
				putstr2("AT+CGPSSTATUS?\r\n");	
				OS_Delay(5000);
				SET_WDT_FLAG(WDT_FL4);
				if(SIM908_get_GPS_st()){
					break;
				}
			} while(conn_atempts ++ < 40);			
			
			putstr2_slow("AT+CGPSINF=0\r\n");
			OS_WaitTC(SIM908_cmd_get_OK(), 500, 5);
			SIM908_pars_GPS_data();	

			putstr2_slow("AT+CNUM\r\n");
			OS_WaitTC(SIM908_cmd_get_OK(), 500, 5);		
			SIM908_pars_tnum_data();			
			
			putstr2_slow("AT+CUSD=1,^*111#^,15\r\n");
			OS_WaitTC(SIM908_pars_money_data(), 500, 20);
			
			if(sender == COMMAND_MODEM_ON){
				ConsolePromt(&putbyte);
				OS_Reset();
			}
			
			if(atoi(dev_data.cMoney) == 0){
				
				ConsolePromt(&putbyte);
				putstr((char *)"\r\ntaskConnect: No money. Stop\r\n");
				ConsolePromt(&putbyte);
				OS_Delay(1000);
				SendMessage(COMMAND_PWRDN_TIMER);
				OS_Reset();
			}			

			// Даем сигнал передачи строки
			BSemOff(SEM_SEND_STR_SUCC);
			
			BSemOn(SEM_DO_SEND_LONG_STRING);
			OS_Wait(!BSemIs(SEM_DO_SEND_LONG_STRING));
			
			if(BSemIs(SEM_SEND_STR_SUCC)){
				//PROCEDURE_SUCC;
				OS_Delay(500);
				putstr((char *)"\r\ntaskConnect: Transmittion successed!\r\n");
				OS_Delay(1000);
				// Разрешаем SMS
				putstr2("AT+CNMI=1,2,0,0,0\r\n");
				OS_Delay(1000);
				ConsolePromt(&putbyte);
				OS_Delay(1000);
				SendMessage(COMMAND_PWRDN_TIMER);
											
			} else {
				//PROCEDURE_FAULT;
				OS_Delay(20000);	
				putstr((char *)"\r\ntaskConnect: Try again\r\n");
				trans_attempt ++;
				
				// Надо сбросить и выйти
				if(trans_attempt > 2)	{					
					SIM908_cmd_reset();
					putstr((char *)"\r\ntaskConnect: Stop\r\n");
					ConsolePromt(&putbyte);
					OS_Delay(1000);
					SendMessage(COMMAND_PWRDN_TIMER);
					
				} else {
					goto trans_again;
				}
			}			
  	
	    }//COMMAND_SEND_DATA_GSM_GPS_WAIT 
	
	OS_Yield();
	OS_TaskEnd();

}




/****************************************************************/
// Измерение 1 раз в минуту
// Надо проснуться 1 раз в минуту, сделать измерения
// принять решение о передаче по событию или по таймеру
void tsk_timers(void){

	static BYTE state = 0;
	static WORD time_meas = 0;
	static WORD time_send = 0;
	static WORD time_SIM908_sleep = 0;
	char str[60];	
	WORD wparam;
	
	
	if(GetMessage(COMMAND_EVENT_CHANGE_R)) {
		SendMessage(COMMAND_SEND_DATA_GSM_GPS_WAIT);
		strcpy(dev_data.cSender, "Change R");
		return;
	}
	
	if(GetMessage(COMMAND_PWRDN_TIMER)) {				
		BSemOn(SEM_WAIT_PWD);
		EE_TO_RAM(PW_SLEEP_T, wparam);		
		sprintf(str, "\r\nModem will be sleep in %umin. \r\n> ", wparam);
		putstr2_slow(str);
		// Заряжаем таймер сна				
		time_SIM908_sleep = wparam * 6;
		return;   
	}	
	
	
	OS_TaskBegin();	

	
	// по включению - замеры и передача Startup, только 1 раз
	if(state == 0){
		
		// соединяемся с модемом
		SendMessage(COMMAND_GET_STATUS);
					
		BSemOn(SEM_DO_MEAS_T);
		BSemOn(SEM_DO_MEAS_R);	
		BSemOn(SEM_DO_MEAS_U);
		putstr((char *)"\r\n");

		OS_Delay(30000);
		OS_Delay(30000);
		
		if(!BSemIs(SEM_CONSOL_BUSY)){
			SendMessage(COMMAND_SEND_DATA_GSM_GPS_WAIT);
			strcpy(dev_data.cSender, "Startup");				
		}
		
		time_meas = 0;
		time_send = 0;			
		state = 1;	
	}	
	
	
	// Здесь крутимся
	// 10s			
	OS_Delay(10000);			
	time_meas ++;
	time_send ++;
							
	// запускаем 1 раз 1 мин
	if(time_meas >= 6){
		time_meas = 0;				
		BSemOn(SEM_DO_MEAS_T);
		BSemOn(SEM_DO_MEAS_R);
		BSemOn(SEM_DO_MEAS_U);						
	}
	
	
	// запускаем 1 раз в час
	EE_TO_RAM(PW_PER, wparam);
	wparam *= 6;
	// Не запускаем, если 0
	if(wparam != 0) {		
	
		if(time_send >= wparam){
			time_send = 0;					
			SendMessage(COMMAND_SEND_DATA_GSM_GPS_WAIT);
			strcpy(dev_data.cSender, "Timer");
		}					

		// off, if uart1
		if(!BSemIs(SEM_WAIT_PWD)) {
			time_SIM908_sleep = 0;
		}
		
		if(time_SIM908_sleep > 0){
			time_SIM908_sleep --;
			if(time_SIM908_sleep == 0){
				putstr((char *)"\r\nModem is setting power down\r\n");
				SendMessage(COMMAND_PWRDN);
			}
		}			
	}	

	OS_Yield();
	OS_TaskEnd();
	
}

