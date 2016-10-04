/*
*
*
* Аппаратные функции
*
*
*/

#include "soilmon_main.h"

/****************************************************************/
void InitializeSystem(void) {
	
	CLOCK_INIT();	
	
	Delay240Us();
	
	
//	Nop();
//	Delay100Us(); //101
//	Nop();
//	Delay50Us(); //51
//	Nop();
//	Delay5Us();	//6.2
//	Nop();	
//	Delay65Us();//73
//	Nop();
//	Delay14Us()	;//	14.5
//	Nop();
//	Delay240Us();//238
//	Nop();

	Nop();
	_DelayMs(100);
	Nop();
	
	PWR_KEY_SIM20 = 1;
	PWR_KEY_SIM20_TRIS = 1;

    PWR_KEY_TRIS = 0;

    spi1_init(); 
    
    // Set PORTF as digital I/O
    ADCON1 = 0x0F;
    
    RBPU = 0;

    CH0 = 0;
    CH1 = 0;    
    CS1 = 0;
    CS2 = 1;

    CH0_TRIS = 0;
    CH1_TRIS = 0;
    CS1_TRIS = 0;
    CS2_TRIS = 0;
    
    T1CON = 0b10110001;    
    TMR1IP = 0;
    TMR1IF = 0;
    TMR1IE = 1;
      
    
//    T0CON = 0b10000101;  //64     
//    TMR0IP = 0;
//    TMR0IF = 1;
//    TMR0IE = 1;  

    PEIE = 1;
    IPEN = 1;
    
    TRISC6 = 0;
    TRISC7 = 1;
    uart16_init();
    
    TRISG1 = 0;
    TRISG2 = 1;
    uart16_init2(); 
    
    WUE1 =1;
    WUE2 =1;            
    
    RCIP = 1;
    RCIE = 1;  
    
    RC2IP = 1;
    RC2IE = 1;
    
}

/****************************************************************/
void set_an_ch(BYTE ch) {
	
	// off
	if(ch > 7){
		CS1 = 1;
        CS2 = 1;
        return;
	}
	
    if TESTBIT(ch, 0) {
        CH0 = 1;
    } else {
        CH0 = 0;
    }
    if TESTBIT(ch, 1) {
        CH1 = 1;
    } else {
        CH1 = 0;
    }
    if TESTBIT(ch, 2) {
        CS1 = 1;
        CS2 = 0;
    } else {
        CS1 = 0;
        CS2 = 1;
    }
}

/****************************************************************/
void SIM908_cmd_reset(void) {

    di();
    PWR_KEY = 1;
    _DelayMs(3000);
    PWR_KEY = 0;
    ei();
}

/******************************************************************************************/
void putstr2_slow(const char *s) {
    while (*s) {
	    if(*s == '^') {
		   putbyte2('"'); 
		   s++;
		} else {
	    	putbyte2(*s++);
	 	}   
	    _DelayMs(50);
	}
}


/****************************************************************/
void dev_sleep(void) {
	
			BAUDCONbits.WUE = 1;
			BAUDCON2bits.WUE2 = 0;			

			ClrWdt();
			SWDTEN = 0;
			
			//GIE = 0;			

			IDLEN = 1;
			Sleep();	
			
			//GIE = 1;
			
			Nop();			
			Nop();
}


/****************************************************************/
// Проверка параметров EEPROM
void chk_eeprom_parameters(void){
	    
	WORD wparam;
	
    EE_TO_RAM(PW_PER, wparam);
    if((wparam == 0) || ((10000 >= wparam) && (wparam >= 20))){	
	    Nop();
	} else {
		wparam = 720;
		RAM_TO_EE(PW_PER, wparam);
	}
	
    EE_TO_RAM(PW_POR, wparam);
    if(wparam <= 100){
	    Nop();	        
	} else {
		wparam = 10;
		RAM_TO_EE(PW_POR, wparam);
	}	
}


/****************************************************************/
void tsk_wdt(void){
	
	if(g_wdt_flag == (WDT_FL1 | WDT_FL2 | WDT_FL3 | WDT_FL4 | WDT_FL5)){
		g_wdt_flag = WDT_FL_NO;
		ClrWdt();		
	}
	
	//4 ms to 131s
	if(SWDTEN == 0)	{
		SWDTEN = 1;	
		ClrWdt();
	}
	
	if(BSemIs(SEM_DO_MEAS_T)) return;
	if(BSemIs(SEM_DO_SEND_LONG_STRING)) return;
	if(1){
		IDLEN = 1;
		Sleep();
	}
}
