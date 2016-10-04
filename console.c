/*
*
* Парсеры
*
*/

#include "console.h"

#include "usart.h"
#include "TimeDelay.h"
#include "messages.h"

COMMAND             CommandInfo;
OLD_COMMANDS        commandBuffer; // буффер команд

char   consol_param1[MAX_COMMAND_LENGTH];
char   consol_param2[MAX_COMMAND_LENGTH];


/**********************************************************************************************************************/
void MonitorUser(void (*PutChar)(char), char in_char);

void EraseCommandLine(void (*PutChar)(char));
void ReplaceCommandLine(void (*PutChar)(char));
BYTE GetCommand(void);
void GetOneWord(char *buffer);
void GetConsol_Param(char *buffer, BYTE num);

/**********************************************************************************************************************/
char sim908_buffer[SIM908_BUFFER_COUNT];
char sim908_prev_buffer[SIM908_BUFFER_COUNT];

/**********************************************************************************************************************/
void GetConsoleCmd(void (*PutChar)(char), char in_char) {

    MonitorUser(PutChar, in_char);
    if(GetCommand()) {
        SkipWhiteSpace();

        SendMessage(CommandInfo.command);
        SendMessage(COMMAND_PRINT_PROMT);
        InitializeConsoleCmd();
    }
}

/****************************************************************************
  Function:
	BYTE GetCommand( void )

  Description:
	This function returns whether or not the user has finished entering a
	command.  If so, then the command entered by the user is determined and
	placed in CommandInfo.command.  The command line index
	(CommandInfo.index) is set to the first non-space character after the
	command.

  Precondition:
	CommandInfo.reading must be valid.

  Parameters:
	None

  Return Values:
	TRUE    - The user has entered a command.  The command is in
				  CommandInfo.command.
	FALSE   - The user has not finished entering a command.

  Remarks:
	None
  ***************************************************************************/

BYTE GetCommand(void) {

    char    firstWord[MAX_COMMAND_LENGTH];

    if(CommandInfo.reading) {
        return FALSE;
    } else {
        CommandInfo.index = 0;

        GetOneWord(firstWord);
        SkipWhiteSpace();

        if(firstWord[0] == 0) {
            CommandInfo.command = COMMAND_NO_CMD;
            return TRUE;
        }

        GetOneWord(consol_param1);
        SkipWhiteSpace();
        GetOneWord(consol_param2);

        if(!strncmp(firstWord, "STAT", 4) && (strlen(firstWord) == 4)) {
            CommandInfo.command = COMMAND_GET_STATUS;
            return TRUE;
        }

        if(!strncmp(firstWord, "LINK", 4) && (strlen(firstWord) == 4)) {
            CommandInfo.command = COMMAND_SET_LINK;
            return TRUE;
        }
        if(!strncmp(firstWord, "RESET", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_RESET;
            return TRUE;
        }
        if(!strncmp(firstWord, "RESETD", 6) && (strlen(firstWord) == 6)) {
            CommandInfo.command = COMMAND_RESET_DEVICE;
            return TRUE;
        }
        if(!strncmp(firstWord, "ENTER", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_ENTER;
            return TRUE;
        }
        if(!strncmp(firstWord, "EXIT", 4) && (strlen(firstWord) == 4)) {
            CommandInfo.command = COMMAND_EXIT;
            return TRUE;
        }
        if(!strncmp(firstWord, "HELP", 4) && (strlen(firstWord) == 4)) {
            CommandInfo.command = COMMAND_HELP;
            return TRUE;
        }

        if(!strncmp(firstWord, "SENDD", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_SEND_DATA_GSM;
            return TRUE;
        }
        if(!strncmp(firstWord, "DEBUG", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_GET_DEBUG_INFO;
            return TRUE;
        }
        if(!strncmp(firstWord, "PWRDN", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_PWRDN;
            return TRUE;
        }
        if(!strncmp(firstWord, "PWRON", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_MODEM_ON;
            return TRUE;
        }
        if(!strncmp(firstWord, "SLEEP", 5) && (strlen(firstWord) == 5)) {
            CommandInfo.command = COMMAND_DEVICE_SLEEP;
            return TRUE;
        }
        if(!strncmp(firstWord, "SET", 3) && (strlen(firstWord) == 3)) {
            CommandInfo.command = COMMAND_SET;
            return TRUE;
        }
        if(!strncmp(firstWord, "GET", 3) && (strlen(firstWord) == 3)) {
            CommandInfo.command = COMMAND_GET;
            return TRUE;
        }
        CommandInfo.command = COMMAND_UNKNOWN;
        return TRUE;
    }
}


/****************************************************************************
  Function:
	void GetOneWord( char *buffer )

  Description:
	This function copies the next word in the command line to the specified
	buffer.  Word deliniation is marked by a space character.  The returned
	word is null terminated.

  Precondition:
	commandInfo.buffer and commandInfo.index are valid

  Parameters:
	*buffer - Pointer to where the word is to be stored.

  Returns:
	None

  Remarks:

  ***************************************************************************/
void GetOneWord(char *buffer) {
    SkipWhiteSpace();

    while((CommandInfo.buffer[CommandInfo.index] != 0) &&
            (CommandInfo.buffer[CommandInfo.index] != ' ')) {
        *buffer++ = CommandInfo.buffer[CommandInfo.index++];
    }
    *buffer = 0;
}

/**********************************************************************************************************************/
void MonitorUser(void (*PutChar)(char), char in_char) {
    char    oneChar;

    oneChar = in_char; //GetChar();

    // If we are currently processing a command, throw the character away.
    if(CommandInfo.reading) {
        if(CommandInfo.escNeedSecondChar) {
            if(CommandInfo.escFirstChar == 0x5B) {
                if(oneChar == 0x41) {		   // Up arrow
                    if(commandBuffer.showing != commandBuffer.oldest) {
                        if(commandBuffer.showing == MAX_BUFFERED_COMMANDS) {
                            commandBuffer.showing = commandBuffer.newest;
                        } else {
                            commandBuffer.showing = (commandBuffer.showing - 1) & (MAX_BUFFERED_COMMANDS - 1);
                        }
                    }
                    ReplaceCommandLine(PutChar);
                } else if(oneChar == 0x42) {	// Down arrow
                    if(commandBuffer.showing != MAX_BUFFERED_COMMANDS) {
                        if(commandBuffer.showing != commandBuffer.newest) {
                            commandBuffer.showing = (commandBuffer.showing + 1) & (MAX_BUFFERED_COMMANDS - 1);
                            ReplaceCommandLine(PutChar);
                        } else {
                            EraseCommandLine(PutChar);
                            commandBuffer.showing = MAX_BUFFERED_COMMANDS;
                        }
                    } else {
                        EraseCommandLine(PutChar);
                    }
                }
            }
            CommandInfo.escNeedSecondChar   = FALSE;
        } else if(CommandInfo.escNeedFirstChar) {
            CommandInfo.escFirstChar        = oneChar;
            CommandInfo.escNeedFirstChar    = FALSE;
            CommandInfo.escNeedSecondChar   = TRUE;
        } else {
            if(oneChar == 0x1B) {	   // ESC - an escape sequence
                CommandInfo.escNeedFirstChar = TRUE;
            } else if(oneChar == 0x08) {	  // Backspace
                if(CommandInfo.index > 0) {
                    CommandInfo.index --;
                    PutChar(0x08);
                    PutChar(' ');
                    PutChar(0x08);
                }
            } else if((oneChar == 0x0D) || (oneChar == 0x0A)) {
                //PrintString((char*)"\r\n");
                PutChar('\r');
                PutChar('\n');
                CommandInfo.buffer[CommandInfo.index]   = 0; // Null terminate the input command
                CommandInfo.reading                     = FALSE;
                CommandInfo.escNeedFirstChar            = FALSE;
                CommandInfo.escNeedSecondChar           = FALSE;

                // Copy the new command into the command buffer
                commandBuffer.showing = MAX_BUFFERED_COMMANDS;
                if(commandBuffer.oldest == MAX_BUFFERED_COMMANDS) {
                    commandBuffer.oldest = 0;
                    commandBuffer.newest = 0;
                } else {
                    commandBuffer.newest = (commandBuffer.newest + 1) & (MAX_BUFFERED_COMMANDS - 1);
                    if(commandBuffer.newest == commandBuffer.oldest) {
                        commandBuffer.oldest = (commandBuffer.oldest + 1) & (MAX_BUFFERED_COMMANDS - 1);
                    }
                }
                strcpy(&(commandBuffer.lines[commandBuffer.newest][0]), CommandInfo.buffer);
            } else if((0x20 <= oneChar) && (oneChar <= 0x7E)) {
                oneChar = UpperCase(oneChar);	// To make later processing simpler
                if(CommandInfo.index < MAX_COMMAND_LENGTH) {
                    CommandInfo.buffer[CommandInfo.index++] = oneChar;
                }
                PutChar(oneChar);	     // Echo the character
                //кирилица
            } /* else if (('А' <= oneChar) && (oneChar <= 'я')) {

					if (CommandInfo.index < MAX_COMMAND_LENGTH) {
						CommandInfo.buffer[CommandInfo.index++] = oneChar;
					}
					PutChar( oneChar );	   // Echo the character
				}
				*/
        }
    }
}

/****************************************************************************
  Function:
	void ReplaceCommandLine( void )

  Description:
	This function is called when the user presses the arrow keys to scroll
	through previous commands.  The function erases the current command line
	and replaces it with the previous command indicated by
	commandBuffer.showing.

  Precondition:
	The buffer of old commands is valid.

  Parameters:
	None

  Returns:
	None

  Remarks:
	None
  ***************************************************************************/

void ReplaceCommandLine(void (*PutChar)(char)) {
    BYTE    i;
    BYTE    lineLength;
    char    oneChar;

    EraseCommandLine(PutChar);

    lineLength = strlen(commandBuffer.lines[commandBuffer.showing]);
    for(i = 0; i < lineLength; i++) {
        oneChar = commandBuffer.lines[commandBuffer.showing][i];
        PutChar(oneChar);
        CommandInfo.buffer[CommandInfo.index++] = oneChar;
    }
}

/****************************************************************************
  Function:
	void EraseCommandLine( void )

  Description:
	This function erases the current command line.  It works by sending a
	backspace-space-backspace combination for each character on the line.

  Precondition:
	CommandInfo.index must be valid.

  Parameters:
	None

  Returns:
	None

  Remarks:
	None
  ***************************************************************************/

void EraseCommandLine(void (*PutChar)(char)) {
    BYTE    i;
    BYTE    lineLength;

    lineLength = CommandInfo.index;
    for(i = 0; i < lineLength; i++) {
        PutChar(0x08);
        PutChar(' ');
        PutChar(0x08);
    }
    CommandInfo.index = 0;
}


/****************************************************************************
  Function:
	void InitializeCommand( void )

  Description:
	This function prints a command prompt and initializes the command line
	information.  If available, the command prompt format is:
					[Volume label]:[Current directory]>

  Precondition:
	None

  Parameters:
	None

  Returns:
	None

  Remarks:
	None
  ***************************************************************************/

void InitializeConsoleCmd(void) {

    CommandInfo.command     = COMMAND_NO_CMD;
    CommandInfo.index       = 0;
    CommandInfo.reading     = TRUE;
    memset(CommandInfo.buffer, 0x00, MAX_COMMAND_LENGTH);
}

/******************************************************************************/
extern void ConsolePromt(void (*PutChar)(char)) {
    // "> "
    PutChar('>');
    PutChar(' ');
}

// Заглушки
/******************************************************************************/
void put_void_char(char v) {
	
}

/******************************************************************************/
void put_void_str(char *v) {
	
}


/******************************************************************************/
//extern BYTE skip;
BYTE SIM908_recieve_list(char in_char) {
    static BYTE in_count = 0;

    if((in_char == 0x0D)) {
        // пропускаем
    } else if(in_char == 0x0a) {
        // терменируем строку
        if(in_count != 0) {
            sim908_buffer[in_count] = '\0';
        }
        // обнуляем указатель
        in_count = 0;

    } else {
        if(in_count == 0) {
            // сохраняем предыдущую строку
            strcpy(&sim908_prev_buffer[0], &sim908_buffer[0]);
            //sim908_prev_buffer[SIM908_BUFFER_COUNT - 1] = '\0';
        }
        sim908_buffer[in_count++] = in_char;
    }

    // reset owerloaded
    if(in_count >= SIM908_BUFFER_COUNT) {
        in_count = 0;
        sim908_buffer[0] = '\0';
        Nop();
    }

    return 1;
}

/****************************************************************/
BYTE SIM908_get_GPS_st(void) {


    if(!strncmp(sim908_prev_buffer, "+CGPSSTATUS: Location 2D Fix", 28)) return 1;
    if(!strncmp(sim908_prev_buffer, "+CGPSSTATUS: Location 3D Fix", 28)) return 2;

    Nop();

    return 0;
}

/****************************************************************/
BYTE SIM908_get_trans_st(void) {

    if(!strncmp(sim908_buffer, "+HTTPACTION:0,200,", 18)) return 1;
    Nop();
    return 0;
}

/****************************************************************/
void SIM908_cmd_clear_buffer(void) {
    BYTE i;
    for(i = 0; i < sizeof(sim908_buffer); i ++) {
        sim908_buffer[i] = 0;
        sim908_prev_buffer[i] = 0;
    }

}

/****************************************************************/
BYTE SIM908_cmd_get_OK(void) {
    if(!strncmp(sim908_buffer, "OK", 2)) return 1;
    else return 0;
}

/****************************************************************/
// Парсер GPS
extern DATA_Struct dev_data;

const GPS_PARS gps_pars[] = {
    {dev_data.cLongitude, sizeof("dddd.mmmmmm")},
    {dev_data.cLatitude, sizeof("dddd.mmmmmm")},
    {dev_data.cAltitude, sizeof("dddd.dddddd")},
    {dev_data.cUTCTime, sizeof("yyyymmddHHMMSS.sss")},
    {NULL, 20},
    {dev_data.cSatNum, sizeof("00")}
};

BYTE SIM908_pars_GPS_data(void) {
    BYTE pos_str_in, pos_str_out, i;
    char str[20];

    if(strncmp(sim908_prev_buffer, "0,", 2)) return 0;

    pos_str_in = 2;

    for(i = 0; i < 6; i ++) {
        pos_str_out = 0;
        while(sim908_prev_buffer[pos_str_in] != ',') {
            str[pos_str_out ++] = sim908_prev_buffer[pos_str_in++];
            if(pos_str_in >= SIM908_BUFFER_COUNT) return 0;
            if(pos_str_out > gps_pars[i].size) return 0;
        }
        str[pos_str_out] = '\0';
        strcpy(gps_pars[i].str, str);
        pos_str_in++;
    }


    Nop();
    return 1;
}

/****************************************************************/
// получение номера
// AT+CNUM
// +CNUM: "My telephone","+380969798813",145,7,4

BYTE SIM908_pars_tnum_data(void) {

    BYTE pos_str_out, s_count = 14;

    if(strncmp(sim908_prev_buffer, "+CNUM:", 6)) return 0;

    pos_str_out = 6;
    while(strncmp(&sim908_prev_buffer[pos_str_out], "+380", 4)) {
        pos_str_out ++;
        Nop();
    }

    strncpy(dev_data.cTNum, &sim908_prev_buffer[pos_str_out], s_count);
    dev_data.cTNum[s_count - 1] = '\0';

    return 1;
}

/****************************************************************/
// получение денег на счету
// at+cusd=1,"*111#",15
// +CUSD: 0,"Na schetu 27.00 grn. Bonusy: *100#",64

BYTE SIM908_pars_money_data(void) {

    BYTE pos_str_out, s_count;

    s_count = 12;

    if(strncmp(sim908_buffer, "+CUSD:", 6)) return 0;

    pos_str_out = 6;
    while(strncmp(&sim908_buffer[pos_str_out], "grn.", 4)) {
        pos_str_out ++;
        Nop();
    }

    // возврящаемся
    pos_str_out -= 7;

    strncpy(dev_data.cMoney, &sim908_buffer[pos_str_out], s_count);
    dev_data.cMoney[s_count - 1] = '\0';

    Nop();
    Nop();

    return 1;
}


/****************************************************************/

BYTE SIM908_pars_SMS(void) {

    BYTE pos_str_out, s_count, in, out;

    if(strncmp(sim908_prev_buffer, "+CMT:", 5)) return 0;

    s_count = 14;
    pos_str_out = 5;
    while(strncmp(&sim908_prev_buffer[pos_str_out], "+380", 4)) {
        pos_str_out ++;
        Nop();
    }

    strncpy(dev_data.cTNumSMS, &sim908_prev_buffer[pos_str_out], s_count);
    dev_data.cTNumSMS[s_count - 1] = '\0';

    Nop();
    Nop();
    
    out = 0;
    in = 0;
    
    while ((sim908_buffer[out] != '\0') && (sim908_buffer[out] == ' ')){
	    out ++;	
	}
    
    while (sim908_buffer[out] != '\0'){	
	    if((sim908_buffer[out] == ' ') || (in > 9)){
		    dev_data.cSMSMessage[in] = '\0';
		    break;
		} else {
		    dev_data.cSMSMessage[in] = UpperCase(sim908_buffer[out]);
		    out ++;
		    in ++;
		    continue;
	 	}   
	}
	
    while ((sim908_buffer[out] != '\0') && (sim908_buffer[out] == ' ')){
	    out ++;	
	}
	
	in = 0;
	while (sim908_buffer[out] != '\0'){
		dev_data.cSMSParam[in] = UpperCase(sim908_buffer[out]);
		out ++;
		in ++;
		if(in > 9){
			dev_data.cSMSParam[in] = '\0';
			break;
		}
 	}
 	 
    //while ((*sim908_buffer != '\0') && (*sim908_buffer != ' ')) putbyte2(*s++);
    //strncpy(dev_data.cSMSMessage, sim908_buffer, 11);

    Nop();
    Nop();

    return 1;
}


/******************************************************************************************/
void replace(char* str, char bad, char good) {
    do {
        if(*str == bad) {
            *str = good;
        }
    } while(*++str);
}


