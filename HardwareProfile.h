/*
$Id: HardwareProfile.h
 */

#ifndef HARDWARE_PROFILE_H
#define HARDWARE_PROFILE_H

#define USE_USART1
#define USE_USART2

//#define GetSystemClock()		(32000000ul)      // Hz
#define GetSystemClock()		(16000000ul)      // Hz
//#define GetSystemClock()		(8000000ul)      // Hz



#if (GetSystemClock() ==  32000000ul)
	#define CLOCK_INIT() 	{OSCCON = 0b01110000; 	PLLEN = 1;}
	#define RELOAD_T1_VAL -999l
#endif
	
#if (GetSystemClock() ==  16000000ul)	
	#define CLOCK_INIT() 	{OSCCON = 0b01100000; 	PLLEN = 1;}
	#define RELOAD_T1_VAL -499l
#endif

#if (GetSystemClock() ==  8000000ul)
	#define CLOCK_INIT() 	{PLLEN = 0;	OSCCON = 0b01110000; }
	#define RELOAD_T1_VAL -249l	
#endif



#define GetInstructionClock()	(GetSystemClock()/4)
#define GetPeripheralClock()	GetInstructionClock()

#define BAUD_RATE       (9600ul)		// bps
#define BAUD_RATE2      (9600ul)		// bps

//#define BAUD_RATE       (57600ul)		// bps
//#define BAUD_RATE2      (57600ul)		// bps

#define PWR_KEY             LATDbits.LATD7
#define PWR_KEY_TRIS        TRISDbits.TRISD7

#define PWR_KEY_SIM20        	PORTGbits.RG4
#define PWR_KEY_SIM20_TRIS   	TRISGbits.TRISG4



#define CS1 LATD6
#define CS1_TRIS TRISD6
#define CS2 LATD5
#define CS2_TRIS TRISD5
#define CS3 LATB3
#define CS3_TRIS TRISB3

#define CH0 LATD3
#define CH0_TRIS TRISD3
#define CH1 LATD4
#define CH1_TRIS TRISD4



#endif  //HARDWARE_PROFILE_H
