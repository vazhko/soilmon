#include "soilmon_main.h"

/****************************************************************/
#define IS_TRANS_GOOD(t, c) 	\
	trans_ok = 0;\
    chk_recieve_is_ok_count = 0;\
    do {\
        OS_Delay(t);\
        if(trans_ok = SIM908_get_trans_st()) {\
			break;\
        }\
    } while(chk_recieve_is_ok_count ++ < c);\
    Nop()
 
#define CHECK_OK(t, c) 	\
    chk_recieve_is_ok_count = 0;\
    do {\
        OS_Delay(t);\
        if(SIM908_cmd_get_OK()) {\
			break;\
        }\
    } while(chk_recieve_is_ok_count ++ < c);\
    Nop()



// отсылка данных на сервер методом GET
// строка очень длинная поэтому разбита на несколько частей
//AT+HTTPPARA="URL","cardfield.com.ua/meteotest/scripts/form_get.php?...
// ждем семафора SEM_DO_SEND_LONG_STRING
void taskSendData(void) {
    static char chk_recieve_is_ok_count;
    static char trans_ok;  // статус удачной передачи +HTTPACTION:0,200,271
    char str_info[20];
    char str[250];
    WORD wparam1, wparam2;
    
    OS_TaskBegin();
    
    
    if(BSemIs(SEM_DO_SEND_LONG_STRING) == 0) {
        return;
    }
   
    
    sprintf(str, "AT+HTTPINIT\r\n");
    putstr2_slow(str);

    CHECK_OK(500, 6);
    sprintf(str, "AT+SAPBR=1,1\r\n");
    putstr2_slow(str);
    
    CHECK_OK(500, 10);
    putstr((char *)"\r\n");
    sprintf(str, "AT+HTTPPARA=^CID^,1\r\n");
    putstr2_slow(str);
   
    CHECK_OK(500, 10);
    EE_TO_BUFF(PC_ID, str_info, 17);
    str_info[16] = '\0';
    sprintf(str, "AT+HTTPPARA=^URL^,^cardfield.com.ua/meteotest/scripts/form_get.php?dev_id=%s", str_info);
    putstr2_slow(str);
    
    OS_Delay(500);
    sprintf(str, "&N=%s&E=%s", dev_data.cLongitude, dev_data.cLatitude);
    putstr2_slow(str);
    
    OS_Delay(500);
    sprintf(str, "&R1=%1.1f&R2=%1.1f&R3=%1.1f&R4=%1.1f&R5=%1.1f&R6=%1.1f&R7=%1.1f&R8=%1.1f",
            dev_data.R[0], dev_data.R[1], dev_data.R[2], dev_data.R[3], dev_data.R[4], dev_data.R[5], dev_data.R[6], dev_data.R[7]);
    putstr2_slow(str);
            
    OS_Delay(500);
    sprintf(str, "&T1=%1.1f&T2=%1.1f&T3=%1.1f&T4=%1.1f&T5=%1.1f&T6=%1.1f&T7=%1.1f&T8=%1.1f",
            dev_data.T[0], dev_data.T[1], dev_data.T[2], dev_data.T[3], dev_data.T[4], dev_data.T[5], dev_data.T[6], dev_data.T[7]);            
    putstr2_slow(str);
    
    OS_Delay(500);
    EE_TO_RAM(PW_PER, wparam1);
    EE_TO_RAM(PW_POR, wparam2);
    sprintf(str, "&message=Event:%s NUM%s M%s U=%1.2fV PER=%u POR=%u^\r\n",
            dev_data.cSender, dev_data.cTNum, dev_data.cMoney, dev_data.fPwrSupply, wparam1, wparam2);
    // Заменяем пробелы на +
    replace(str, ' ', '+');
    putstr2_slow(str);
    
    CHECK_OK(500, 6);
    sprintf(str, "AT+HTTPACTION=0\r\n");
	putstr2_slow(str);
	
	IS_TRANS_GOOD(500, 14);
	sprintf(str, "AT+SAPBR=0,1\r\n");
	putstr2_slow(str);
	
	CHECK_OK(500, 6);	
	sprintf(str, "AT+HTTPTERM\r\n");
	putstr2_slow(str);
	
	CHECK_OK(500, 6);	
	if(trans_ok) BSemOn(SEM_SEND_STR_SUCC);
    BSemOff(SEM_DO_SEND_LONG_STRING);
   
    OS_Yield();
    OS_TaskEnd();

}
