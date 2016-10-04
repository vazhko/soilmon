#include "soilmon_main.h"

extern void rx_byte1(char in);
extern void rx_byte2(char in);

//int T1_Val = -499;


/******************************************************************************/
void interrupt low_priority LOW_ISR(void) {

    //Nop();

    if((TMR1IF) && (TMR1IE)) {  // 1mS
        WRITETIMER1(RELOAD_T1_VAL);
        TMR1IF = 0;
        sys_tick ++;
        OS_Tick();
        NOP();

    }
  
}

/******************************************************************************/
static void interrupt HI_ISR(void) {

    if(RCIF) {
        rx_byte1(getbyte());
    }

    if(RC2IF && RC2IE) {	    
        rx_byte2(getbyte2());
    }
}


/****************************************************************/
/*
BYTE dev_slow_(BYTE st){
	
	static BYTE state = 0;
	BYTE res = 0;
	
	res = state;
	di();	
	switch (st){
		default:			
		case 0:
			CREN2 = 0;
			T1_Val = -499l;
			// 16Mhz
			OSCCON = 0b01100000; 	
			PLLEN = 1;
			CREN2 = 1;
			RC2IE = 1;			
		break;
		case 1:
			T1_Val = -31l;
			// 1Mhz
			PLLEN = 0;
			OSCCON = 0b01000000; 
			RC2IE = 0;
		break;		
	}
	
	//while(!IOFS);	
	 _DelayMs(200);
	state = st;
	ei();
	
	return res;
}
*/