/* 
 * File:   soilmon_main.h
 * Author: Vlad
 *
 * Created on 6 Май 2013 г., 13:00
 */

#ifndef SOILMON_MAIN_H
#define	SOILMON_MAIN_H

#include <xc.h>
#include <GenericTypeDefs.h>
#include <stdio.h>
#include <math.h>


#include "HardwareProfile.h"
#include "messages.h"
#include "MassagesConfig.h"

#include "spi.h"
#include "ow.h"
#include "macro.h"
#include "usart.h"
#include "console.h"
#include "TimeDelay.h"
#include "gps.h"
#include "eeprom.h"

#include	"oss.h"

// Флаги для стожного WDT
typedef enum  {	
    WDT_FL_NO = 0, 
    WDT_FL1 = 1,
    WDT_FL2 = 2, 
    WDT_FL3 = 4,
    WDT_FL4 = 8,
    WDT_FL5 = 16,
    WDT_FL6 = 32,
    WDT_FL7 = 64,
    WDT_FL8 = 128,
} WDT_FL;

#define SET_WDT_FLAG(x)  g_wdt_flag |= (x)

 

extern void InitializeSystem(void);
extern void set_an_ch(BYTE ch);

extern void cmd_reset(void);
extern void putstr2_slow(const char *s);


extern void dev_sleep(void);
extern BYTE dev_slow_(BYTE st);
void chk_eeprom_parameters(void);

extern volatile int sys_tick;


extern DATA_Struct dev_data;
extern DEBUG_INFO_Stract debug_data;
extern BYTE g_wdt_flag;

#endif	/* SOILMON_MAIN_H */

