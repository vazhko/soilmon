/*
*
*
* Работа с SMS
*
*
*/


#include "soilmon_main.h"


/****************************************************************/
void tsk_SMS(void){
	static int prev_tick;
	static BYTE chk_recieve_is_ok_count = 0, send_cmd;
	char str[100], str2[50];

	// консоль занята задачей выходим
	if(BSemIs(SEM_CONSOL_BUSY)) {
		return;
	}
		
	OS_TaskBegin();
	
	// Ждем прихода признака СМС
	OS_Wait(SIM908_pars_SMS());
	OS_Delay(500);
		
	SIM908_pars_SMS();
	OS_Delay(500);
	
	

	send_cmd = 0;
	if(!strcmp("GET", dev_data.cSMSMessage)) {
		if(!strcmp("T", dev_data.cSMSParam)) {	                
			sprintf(str, "%2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f\r\n", dev_data.T[0],dev_data.T[1],dev_data.T[2],dev_data.T[3],dev_data.T[4],dev_data.T[5],dev_data.T[6],dev_data.T[7]);
            send_cmd = 1;
		} else if(!strcmp("R", dev_data.cSMSParam)) {
			sprintf(str, "%2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f\r\n", dev_data.R[0],dev_data.R[1],dev_data.R[2],dev_data.R[3],dev_data.R[4],dev_data.R[5],dev_data.R[6],dev_data.R[7]);
            send_cmd = 1;		
		} else if(!strcmp("U", dev_data.cSMSParam)) {
			sprintf(str, "U=%1.2fV\r\n", dev_data.fPwrSupply);			
			send_cmd = 1;
		}
				
	} else if(!strcmp("SET", dev_data.cSMSMessage)) {
		if(!strcmp("PER", dev_data.cSMSParam)) {	                
			//sprintf(str, "%2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f\r\n", dev_data.T[0],dev_data.T[1],dev_data.T[2],dev_data.T[3],dev_data.T[4],dev_data.T[5],dev_data.T[6],dev_data.T[7]);
            //send_cmd = 1;
		} else if(!strcmp("POR", dev_data.cSMSParam)) {
			//sprintf(str, "%2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f %2.1f\r\n", dev_data.R[0],dev_data.R[1],dev_data.R[2],dev_data.R[3],dev_data.R[4],dev_data.R[5],dev_data.R[6],dev_data.R[7]);
            //send_cmd = 1;		
		} else if(!strcmp("U", dev_data.cSMSParam)) {
			//sprintf(str, "U=%1.2fV\r\n", dev_data.fPwrSupply);			
			//send_cmd = 1;
		}		
	}
	
	
	
	if(send_cmd == 1){
		send_cmd = 0;
	
		sprintf(str2, "AT+CMGS=^%s^\r\n", dev_data.cTNumSMS);
		putstr2_slow(str2);
		OS_Delay(500);
		//sprintf(str, "SIM908: Mess:%s Raram:%s%c\r\n", dev_data.cSMSMessage, dev_data.cSMSParam, 0x1a);
		//sprintf(str, "%s%c", str, 0x1a);
		putstr2_slow((const char *)str);
		putbyte2(0x1a);
		OS_WaitTC(SIM908_cmd_get_OK(), 500, 20);
	
	}

	ConsolePromt(&putbyte);
	
	
	//OS_Yield();	
	OS_TaskEnd();

}

