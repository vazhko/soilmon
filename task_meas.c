
#include "soilmon_main.h"

/****************************************************************/
void tsk_mes_T(void) {
    BYTE i;

    OS_TaskBegin();

    SET_WDT_FLAG(WDT_FL1);


    if(BSemIs(SEM_DO_MEAS_T)) {	    
        ow_set_port(&TRISF, &LATF, &PORTF); 
    
        ds18b20_start_port();
        OS_Delay(800);
        for(i = 0; i < 8; i++) {
            dev_data.T[i] = ds18b20_read_bit(i);
        }
        BSemOff(SEM_DO_MEAS_T);
    }


    OS_Yield();
    OS_TaskEnd();

}


/****************************************************************/
#define RMAX 99999.9
void tsk_mes_R(void) {
    static BYTE ch = 0;
    double U1, U2, U3, I, R;    
    static BYTE begin = 1;
    static int prev_tick;
    BYTE i;
    WORD wparam;

    OS_TaskBegin();

    SET_WDT_FLAG(WDT_FL2);


    if(BSemIs(SEM_DO_MEAS_R)) {
        ch = 0;
        do {
            spi1_init();
            set_an_ch(ch);
            OS_Delay(250);

            U1 = MCP3204_read_i(16, 0);
            U2 = MCP3204_read_i(16, 1);
            U3 = MCP3204_read_i(16, 2);
            
            I = (2.5 - U1) / 10000.0;
            R = fabs((U2 - U3) / I);

            if(R < 0.0) R = 0.0;
            if(R > RMAX) R = RMAX;

            dev_data.R[ch] = R;
            if(begin) {
                dev_data.prev_R[ch] = R;
            }
        } while(++ ch < 8);

        begin = 0;
        
        // проверяем на изменение
        EE_TO_RAM(PW_POR, wparam);
        if(wparam != 0) {
            for(i = 0; i < 8; i ++) {
                if(fabs(100.0 * (dev_data.prev_R [i] - dev_data.R[i]) / dev_data.R[i]) > wparam) {
                    //if((dev_data.prev_R [i] == RMAX) || (dev_data.R [i] == RMAX) || (dev_data.prev_R [i] == 0.0) || (dev_data.R [i] == 0.0))
                    SendMessage(COMMAND_EVENT_CHANGE_R);
                    dev_data.prev_R [i] = dev_data.R[i];
                    //break;
                }
            }
        }

        // все каналы измерялм, сбрсываем флаг
        // спим
        BSemOff(SEM_DO_MEAS_R);

    }


    OS_Yield();
    OS_TaskEnd(); 

}


/******************************************************************************************/
/*
LVV = 0000	2.17
LVV = 0001	2.23
LVV = 0010	2.36
LVV = 0011	2.44
LVV = 0100	2.60
LVV = 0101	2.79
LVV = 06	2.89
LVV = 07	3.12
LVV = 08	3.39
LVV = 09	3.55
LVV = 10	3.71
LVV = 11	3.90
LVV = 12	4.11
LVV = 13	4.33
LVV = 14	4.59
*/
// определение напряжения батарей
void task_HLV_get_accu(void) {

    static BYTE state = 0;
    static int prev_tick;
    static BYTE i_charge = 0;
    double v;


    OS_TaskBegin();
    SET_WDT_FLAG(WDT_FL2);

    if(BSemIs(SEM_DO_MEAS_U)) {
        v = 0.0;
        HLVDIE = 0;
        HLVDCON = 0b10100000;
        i_charge = 5;
        do {
            HLVDEN = 0;
            HLVDCON	&= 0xf0;
            HLVDCON = HLVDCON | (i_charge & 0x0f);
            HLVDEN = 1;
            HLVDIF = 0;
            OS_Delay(250);
            if(HLVDIF) {
                i_charge ++;
                continue;
            }  else {
                HLVDEN = 0;
                switch(i_charge) {
                    default:
                        v = 0.0;
                        break;
                    case 7:
                        v = 2.89;
                        break;
                    case 8:
                        v = 3.12;
                        break;
                    case 9:
                        v = 3.39;
                        break;
                    case 10:
                        v = 3.55;
                        break;
                    case 11:
                        v = 3.71;
                        break;
                    case 12:
                        v = 3.90;
                        break;
                    case 13:
                        v = 4.11;
                        break;
                    case 14:
                        v = 4.33;
                        break;
                }
            }
            BSemOff(SEM_DO_MEAS_U);
            dev_data.fPwrSupply = v;
            break;

        }  while(i_charge < 15);

    }

    OS_Yield();
    OS_TaskEnd();
  
}
