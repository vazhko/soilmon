#include <string.h>

#include "GenericTypeDefs.h"


#define MAX_BUFFERED_COMMANDS           4           // Must be a power of 2
#define MAX_COMMAND_LENGTH              20

#define SIM908_BUFFER_COUNT           	250


#define TRUE              1
#define FALSE             0
#define IsNum(c)            ((('0' <= c) && (c <= '9')) ? TRUE : FALSE)
#define UpperCase(c)        (('a'<= c) && (c <= 'z') ? c - 0x20 : c)
#define SkipWhiteSpace()    { while (CommandInfo.buffer[CommandInfo.index] == ' ') CommandInfo.index++; }
	
typedef struct {
    char        buffer[MAX_COMMAND_LENGTH];
    BYTE        index;
    BYTE        command;
    BYTE        escFirstChar;
    struct
    {
        BYTE    reading             : 1;
        BYTE    escNeedFirstChar    : 1;
        BYTE    escNeedSecondChar   : 1;
    };
} COMMAND;

typedef struct {
    char        lines[MAX_BUFFERED_COMMANDS][MAX_COMMAND_LENGTH];
    BYTE        oldest;
    BYTE        newest;
    BYTE        showing;
} OLD_COMMANDS;


typedef struct {
	//char cDevId[sizeof("00001")];
	char cLongitude[sizeof("dddd.mmmmmm")]; 
	char cLatitude [sizeof("dddd.mmmmmm")];  
	char cAltitude [sizeof("dddd.dddddd")];
	char cUTCTime  [sizeof("yyyymmddHHMMSS.sss")];
	char cSatNum[sizeof("00")];
	double T[8];	
	double R[8];
	double prev_R[8];
	char cTNum  [sizeof("+380969798813")];
	char cTNumSMS  [sizeof("+380969798813")];
	char cSMSMessage [11];
	char cSMSParam[11];
	char cMoney  [sizeof(" 00.00 grn.")];
	char cSender[9];
	double fPwrSupply;
} DATA_Struct;
	
typedef struct {	
	char *str;
	BYTE size;
} GPS_PARS;	

typedef struct {	
	WORD uart1_errors;
	WORD uart2_errors;
	WORD sim_908_resets;
	WORD sim_908_trans_errors;
	
} DEBUG_INFO_Stract;

extern char   consol_param1[];
extern char   consol_param2[];		

extern void GetConsoleCmd(void (*PutChar)(char), char in_char);
extern void InitializeConsoleCmd(void);
extern void ConsolePromt(void (*PutChar)(char));



BYTE SIM908_recieve_list(char in_char);

void put_void_char(char v);
void put_void_str(char *v);

void SIM908_cmd_reset(void);
BYTE SIM908_cmd_get_OK(void);
BYTE SIM908_get_GPS_st(void);
BYTE SIM908_pars_GPS_data(void);
void SIM908_cmd_clear_buffer(void);
BYTE SIM908_get_trans_st(void);
void replace(char* str, char bad, char good);

BYTE SIM908_pars_tnum_data(void);
BYTE SIM908_pars_money_data(void);
BYTE SIM908_pars_SMS(void);

