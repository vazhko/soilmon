#include "spi.h"


#if defined(__XC)
    #include <xc.h>         /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>        /* HiTech General Include File */
#elif defined(__18CXX)
    #include <p18cxxx.h>    /* C18 General Include File */
#endif


/**************************************************************************************************/
WORD_VAL idata;

/**************************************************************************************************/
char spi1_read (char DATA) {
    static BYTE _gie;
    static BYTE _databuf;

    _gie = (BYTE)GIE;
    GIE = 0;
    SSP1BUF = DATA;
    while (SSP1STATbits.BF == 0);
    _databuf = SSP1BUF;
    GIE = (bit)_gie;

    return _databuf;
}


/**************************************************************************************************/
void spi1_init(void){

	//SPI
	SSP1STAT = 0;
	SSP1CON1 = 0x32;

	TRISCbits.TRISC3 = 0;
	TRISCbits.TRISC4 = 1;
	TRISCbits.TRISC5 = 0;

	CS1 = 1;
	CS2 = 1;
	CS3 = 1;
	CS1_TRIS = 0;
	CS2_TRIS = 0;
	CS3_TRIS = 0;

}

/**************************************************************************************************/
void spi1_close(void){
	
	SSP1STAT = 0;
	SSP1CON1 = 0;
	
	CS1 = 1;
	CS2 = 1;
	CS3 = 1;

}
/******************************************************************************/
WORD MCP3204_read(BYTE ch){

	CS3 = 0;

	_delay(5);

	spi1_read(0b00000110);
	idata.v[1] = spi1_read(ch<<6);
	idata.v[0] = spi1_read(0);

	idata.Val &= 0xfff;

	CS3 = 1;

	return idata.Val;

}

/******************************************************************************/
double MCP3204_read_i(WORD count, BYTE ch){
	double res;
	WORD i;
	CS3 = 0;	
	DWORD summ;	

	res = 0;
	summ = 0;	
	for(i = 0; i < count; i ++){
		summ += (DWORD)MCP3204_read(ch);
	}
	
	res = ((double) summ  / (double)count) * 2.5 / (double)0xfff;
	
	return res;

}