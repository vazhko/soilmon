#include "soilmon_main.h"

#define IN_BUFF_COUNT 250

//	RX1/TX1 - консоль USB
//	RX2/TX2 - модуль sim908
volatile near char RX_buf1[IN_BUFF_COUNT];
volatile near char RX_buf2[IN_BUFF_COUNT];

// счетчики входящих байт
volatile BYTE rx_count1 = 0, rx_count2 = 0, rx_begin1 = 0, rx_begin2 = 0;


/******************************************************************************/
char get_byte1(void);
char get_byte2(void);
void rx_tx_buff_reset(void);

char TxReady1(void);
char TxReady2(void);


/******************************************************************************/
char TxReady1(void){
	if(rx_count1 > 0) return 1; else return 0;
}

/******************************************************************************/
char TxReady2(void){
	if(rx_count2 > 0) return 1; else return 0;
}

/******************************************************************************/
char get_byte1(void) {
    char res = 0;
    if(rx_count1 > 0) {
	    if(rx_begin1 >= rx_count1){
        	res = RX_buf1[rx_begin1 - rx_count1];
     	} else {
     		res = RX_buf1[IN_BUFF_COUNT - rx_count1 + rx_begin1];
     	}  
        rx_count1 --;
    }
    return res;
}

/******************************************************************************/
char get_byte2(void) {
	char res = 0;
    if(rx_count2 > 0) {
	    if(rx_begin2 >= rx_count2){
        	res = RX_buf2[rx_begin2 - rx_count2];
     	} else {
     		res = RX_buf2[IN_BUFF_COUNT - rx_count2 + rx_begin2];
     	}  
        rx_count2 --;
    }
    return res;
}

/******************************************************************************/
void rx_byte1(char in) {
	static char boot_sign = 0;
	
    RX_buf1[rx_begin1] = in;
    if(in == 0xff){
	    boot_sign ++;
	    if(boot_sign > 5) {
		    Nop();
		    Nop();
		    Reset();
		}
	} else {
		boot_sign = 0;
	}
    rx_begin1 ++;
    if(rx_begin1 == IN_BUFF_COUNT) {
        rx_begin1 = 0;
    }
    rx_count1 ++;
}

/******************************************************************************/
void rx_byte2(char in) {

    RX_buf2[rx_begin2] = in;
        
    rx_begin2 ++;
    if(rx_begin2 == IN_BUFF_COUNT) {
        rx_begin2 = 0;
    }
    rx_count2 ++;
}


/******************************************************************************/
void rx_tx_buff_reset(void) {
	
    rx_begin1 = 0;
    rx_begin2 = 0;
    rx_count1  = 0;
    rx_count2  = 0;
}



/****************************************************************/
void tsk_UsartProc(void) {
    char rx1_byte, rx2_byte;
    
    
    while(TxReady2()) {
        RC2IE = 0;
        rx2_byte = get_byte2();
        SIM908_recieve_list(rx2_byte);        
        putbyte(rx2_byte);
        RC2IE = 1;
    }
    
    while(TxReady1()) {
	    if(BSemIs(SEM_WAIT_PWD)) {
		    BSemOff(SEM_WAIT_PWD);
		}
        RCIE = 0;
        rx1_byte = get_byte1();
        if(BSemIs(SEM_CONSOL_EN)) {
            GetConsoleCmd(&putbyte, rx1_byte);
        } else { 
        	GetConsoleCmd(&put_void_char, rx1_byte);           
            putbyte2(rx1_byte);
        }
        RCIE = 1;
    }    

    
    if ((FERR) || (OERR)) {	    
    	CREN = 0;		
    	NOP();
    	debug_data.uart1_errors ++;
    	CREN = 1;    	
    }
    
    if ((FERR2) || (OERR2) || (RC2IF)) {	    
    	CREN2 = 0;		
    	NOP();	
    	debug_data.uart2_errors ++;
    	CREN2 = 1;
    	
    }
    
}
