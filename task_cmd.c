/*
*
*
* ќбработка команд с консоли и не только
*
*
*/



#include "soilmon_main.h"


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


char noparam_str [] = " - Error: parameter(s) required\r\n";
char unparam_str [] = "\x1b[31m - Error: Unsupported parameter\x1b[0m\r\n";

#define TP_ANSI_FG_BLUE  "\x1b[34m"
#define TP_ANSI_FG_RED  "\x1b[31m"
#define TP_ANSI_FG_GREEN  "\x1b[32m"
#define TP_ANSI_FG_YELLOW  "\x1b[33m"

#define TP_ANSI_BOLD_ON "\x1b[1m"
#define TP_ANSI_RESET  "\x1b[0m"
/****************************************************************/
// «адача по по приему сообщений от консоли дл€ работы с модемом и контроллером в режиме один запрос - ответ
void tsk_Cmd(void) {

    char str[60], i;
    char   consol_param[MAX_COMMAND_LENGTH];
    WORD wparam;
    static char chk_recieve_is_ok_count;

    SET_WDT_FLAG(WDT_FL3);

    // консоль зан€та задачей выходим
    if(BSemIs(SEM_CONSOL_BUSY)) {
        return;
    }

    //------------------------------------
    if(GetMessage(COMMAND_ENTER)) {
        BSemOn(SEM_CONSOL_EN);
        putstrc((char *)"\r\nEnter mode on\r\n");
        ConsolePromt(&putbyte);
        return;
    }

    //------------------------------------
    if(GetMessage(COMMAND_EXIT)) {
        BSemOff(SEM_CONSOL_EN);
        putstrc((char *)"Enter mode off\r\n");
        return;
    }

    //------------------------------------
    if(!BSemIs(SEM_CONSOL_EN)) return;

    //------------------------------------
    OS_TaskBegin();
    Nop();

    //------------------------------------
    if(GetMessage(COMMAND_UNKNOWN)) {
        putstrc((char *)"Unknown command\r\n");        
    }

    //------------------------------------
    if(GetMessage(COMMAND_GET_STATUS)) {
        Nop();
        SIM908_cmd_clear_buffer();
        putstr2("AT\r\n");
        OS_Delay(500);
        Nop();
        if(SIM908_cmd_get_OK()) {
            putstr((char *)"Connected\r\n");
        } else {
            putstr((char *)"Disconnected\r\n");
        }
        ConsolePromt(&putbyte);
        //OS_Reset();
        OS_Yield();
    }

    //------------------------------------
    if(GetMessage(COMMAND_RESET)) {
        SIM908_cmd_reset();
        putstrc((char *)"Reset SIM 908\r\n");
    }

    //------------------------------------
    if(GetMessage(COMMAND_DEVICE_SLEEP)) {
        putstrc((char *)"Now is sleeping\r\n");
        OS_Delay(500);
        dev_sleep();
    }

    //------------------------------------
    if(GetMessage(COMMAND_PWRDN)) {
        putstr2("AT+CNMI=0,2,0,0,0 \r\n");
        OS_Delay(500);
        putstr2_slow("AT+CPOWD=1\r\n");
        OS_Delay(500);
        ConsolePromt(&putbyte);
    }

    //------------------------------------
    if(GetMessage(COMMAND_RESET_DEVICE)) {
        Nop();
        if(SWDTEN == 1) {
            putstrc((char *)"Reset by WDT\r\n");
            OS_Delay(500);
            while(1);
        } else {
            putstrc((char *)"Reset by cmd\r\n");
            OS_Delay(500);
            Reset();
        }
    }

    //------------------------------------
    if(GetMessage(COMMAND_SET)) {

        if(consol_param1[0] == 0) {
            putstrc(noparam_str);
        } else {
            // ”становка ID
            if(!strcmp("ID", consol_param1)) {

                if(consol_param2[0] == 0) {
                    putstrc(noparam_str);

                } else {
                    consol_param2[17] = '\0';
                    BUFF_TO_EE(PC_ID, consol_param2, 16);
                    sprintf(str, "SET: ID is %s\r\n", consol_param2);
                    putstrc(str);
                }

                // ”становка периода
            } else if(!strcmp("PER", consol_param1)) {

                if(consol_param2[0] == 0) {
                    putstrc(noparam_str);
                } else {
                    wparam = atoi(consol_param2);
                    if((wparam == 0) || ((10000 >= wparam) && (wparam >= 10))) {
                        RAM_TO_EE(PW_PER, wparam);
                        sprintf(str, "SET: PERIOD is %umin\r\n", wparam);
                        putstrc(str);
                    } else {
                        putstrc((char *)"PERIOD must be 0 or 10..10000min\r\n");
                    }
                }

                // ”становка порога
            } else if(!strcmp("POR", consol_param1)) {

                if(consol_param2[0] == 0) {
                    putstrc(noparam_str);

                } else {
                    wparam = atoi(consol_param2);

                    if(wparam <= 100) {
                        RAM_TO_EE(PW_POR, wparam);
                        sprintf(str, "SET: POROG is %u\r\n", wparam);
                        putstrc(str);
                    } else {
                        putstrc((char *)"POROG must be 0..100%%\r\n");
                    }
                }


            } else if(!strcmp("?", consol_param1)) {
                putstrc((char *)"SET ID(string 1..16), PER(0, 10..10000min), POR(0..100perc)\r\n");
            } else {
                putstrc(unparam_str);
            }
        }
        ConsolePromt(&putbyte);
        OS_Yield();
    }

    //------------------------------------
    if(GetMessage(COMMAND_GET)) {
        if(consol_param1[0] == 0) {
            putstrc(noparam_str);
        } else {
            if(!strcmp("R", consol_param1)) {
                BSemOn(SEM_DO_MEAS_R);
                OS_Wait(!BSemIs(SEM_DO_MEAS_R));
                for(i = 0; i < 8; i ++) {
                    sprintf(str, "%2.1f ", dev_data.R[i]);
                    putstr((char *)TP_ANSI_FG_GREEN);
                    putstrc(str);
                    putstr((char *)TP_ANSI_RESET);
                }
                putstrc((char *)"\r\n");

            } else if(!strcmp("T", consol_param1)) {
                BSemOn(SEM_DO_MEAS_T);
                OS_Wait(!BSemIs(SEM_DO_MEAS_T));
                
                for(i = 0; i < 8; i ++) {	                
                    sprintf(str, "%2.1f ", dev_data.T[i]);
                    putstr((char *)TP_ANSI_FG_GREEN);
                    putstrc(str);
                    putstr((char *)TP_ANSI_RESET);
                }
                putstrc((char *)"\r\n");

            } else if(!strcmp("U", consol_param1)) {
                BSemOn(SEM_DO_MEAS_U);
                OS_Wait(!BSemIs(SEM_DO_MEAS_U));
                sprintf(str, "U=%1.2fV\r\n", dev_data.fPwrSupply);
                putstr((char *)TP_ANSI_FG_GREEN);
                putstrc(str);
                putstr((char *)TP_ANSI_RESET);

            } else if((!strcmp("TN", consol_param1)) || (!strcmp("NUM", consol_param1))) {
                SIM908_cmd_clear_buffer();
                putstr2_slow("AT+CNUM\r\n");
                OS_Delay(1000);
                SIM908_pars_tnum_data();
                putstr((char *)TP_ANSI_FG_YELLOW);
                putstr(dev_data.cTNum);
                putstr((char *)TP_ANSI_RESET);
                putstr((char *)"\r\n");


            } else if(!strcmp("MON", consol_param1)) {
                Nop();
                SIM908_cmd_clear_buffer();
                putstr2_slow("at+cusd=1,^*111#^,15\r\n");

                chk_recieve_is_ok_count = 0;
                do {
                    OS_Delay(500);
                    if(SIM908_pars_money_data()) {
	                    putstr((char *)TP_ANSI_FG_YELLOW);
                        putstr(dev_data.cMoney);
                        putstr((char *)TP_ANSI_RESET);
                        putstr((char *)"\r\n");
                        break;
                    }
                } while(chk_recieve_is_ok_count ++ < 20);


            } else if(!strcmp("GPS", consol_param1)) {
                SIM908_cmd_clear_buffer();
                putstr2_slow("AT+CGPSSTATUS?\r\n");
                OS_Delay(500);
                if(SIM908_cmd_get_OK()) {
                    putstr2_slow("AT+CGPSINF=0\r\n");
                    OS_Delay(500);
                } else {
                    putstr((char *)"Modem is off\r\n");
                }


            } else if(!strcmp("ID", consol_param1)) {
                Nop();
                EE_TO_BUFF(PC_ID, consol_param, 17);
                consol_param[16] = '\0';
                sprintf(str, "GET: ID is %s\r\n", consol_param);
                putstrc(str);

            } else if(!strcmp("PER", consol_param1)) {
                Nop();
                EE_TO_RAM(PW_PER, wparam);
                sprintf(str, "GET: PER is %umin\r\n", wparam);
                putstrc(str);

            }   else if(!strcmp("POR", consol_param1)) {
                Nop();
                EE_TO_RAM(PW_POR, wparam);
                sprintf(str, "GET: POR is %u%%\r\n", wparam);
                putstrc(str);

            } else if(!strcmp("?", consol_param1)) {
                putstrc((char *)"GET - T, R, U, TN, MON, ID, PER, POR\r\n");

            } else {
                putstrc(unparam_str);
            }
        }

        ConsolePromt(&putbyte);
        OS_Yield();
    } // GET

    //------------------------------------
    if(GetMessage(COMMAND_HELP)) {
        putstrc((char *)"STATUS\r\n");
        putstrc((char *)"GET - T, R, U, TN, MON, ID, PER, POR, (G)GPS\r\n");
        putstrc((char *)"SET - ID, PER, POR\r\n");
        putstrc((char *)"RESET - reset SIM908\r\n");
        putstrc((char *)"RESETD - reset entire device\r\n");
        putstrc((char *)"ENTER in controller consol\r\n");
        putstrc((char *)"EXIT in SIM908 consol\r\n");
        putstrc((char *)"SENDD - send data\r\n");        
        putstrc((char *)"DEBUG - get debug info\r\n");
        putstrc((char *)"PWRON - Modem ON\r\n");
        putstrc((char *)"PWRDN - Modem OFF\r\n");
        putstrc((char *)"SLEEP - Device Sleep\r\n");
        //ConsolePromt(&putbyte);
    }
    
    //------------------------------------
    if(GetMessage(COMMAND_PRINT_PROMT)) {
        ConsolePromt(&putbyte);
    }

    OS_Yield();
    OS_TaskEnd();

}