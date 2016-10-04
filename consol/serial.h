#ifndef _SERIAL_H
#define __SERIAL_H
    
    
//******************************************************************************
// Constants
//******************************************************************************

//U2BRG register value and baudrate mistake calculation

#define BAUDRATEREG1        (((GetSystemClock()/2)+(BRG_DIV2/2*BAUDRATE1))/BRG_DIV2/BAUDRATE1-1)
#define BAUDRATEREG2        (((GetSystemClock()/2)+(BRG_DIV2/2*BAUDRATE2))/BRG_DIV2/BAUDRATE2-1)

/**************************************************************************************************/   
/**************************************************************************************************/

void UART1Init(void);
void UART2Init(void);

void UART1PutChar(char ) ;
void UART2PutChar(char ) ;

void UART1PutWord(WORD );

char UART1GetChar();
char UART2GetChar();

void UART1PutStr(const char *);
void UART2PutStr(const char *);

void UART1PrintString( char * );
void UART2PrintString( char * );





#endif
