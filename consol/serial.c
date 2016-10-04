
#include "MainPult.h"


const char str_date[] = __DATE__;
const char str_autor[] = " Vazhko Vladislav©";

/**************************************************************************************************/
//void __attribute__((__interrupt__, auto_psv)) _U2RXInterrupt(void);
//void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void);


//BYTE   RxTxBuff[20];

volatile BYTE   Rx1Buff[20];
volatile BYTE   Rx2Buff[20];

/**************************************************************************************************/
//volatile unsigned char RX_buf[10];
volatile unsigned char timeout_error1;
volatile unsigned char timeout_error2;

/**************************************************************************************************/
void  DelayCy(unsigned long cy){
while (--cy) ClrWdt();
}



/**************************************************************************************************/
void UART1Init(void) {
	
    U1BRG = BAUDRATEREG1;
    U1MODE = 0;
    U1MODEbits.BRGH = BRGH1;
    U1STA = 0;
    U1MODEbits.UARTEN = 1;
    
    U1MODEbits.PDSEL0 = 1; // 9 bits
	U1MODEbits.PDSEL1 = 1;
    
    U1STAbits.UTXEN = 1;	
	IEC0bits.U1RXIE = 1;
	
	
	


}

/**************************************************************************************************/
void UART2Init(void) {
	
    U2BRG = BAUDRATEREG2;
    U2MODE = 0;
    U2MODEbits.BRGH = BRGH2;
    U2STA = 0;
    U2MODEbits.UARTEN = 1;

	
    U2STAbits.UTXEN = 1;
    //U2STAbits.ADDEN = 1;
    
    _U2RXIF = 0;
    _U2RXIE = 1; 
	


}

/**************************************************************************************************/
void UART1PutChar(char byte) {
	
	while(U1STAbits.UTXBF) continue;
	U1TXREG = byte;
	Nop();
	while(U1STAbits.TRMT==0) continue;
	ClrWdt();
}

/**************************************************************************************************/
void UART1PutWord(WORD w) {
	
	while(U1STAbits.UTXBF) continue;
	U1TXREG = w;
	Nop();
	while(U1STAbits.TRMT==0) continue;
	ClrWdt();
}

/**************************************************************************************************/
void UART2PutChar(char byte) {
	
	while(U2STAbits.UTXBF) continue;
	U2TXREG = byte;
	Nop();
	while(U2STAbits.TRMT==0) continue;
	
}


/**************************************************************************************************/
char UART1GetChar() {
	volatile DWORD retry;
	
	retry=0xefff; 
	timeout_error1 = 0;
	do {
		if (U1STAbits.URXDA)	 return (U1RXREG);
		else retry--;	
	} while (retry != 0);
	timeout_error1 = 1;
	return (0);	
}


/**************************************************************************************************/
char UART2GetChar() {
	volatile DWORD retry;
	retry=0xefff; 
	timeout_error2 = 0;
	do {
		if (U2STAbits.URXDA)	 return (U2RXREG);
		//if(IFS1bits.U2RXIF) return (U2RXREG);
		else retry--;	
	} while (retry != 0);
	
	timeout_error2 = 1;
	return (0x33);	
}

/**************************************************************************************************/
// Ïðèåìíèê 1 RS-232
void __attribute__((__interrupt__, auto_psv)) _U1RXInterrupt(void)  {
	char c;
	BYTE i = 0;
	
			
		while (1){
			Rx1Buff[i++] = UART1GetChar();
			if (timeout_error1 == 1) break;
			rx_in_count ++;
		}


rx1_error:;
	IFS0bits.U1RXIF = 0;
	Nop();

}


/**************************************************************************************************/
// Ïðèåìíèê 2 RS-232
void __attribute__((interrupt, shadow, auto_psv)) _U2RXInterrupt(void)  {
	char c;
	BYTE i = 0;
        
	
	while (1) {
		Rx2Buff[i++] = UART2GetChar();
		if (timeout_error2 == 1) break;
		rx2_in_count ++;
	}



rx2_error:;
	_U2RXIF = 0;
	Nop();

}

/******************************************************************************************/
void UART1PutStr(const char *s){
  while(*s) UART1PutChar(*s++);
}

/******************************************************************************************/
void UART2PutStr(const char *s){
  while(*s) UART2PutChar(*s++);
}

/******************************************************************************************/
void UART1PrintString( char *str )
{
    unsigned char c;

    while( (c = *str++) )
        UART1PutChar(c);
}
/******************************************************************************************/
void UART2PrintString( char *str )
{
    unsigned char c;

    while( (c = *str++) )
        UART2PutChar(c);
}


