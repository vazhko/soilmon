
#include "MainPult.h"


int __attribute__((address(0x47fe))) flag_boot;



OLD_COMMANDS        commandBuffer;
COMMAND             commandInfo;
LOGGER_STATUS       loggerStatus; // todo: нехорошо
VOLUME_INFO         volume;

//******************************************************************************
DWORD GetUserTime(void);
DWORD GetUserDate(void);

char unparam_str [] = "\x1b[31m\x1b[1m - Error: Unsupported parameter\x1b[0m\x1b[1m\r\n";
char noparam_str [] = " - Error: parameter(s) required\r\n";
char smd_err_str [] = " - Error: communication lost\r\n";
char cmd_err_str [] = " - Error performing command\r\n";
char no_media_str [] = " - No media present\r\n";
char inval_sw_str [] = " - Invalid switch - ";

//******************************************************************************
//******************************************************************************
//******************************************************************************
// console
//******************************************************************************
//******************************************************************************
//******************************************************************************

void console(void (*PrintString)(char *), void (*PutChar)(char), char(*GetChar)(void), char(*InBuff)(void)) {

    FSFILE              *filePointer1;
    FSFILE              *filePointer2;
    char                oneChar;
    char                param1[MAX_COMMAND_LENGTH];
    char                param2[MAX_COMMAND_LENGTH];
    char                param3[MAX_COMMAND_LENGTH];
    char                param4[MAX_COMMAND_LENGTH];
    unsigned char       setAttribute, checkAttribute;
    SearchRec           searchRecord;

    RTCC dt;
    char str[20];

    BYTE smd_num;
    BYTE smd_mode;
    WORD smd_steps;
    DWORD_VAL smd_period;
    BYTE smd_c1, smd_c2, smd_interr, aduc_param, smd_status;
    WORD value, value_read;
    WORD_VAL wvalue, wvalue_read;
    WORD aduc_res;

    WORD_VAL wvPROD;



    if(GetCommand()) {

        set_fs_time(); // se_fs_time

        SkipWhiteSpace();
        switch(commandInfo.command) {
            case COMMAND_NO_COMMAND:
                break;

            case COMMAND_ADUC:

                GetOneWord(param1);
                GetOneWord(param2);
                if(param1[0] == 0) {
                    PrintString(noparam_str);
                } else {
                    // Установка тока
                    if(!strcmp("C", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);

                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(0, aduc_param)) {
                                sprintf(param4, "ADUC: current is %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }
                        }



                        // Установка направления тока
                    } else if(!strcmp("D", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);
                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(1, aduc_param)) {
                                sprintf(param4, "ADUC: current direction is %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }
                        }



                        // закоротка входа измерения удельного сопротивления на землю
                    } else if(!strcmp("S1", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);

                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(2, aduc_param)) {
                                sprintf(param4, "ADUC: RES is unshorted %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }
                        }



                        // закоротка входа ВАХ и термо на землю
                    } else if(!strcmp("S2", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);

                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(5, aduc_param)) {
                                sprintf(param4, "ADUC: VAH is unshorted %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }

                        }


                        // режима измерения ВАХ
                    } else if(!strcmp("V", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);
                            break;
                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(3, aduc_param)) {
                                sprintf(param4, "ADUC: VAH is %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }
                        }



                        // изменение коэффициента усиления в канале ВАХ и термо
                    } else if(!strcmp("G", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);

                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_set(4, aduc_param)) {
                                sprintf(param4, "ADUC: VAH gain is %u\r\n", aduc_param);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }
                        }


                        // измеhение
                    } else if(!strcmp("M", param1)) {

                        if(param2[0] == 0) {
                            PrintString(noparam_str);

                        } else {
                            aduc_param = atoi(param2);
                            if(aduc_get(aduc_param, &aduc_res)) {
                                if(aduc_param == 0)
                                    sprintf(param4, "ADUC: RES result is %d\r\n", aduc_res);
                                else if(aduc_param == 1)
                                    sprintf(param4, "ADUC: VAH result is %d\r\n", aduc_res);
                                PrintString(param4);

                            } else {
                                PrintString(smd_err_str);
                            }
                        }


                    } else if(!strcmp("?", param1)) {
                        PrintString("Curr[0..6]; Dir,S1,S2,Vah,Gain(0,1); Mes(0,1)\r\n");
                    } else {
                        PrintString(unparam_str);
                    }
                }
                break;



            case COMMAND_ATTRIB:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                checkAttribute = ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_ARCHIVE;
                setAttribute = 0x00;

                GetOneWord(param1);

                while((param1[0] == '+' || param1[0] == '-') && setAttribute != 0xFF) {
                    switch(param1[1]) {
                        case 'R':
                            if(((checkAttribute & ATTR_READ_ONLY) == 0) || (param1[2] != 0)) {
                                PrintString(inval_sw_str);
                                PutChar(param1[0]);
                                PrintString("R\r\n");
                                setAttribute = 0xFF;
                            } else {
                                checkAttribute &= ~ATTR_READ_ONLY;
                                if(param1[0] == '+')
                                    setAttribute |= ATTR_READ_ONLY;
                            }
                            break;
                        case 'A':
                            if(((checkAttribute & ATTR_ARCHIVE) == 0) || (param1[2] != 0)) {
                                PrintString(inval_sw_str);
                                PutChar(param1[0]);
                                PrintString("A\r\n");
                                setAttribute = 0xFF;
                            } else {
                                checkAttribute &= ~ATTR_ARCHIVE;
                                if(param1[0] == '+')
                                    setAttribute |= ATTR_ARCHIVE;
                            }
                            break;
                        case 'H':
                            if(((checkAttribute & ATTR_HIDDEN) == 0) || (param1[2] != 0)) {
                                PrintString(inval_sw_str);
                                PutChar(param1[0]);
                                PrintString("H\r\n");
                                setAttribute = 0xFF;
                            } else {
                                checkAttribute &= ~ATTR_HIDDEN;
                                if(param1[0] == '+')
                                    setAttribute |= ATTR_HIDDEN;
                            }
                            break;
                        case 'S':
                            if(((checkAttribute & ATTR_SYSTEM) == 0) || (param1[2] != 0)) {
                                PrintString(inval_sw_str);
                                PutChar(param1[0]);
                                PrintString("S\r\n");
                                setAttribute = 0xFF;
                            } else {
                                checkAttribute &= ~ATTR_SYSTEM;
                                if(param1[0] == '+')
                                    setAttribute |= ATTR_SYSTEM;
                            }
                            break;
                        default:
                            PrintString(inval_sw_str);
                            PutChar(param1[0]);
                            PutChar(param1[1]);
                            PrintString("\r\n");
                            setAttribute = 0xFF;
                            break;
                    }
                    GetOneWord(param1);
                }

                if(param1[0] == 0) {
                    PrintString("Please enter a target file or directory.\r\n");
                    break;
                }

                if(setAttribute != 0xFF) {
                    if(!FindFirst(param1, ATTR_MASK & ~ATTR_VOLUME, &searchRecord)) {
                        if((setAttribute == 0x00) && (checkAttribute == 0x27)) {
                            char buffer[50];
                            sprintf(buffer, "       %.42s", searchRecord.filename);
                            if(searchRecord.attributes &     ATTR_READ_ONLY) {
                                buffer[0] = 'R';
                            }
                            if(searchRecord.attributes &     ATTR_HIDDEN) {
                                buffer[1] = 'H';
                            }
                            if(searchRecord.attributes &     ATTR_SYSTEM) {
                                buffer[2] = 'S';
                            }
                            if(searchRecord.attributes &     ATTR_ARCHIVE) {
                                buffer[3] = 'A';
                            }
                            PrintString(buffer);
                            PrintString("\r\n");
                        } else {
                            if((filePointer2 = FSfopen(param1, "r")) == NULL) {
                                PrintString(" - Error opening file\r\n");
                                break;
                            }

                            if(checkAttribute & ATTR_READ_ONLY)
                                setAttribute |= (searchRecord.attributes & ATTR_READ_ONLY);
                            if(checkAttribute & ATTR_ARCHIVE)
                                setAttribute |= (searchRecord.attributes & ATTR_ARCHIVE);
                            if(checkAttribute & ATTR_SYSTEM)
                                setAttribute |= (searchRecord.attributes & ATTR_SYSTEM);
                            if(checkAttribute & ATTR_HIDDEN)
                                setAttribute |= (searchRecord.attributes & ATTR_HIDDEN);
                            if(FSattrib(filePointer2, setAttribute)) {
                                PrintString("Could not set attributes\r\n");
                            } else {
                                set_fs_time(); // se_fs_time
                                if(FSfclose(filePointer2)) {
                                    PrintString("Could not set attributes\r\n");
                                }
                            }
                        }
                    }
                }

                break;

            case COMMAND_BOOT:
                PrintString("Booting, close console");
                //U1CON = 0;
                //HDByteWriteI2C(0xa0, 0xff, 0xff, 0x5a);
                DelayMs(500);
                //flag_boot = 0x5a;
                __asm__("goto 0x106c");
                ///Reset();


                break;

            case COMMAND_CD:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString(noparam_str);
                } else {
                    if(FSchdir(&(commandInfo.buffer[commandInfo.index]))) {
                        PrintString(cmd_err_str);
                    }
                }
                break;

            case COMMAND_COPY:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                GetOneWord(param1);
                GetOneWord(param2);
                if((param1[0] == 0) || (param2[0] == 0)) {
                    PrintString(" - Two parameters required\r\n");
                } else {
                    if(!strcmp("CON", param1)) {
                        // Create the file from the console.  This is just a quick example.  A better one
                        // would be to take in an entire line, and then write the line.  That way, line
                        // editing would be supported.
                        if((filePointer2 = FSfopen(param2, "w")) == NULL) {
                            PrintString(" - Error creating file\r\n");
                        } else {
                            PrintString("Creating file from console. Enter Ctrl-Z to terminate...\r\n");
                            oneChar = 0;
                            do {
                                /// MonitorMedia();
                                ///if (U2STAbits.URXDA)  //UART2IsPressed())
                                if((InBuff()) > 0) {
                                    oneChar = GetChar();
                                    if(oneChar != 0x1A) {  // Control-Z
                                        PutChar(oneChar);
                                        if(FSfwrite(&oneChar, 1, 1, filePointer2) != 1) {
                                            PrintString(" - Error writing file\r\n");
                                            break;
                                        }
                                        if(oneChar == 0x0D) {
                                            oneChar = 0x0A;
                                            PutChar(oneChar);
                                            FSfwrite(&oneChar, 1, 1, filePointer2);
                                        }
                                    } else {
                                        PrintString("\r\n");
                                    }
                                }
                            } while(oneChar != 0x1A);
                            set_fs_time(); // se_fs_time
                            FSfclose(filePointer2);
                        }
                    } else {
                        if(FindFirst(param1,
                                     ATTR_DIRECTORY | ATTR_ARCHIVE | ATTR_READ_ONLY | ATTR_HIDDEN, &searchRecord)) {
                            PrintString(" - File not found\r\n");
                        } else if((filePointer1 = FSfopen(param1, "r")) == NULL) {
                            PrintString(" - Error opening file\r\n");
                        } else {
                            if((filePointer2 = FSfopen(param2, "w")) == NULL) {
                                PrintString(" - Error creating file\r\n");
                            } else {
                                size_t  charsRead;
                                BYTE    readBuffer[COPY_BUFFER_SIZE];

                                while(!FSfeof(filePointer1)) {
                                    charsRead = FSfread(readBuffer, 1, COPY_BUFFER_SIZE, filePointer1);
                                    if(charsRead) {
                                        if(FSfwrite(readBuffer, 1, charsRead, filePointer2) != charsRead) {
                                            PrintString(" - Error writing file\r\n");
                                            break;
                                        }
                                    }
                                }
                                FSfclose(filePointer2);
                            }
                            FSfclose(filePointer1);
                        }
                    }
                }
                break;


            case COMMAND_DATE:
                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString("Current date: ");
                    RTCCGetTime(&dt);
                    sprintf(str, "%04u-%02u-%02u\r\n", dt.yr + 2000, dt.mth, dt.day);
                    PrintString(str);

                } else {
                    // Set the current date.
                    DWORD_VAL   date;
                    rtccDate __date;

                    date.Val = GetUserDate();

                    if(date.Val) {

                        __date.year = date.v[2];
                        __date.mon = date.v[1];
                        __date.mday = date.v[0];
                        RTCCSetDate(&__date);

                    } else {
                        PrintString(" - Invalid date specified\r\n");
                    }
                }
                break;

            case COMMAND_DEL:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString(noparam_str);
                } else {
                    if(FindFirst(&(commandInfo.buffer[commandInfo.index]),
                                 ATTR_DIRECTORY | ATTR_ARCHIVE | ATTR_READ_ONLY | ATTR_HIDDEN, &searchRecord)) {
                        PrintString(" - File not found\r\n");
                    } else {
                        if(FSremove(&(commandInfo.buffer[commandInfo.index]))) {
                            PrintString(cmd_err_str);
                        }
                    }
                }
                break;

            case COMMAND_DIR:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    strcpy(param1, "*.*");
                } else {
                    strcpy(param1, &(commandInfo.buffer[commandInfo.index]));
                }
                if(!FindFirst(param1, ATTR_DIRECTORY | ATTR_ARCHIVE | ATTR_READ_ONLY | ATTR_HIDDEN, &searchRecord)) {
                    PrintFileInformation(searchRecord, PrintString);
                    while(!FindNext(&searchRecord)) {
                        PrintFileInformation(searchRecord , PrintString);
                    }
                } else {
                    PrintString("No files found.\r\n");
                }
                break;

            case COMMAND_DIREX:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    strcpy(param1, "*.*");
                } else {
                    strcpy(param1, &(commandInfo.buffer[commandInfo.index]));
                }
                if(!FindFirst(param1, ATTR_DIRECTORY | ATTR_ARCHIVE | ATTR_READ_ONLY | ATTR_HIDDEN, &searchRecord)) {
                    PrintFileFirstStr(searchRecord, PrintString,  PutChar);


                    while(!FindNext(&searchRecord)) {
                        PrintFileFirstStr(searchRecord , PrintString,  PutChar);

                    }
                } else {
                    PrintString("No files found.\r\n");
                }
                break;


            case COMMAND_FORMAT:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }
                char volID[] = "Terex pius";
                if(/*(FSCreateMBR(2,2000) == 0)&&*/(FSformat(0, 0x12345678, volID) == 0))
                    PrintString("\r\n");
                else
                    PrintString("\r\n - Error format\r\n");

                break;


            case COMMAND_HELP:
                PrintString("\r\nPIUS SD Card Explorer ");
                PrintString(VERSION);
                PrintString("\r\n\r\nAvailable commands:\r\n");
                PrintString("  ATTRIB <+|-><R|S|H|A> <name> - change attributes\r\n");
                PrintString("  CD <name>             - change directory\r\n");
                PrintString("  COPY <file1> <file2>  - copy [file1] to [file2]\r\n");
                PrintString("  COPY CON <file>       - create [file] from input\r\n");
                PrintString("  DATE [yyyy-mm-dd]     - display or set the date\r\n");
                PrintString("  DEL <file>            - delete file\r\n");
                PrintString("  DIR [file]            - display directory\r\n");
                PrintString("  DIREX [file]          - 1-st stroke in directory files\r\n");
                PrintString("  HELP or ?             - display help\r\n");
                //PrintString( "    LOG <POT|TMP> <file>  - log input to file\r\n" );
                PrintString("  MD <name>             - make directory\r\n");
                PrintString("  RD <name>             - remove directory\r\n");
                PrintString("  REN <file1> <file2>   - rename [file1] to [file2]\r\n");
                PrintString("  TIME [hh:mm:ss]       - display or set the time\r\n");
                PrintString("  TYPE <file>           - print out contents of file\r\n");

                break;

            case COMMAND_MD:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString(noparam_str);
                } else {
                    if(FSmkdir(&(commandInfo.buffer[commandInfo.index]))) {
                        PrintString(cmd_err_str);
                    }
                }
                break;

            case COMMAND_RD:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString(noparam_str);
                } else {
                    if(FSrmdir(&(commandInfo.buffer[commandInfo.index]), TRUE)) {
                        PrintString(cmd_err_str);
                    }
                }
                break;

            case COMMAND_REN:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                GetOneWord(param1);
                GetOneWord(param2);
                if((param1[0] == 0) || (param2[0] == 0)) {
                    PrintString(" - Two parameters required\r\n");
                } else {
                    if((filePointer1 = FSfopen(param1, "r")) == NULL) {
                        PrintString(" - Cannot find file\r\n");
                    } else {
                        if(FSrename(param2, filePointer1)) {
                            PrintString(cmd_err_str);
                        }
                        FSfclose(filePointer1);
                    }
                }
                break;

            case COMMAND_TIME:
                if(commandInfo.buffer[commandInfo.index] == 0) {
                    // Display the current time.
                    PrintString("Current time: ");
                    RTCCGetTime(&dt);
                    sprintf(str, "%02u:%02u:%02u\r\n", dt.hr, dt.min, dt.sec);
                    PrintString(str);


                } else {
                    // Set the current date.
                    DWORD_VAL   time;
                    rtccTime __time;

                    time.Val = GetUserTime();

                    if(time.Val) {

                        __time.hour = time.v[2];
                        __time.min = time.v[1];
                        __time.sec = time.v[0];
                        RTCCSetTime(&__time);

                    } else {
                        PrintString(" - Invalid time specified\r\n");
                    }
                }
                break;

            case COMMAND_TYPE:
                if(!loggerStatus.mediaPresent) {
                    PrintString(no_media_str);
                    break;
                }

                if(commandInfo.buffer[commandInfo.index] == 0) {
                    PrintString(noparam_str);
                } else {
                    // распечатка из памяти последнего эксперимента
                    if(!strcmp("MEM", &(commandInfo.buffer[commandInfo.index]))) {
                        TypeShotFile(&g_curr_shot, PrintString, PutChar);
                    } else if(FindFirst(&(commandInfo.buffer[commandInfo.index]),
                                        ATTR_DIRECTORY | ATTR_ARCHIVE | ATTR_READ_ONLY | ATTR_HIDDEN, &searchRecord)) {
                        PrintString(" - File not found\r\n");
                    } else if((filePointer1 = FSfopen(&(commandInfo.buffer[commandInfo.index]), "r")) == NULL) {
                        PrintString(" - Error opening file\r\n");
                    } else {
                        while(!FSfeof(filePointer1)) {
                            // This is just a quick example.  It's not very efficient to
                            // read only one byte at a time!
                            if(FSfread(&oneChar, 1, 1, filePointer1) == 1) {
                                PutChar(oneChar);
                            } else {
                                PrintString("\r\n - Error reading file\r\n");
                                break;
                            }
                        }
                        PrintString("\r\n");
                        FSfclose(filePointer1);
                    }
                }
                break;


            case COMMAND_SET:

                GetOneWord(param1);
                GetOneWord(param2);
                if(((param1[0] == 0) || (param2[0] == 0)) && (param1[0] != '?')) {
                    PrintString("Error: Two parameters required\r\n");
                } else {
                    if(!strcmp("N", param1)) {

                        GetOneWord(param3);
                        GetOneWord(param4);

                        if((param3[0] == 0) || (param4[0] == 0)) {
                            PrintString("Error: Four parameters required\r\n");
                        } else {
                            sprintf(&gShift.name1[0], "%s", param2);
                            sprintf(&gShift.name2[0], "%s", param3);
                            sprintf(&gShift.name3[0], "%s", param4);
                            PrintString("\r\n");
                        }

                    } else if(!strcmp("S", param1)) {
                        sprintf(gShift.shift, "%s", param2);
                    } else if(!strcmp("STEP", param1)) {
                        g_steps = atoi(param2);
                        DataEEWrite(g_steps, ADDRESS_W_STEP);
                    } else if(!strcmp("MSTEP", param1)) {
                        g_microsteps = atoi(param2);
                        DataEEWrite(g_microsteps, ADDRESS_W_MSTEP);
                    } else if(!strcmp("G", param1)) {
                        g_vah_gap = atof(param2);
                        DataEEWrite((WORD)(g_vah_gap * 10.0), ADDRESS_W_ZAZOR);
                    } else if(!strcmp("O", param1)) {
                        sprintf(&gSample.sample[0], "%s", param2);
                    } else if(!strcmp("L", param1)) {
                        gSample.length = atoi(param2);
                        DataEEWrite(gSample.length, ADDRESS_W_LENGHT);
                    } else if(!strcmp("W", param1)) {
                        gSample.weigh = atoi(param2);
                        DataEEWrite(gSample.weigh, ADDRESS_W_WEITH);
                        // проверка записи в EEPROM
                    } else if(!strcmp("E", param1)) {
                        value_read = atoi(param2);
                        DataEEWrite(value_read, ADDRESS_W_TEST);
                        value_read = 0;
                        value_read = DataEERead(ADDRESS_W_TEST);
                        sprintf(param4, "E %u\r\n", value_read);
                        PrintString(param4);
                    } else if(!strcmp("?", param1)) {
                        PrintString("  SET [param]  - set variable, [param] is:\r\n");
                        PrintString("   N NAME1 NAME2 NAME3 - operator name\r\n");
                        PrintString("   S SHIFT - set operator shift\r\n");
                        PrintString("   O ID - set sample ID\r\n");
                        PrintString("   L LENGHT - set sample length (0..65535)\r\n");
                        PrintString("   W WEITH - set sample weigh (0..65535)\r\n");
                        PrintString("   G - set VAH gap (""0.8"")\r\n");
                        PrintString("   STEP, MSTEP - set step and nicrostep count\r\n");

                    } else {
                        PrintString(unparam_str);
                    }
                }
                break;

            case COMMAND_GET:

                GetOneWord(param1);
                if(param1[0] == 0) {
                    PrintString(unparam_str);
                } else {
                    if(!strcmp("N", param1)) {
                        PrintString(gShift.name1);
                        PrintString(" ");
                        PrintString(gShift.name2);
                        PrintString(" ");
                        PrintString(gShift.name3);
                        PrintString("\r\n");

                    } else if(!strcmp("S", param1)) {
                        PrintString(gShift.shift);
                        PrintString("\r\n");
                    } else if(!strcmp("D", param1)) {
                        PrintString("Date\r\n");
                    } else if(!strcmp("O", param1)) {
                        PrintString(gSample.sample);
                        PrintString("\r\n");
                    } else if(!strcmp("L", param1)) {
                        //gSample.length = 12345;
                        sprintf(&param2[0], "%u", gSample.length);
                        PrintString(&param2[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("W", param1)) {
                        sprintf(&param2[0], "%u", gSample.weigh);
                        PrintString(&param2[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("STEP", param1)) {
                        sprintf(&param2[0], "%u", g_steps);
                        PrintString(&param2[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("MSTEP", param1)) {
                        sprintf(&param2[0], "%u", g_microsteps);
                        PrintString(&param2[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("G", param1)) {
                        sprintf(&param2[0], "%f", g_vah_gap);
                        PrintString(&param2[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("?", param1)) {
                        PrintString("  GET [N|S|O|L|W|G|STEPS|MSTEPS]  - get variable, see SET ?\r\n");
                    } else {
                        PrintString(unparam_str);
                    }
                }
                break;

            case COMMAND_STEP1:
                smd_num = 1;
            case COMMAND_STEP2:
                if(commandInfo.command == COMMAND_STEP2) smd_num = 2;

                GetOneWord(param1);
                GetOneWord(param2);
                if(param1[0] == 0) {
                    PrintString(noparam_str);
                } else {
                    if(!strcmp("L", param1)) {

                        if(param2[0] == 0) {
                            smd_mode = 0x80;
                            smd_steps = 0;
                        } else {
                            smd_mode = 0x81;
                            smd_steps = atoi(param2);
                        }

                        if(smd004_set_mode(smd_num, smd_mode, smd_steps) && (smd004_start(smd_num))) {
                            sprintf(param4, "SMD004: SM%u steps left %u\r\n", smd_num, smd_steps);
                            PrintString(param4);

                        } else {
                            PrintString(smd_err_str);
                        }
                    } else if(!strcmp("R", param1)) {

                        if(param2[0] == 0) {
                            smd_mode = 0x00;
                            smd_steps = 0;
                        } else {
                            smd_mode = 0x01;
                            smd_steps = atoi(param2);
                        }
                        if(smd004_set_mode(smd_num, smd_mode, smd_steps) && (smd004_start(smd_num))) {
                            sprintf(param4, "SMD004: SM%u steps right %u\r\n", smd_num, smd_steps);
                            PrintString(param4);
                        } else {
                            PrintString(smd_err_str);
                        }
                    } else if(!strcmp("S", param1)) {
                        if(smd004_stop(smd_num)) {
                            sprintf(param4, "SMD004: SM%u stop\r\n", smd_num);
                            PrintString(param4);
                        } else {
                            PrintString(smd_err_str);
                        }
                    } else if(!strcmp("C", param1)) {
                        if(param2[0] == 0) {
                            PrintString(noparam_str);
                        } else {
                            // определение введенного тоrа
                            smd_c1 = param2[0] - 0x30;
                            smd_c2 = param2[1] - 0x30;
                            if(smd_c1 > 7)smd_c1 = 7;
                            if(smd_c2 > 7)smd_c2 = 7;
                            // сохранение в еепром
                            wvalue.v[0] = smd_c1;
                            wvalue.v[1] = smd_c2;
                            wvalue_read.Val = 0;
                            if(smd_num == 1) {
                                DataEEWrite(wvalue.Val, ADDRESS_W_STEP1_CURR);
                                wvalue_read.Val = DataEERead(ADDRESS_W_STEP1_CURR);

                            }	else if(smd_num == 2) {
                                DataEEWrite(wvalue.Val, ADDRESS_W_STEP2_CURR);
                                wvalue_read.Val = DataEERead(ADDRESS_W_STEP2_CURR);

                            }

                            smd_c1 = wvalue_read.v[0];
                            smd_c2 = wvalue_read.v[1];

                            // запись в смд004
                            if(smd004_set_current(smd_num, smd_c1, smd_c2)) {
                                sprintf(param4, "SMD004: SM%u current move-%u keep-%u W%X R%X\r\n", smd_num, smd_c1, smd_c2, wvalue.Val, wvalue_read.Val);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }

                        }
                    } else if(!strcmp("P", param1)) {
                        if(param2[0] == 0) {
                            PrintString(noparam_str);
                        } else {
                            // определение введенного периода
                            smd_period.Val = atol(param2);
                            // сохранение в еепром
                            if(smd_num == 1) {
                                DataEEWrite(smd_period.w[0], ADDRESS_DW_STEP1_PER + 0);
                                DataEEWrite(smd_period.w[1], ADDRESS_DW_STEP1_PER + 2);
                            } else if(smd_num == 2) {
                                DataEEWrite(smd_period.w[0], ADDRESS_DW_STEP2_PER + 0);
                                DataEEWrite(smd_period.w[1], ADDRESS_DW_STEP2_PER + 2);
                            }
                            // запись в смд004
                            if(smd004_set_speed(smd_num, smd_period.Val)) {
                                sprintf(param4, "SMD004: %u period-%lu\r\n", smd_num, smd_period.Val);
                                PrintString(param4);
                            } else {
                                PrintString(smd_err_str);
                            }

                        }

                    } else if(!strcmp("P1", param1)) {    // hf,jxfz crjhjcnm
                        if(param2[0] == 0) {
                            PrintString(noparam_str);
                        } else {
                            // определение введенного периода
                            smd_period.Val = atol(param2);
                            // сохранение в еепром
                            if(smd_num == 2) {
                                DataEEWrite(smd_period.w[0], ADDRESS_DW_STEP22_PER + 0);
                                DataEEWrite(smd_period.w[1], ADDRESS_DW_STEP22_PER + 2);
                                sprintf(param4, "SMD004: %u marsh period-%lu\r\n", smd_num, smd_period.Val);
                                PrintString(param4);
                            }

                        }

                    } else if(!strcmp("I", param1)) {
                        if(smd004_get_status(&smd_status, &smd_interr)) {
                            sprintf(param4, "SMD004: Interrupters: %X\r\n", smd_interr);
                            PrintString(param4);
                        } else {
                            PrintString(smd_err_str);
                        }

                    } else if(!strcmp("?", param1)) {
                        PrintString("STEP(1,2): Left, Rigth, Curr, Speed, Period, Int\r\n");
                    } else {
                        PrintString(unparam_str);
                    }
                }
                break;

            case COMMAND_U1:

                GetOneWord(param1);
                GetOneWord(param2);
                if((param1[0] == 0) || (param2[0] == 0)) {
                    PrintString("Error: Two parameters required\r\n");
                } else {
                    if(!strcmp("H", param1)) {

                    } else {
                        PrintString(unparam_str);
                    }
                }

                break;

            case COMMAND_WORK:
                GetOneWord(param1);
                if(param1[0] == 0) {
                    PrintString(unparam_str);
                } else {
                    if(!strcmp("BACK", param1)) {
                        pultState = CMD_MOVE_TO_START_POINT;
                        PrintString(&param1[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("START", param1)) {
                        pultState = CMD_WF_START;
                        shots_point = 0;
                        g_curr_shot.last.steps = 0;
                        NewShot(&g_curr_shot);
                        NewWfLb(&g_curr_shot);
                        PrintString(&param1[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("STOP", param1)) {
                        pultState = CMD_WF_STOP;
                        PrintString(&param1[0]);
                        PrintString("\r\n");
                    } else if(!strcmp("ST", param1)) {
                        if(pultState == PULT_IDLE) {
                            PrintString("DONE");
                        } else {
                            PrintString("MOVE");
                        }
                        PrintString("\r\n");
                    } else if(!strcmp("POINT", param1)) {


                        g_curr_shot.last.ut = 20.0;
                        g_curr_shot.last.rs += 1.2;
                        g_curr_shot.last.un += 3.3;
                        g_curr_shot.last.up -= 1.2;
                        g_curr_shot.last.curr += 1;
                        g_curr_shot.last.attempt = 2;
                        g_curr_shot.last.steps ++;
                        AddShotPoint(&g_curr_shot);
                        AddWfPointLb(&g_curr_shot);


                        PrintString("\r\n");
                    } else if(!strcmp("DUMMY", param1)) {
                        PrintString("Created dummy file... ");
                        if(CreateDummyFile() == 0) {
                            PrintString("not");
                        }
                        PrintString(" created.");
                        PrintString("\r\n");
                    } else if(!strcmp("P", param1)) {
                        shots_point = 0;
                        PrintString("\r\n");
                    } else if(!strcmp("?", param1)) {
                        PrintString("  WORK [BACK|START|STOP|ST|DUMMY]\r\n");
                    } else {
                        PrintString(unparam_str);
                    }
                }
                break;

            default:
                PrintString("Unsupported command\r\n");
                break;
        }
        InitializeCommand(PrintString, PutChar);
    }
    // }
}

//******************************************************************************
void console_init(void) {
    commandBuffer.newest    = MAX_BUFFERED_COMMANDS;
    commandBuffer.oldest    = MAX_BUFFERED_COMMANDS;
    commandBuffer.showing   = MAX_BUFFERED_COMMANDS;
    loggerStatus.value      = 0;
    volume.valid            = FALSE;
}

//******************************************************************************
//******************************************************************************
// Internal Functions
//******************************************************************************
//******************************************************************************




/****************************************************************************
  Function:
	BYTE GetCommand( void )

  Description:
	This function returns whether or not the user has finished entering a
	command.  If so, then the command entered by the user is determined and
	placed in commandInfo.command.  The command line index
	(commandInfo.index) is set to the first non-space character after the
	command.

  Precondition:
	commandInfo.reading must be valid.

  Parameters:
	None

  Return Values:
	TRUE    - The user has entered a command.  The command is in
				  commandInfo.command.
	FALSE   - The user has not finished entering a command.

  Remarks:
	None
  ***************************************************************************/

BYTE GetCommand(void) {
    char    firstWord[MAX_COMMAND_LENGTH];

    if(commandInfo.reading) {
        return FALSE;
    } else {
        commandInfo.index = 0;

        commandInfo.index = 0;
        GetOneWord(firstWord);
        SkipWhiteSpace();

        if(firstWord[0] == 0) {
            commandInfo.command = COMMAND_NO_COMMAND;
            return TRUE;
        }
        if(!strncmp(firstWord, "ADUC", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_ADUC;
            return TRUE;
        }
        if(!strncmp(firstWord, "ATTRIB", 6) && (strlen(firstWord) == 6)) {
            commandInfo.command = COMMAND_ATTRIB;
            return TRUE;
        }
        if(!strncmp(firstWord, "CD", 2) && (strlen(firstWord) == 2)) {
            commandInfo.command = COMMAND_CD;
            return TRUE;
        }
        if(!strncmp(firstWord, "COPY", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_COPY;
            return TRUE;
        }
        if(!strncmp(firstWord, "DATE", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_DATE;
            return TRUE;
        }
        if(!strncmp(firstWord, "DEL", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_DEL;
            return TRUE;
        }
        if(!strncmp(firstWord, "DIR", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_DIR;
            return TRUE;
        }
        if(!strncmp(firstWord, "DIREX", 5) && (strlen(firstWord) == 5)) {
            commandInfo.command = COMMAND_DIREX;
            return TRUE;
        }
        if((!strncmp(firstWord, "HELP", 4)  && (strlen(firstWord) == 4)) ||
                (!strncmp(firstWord, "?", 1) && (strlen(firstWord) == 1))) {
            commandInfo.command = COMMAND_HELP;
            return TRUE;
        }
        if(!strncmp(firstWord, "LOG", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_LOG;
            return TRUE;
        }
        if(!strncmp(firstWord, "MD", 2) && (strlen(firstWord) == 2)) {
            commandInfo.command = COMMAND_MD;
            return TRUE;
        }
        if(!strncmp(firstWord, "RD", 2) && (strlen(firstWord) == 2)) {
            commandInfo.command = COMMAND_RD;
            return TRUE;
        }
        if(!strncmp(firstWord, "REN", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_REN;
            return TRUE;
        }
        if(!strncmp(firstWord, "TIME", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_TIME;
            return TRUE;
        }
        if(!strncmp(firstWord, "TYPE", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_TYPE;
            return TRUE;
        }
        if(!strncmp(firstWord, "FORMAT", 6) && (strlen(firstWord) == 6)) {
            commandInfo.command = COMMAND_FORMAT;
            return TRUE;
        }
        if(!strncmp(firstWord, "SET", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_SET;
            return TRUE;
        }
        if(!strncmp(firstWord, "STEP1", 5) && (strlen(firstWord) == 5)) {
            commandInfo.command = COMMAND_STEP1;
            return TRUE;
        }
        if(!strncmp(firstWord, "STEP2", 5) && (strlen(firstWord) == 5)) {
            commandInfo.command = COMMAND_STEP2;
            return TRUE;
        }
        if(!strncmp(firstWord, "GET", 3) && (strlen(firstWord) == 3)) {
            commandInfo.command = COMMAND_GET;
            return TRUE;
        }
        if(!strncmp(firstWord, "U1", 2) && (strlen(firstWord) == 2)) {
            commandInfo.command = COMMAND_U1;
            return TRUE;
        }
        if(!strncmp(firstWord, "U2", 2) && (strlen(firstWord) == 2)) {
            commandInfo.command = COMMAND_U2;
            return TRUE;
        }
        if(!strncmp(firstWord, "WORK", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_WORK;
            return TRUE;
        }
        if(!strncmp(firstWord, "BOOT", 4) && (strlen(firstWord) == 4)) {
            commandInfo.command = COMMAND_BOOT;
            return TRUE;
        }
        commandInfo.command = COMMAND_UNKNOWN;
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

    while((commandInfo.buffer[commandInfo.index] != 0) &&
            (commandInfo.buffer[commandInfo.index] != ' ')) {
        *buffer++ = commandInfo.buffer[commandInfo.index++];
    }
    *buffer = 0;
}


/****************************************************************************
  Function:
	DWORD GetUserDate( void )

  Description:
	This function extracts a user entered date from the command line and
	places it in a DWORD that matches the format required for the RTCC.
	The required format is:
						YYYY-MM-DD
	where YY is between 2000 and 2099.  If the date is not in a valid format,
	0 is returned.

  Precondition:
	commandInfo.buffer and commandInfo.index are valid, and
	commandInfo.index points to the first character of the date

  Parameters:


  Returns:
	If the project is built for a PIC24F, this function returns a DWORD in
	the format <00><YY><MM><DD>.  If it is built for a PIC32MX, this function
	returns a DWORD in the format <YY><MM><DD><00>.

  Remarks:
	Range checks are not comprehensive.  The day is not qualified based on
	how many days are in the specified month.

	The values from the RTCC are assumed to be in BCD format.

	The two architectures have different formats for the date.  The index
	values are set above accordingly.
  ***************************************************************************/
///*
DWORD GetUserDate(void) {
    DWORD_VAL   date;

    date.Val = 0;

    // Get the year.
    if(commandInfo.buffer[commandInfo.index++] != '2')						return 0;
    if(commandInfo.buffer[commandInfo.index++] != '0')						return 0;

    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_YEAR] = (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_YEAR] |= (commandInfo.buffer[commandInfo.index++] - '0');

    if(commandInfo.buffer[commandInfo.index++] != '-')						return 0;

    // Get the month.
    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_MONTH] = (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_MONTH] |= (commandInfo.buffer[commandInfo.index++] - '0');
    if(!((0x01 <= date.v[INDEX_MONTH]) && (date.v[INDEX_MONTH] <= 0x12)))	return 0;

    if(commandInfo.buffer[commandInfo.index++] != '-')						return 0;

    // Get the day.
    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_DAY] = (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index]))					return 0;
    date.v[INDEX_DAY] |= (commandInfo.buffer[commandInfo.index++] - '0');
    if(!((0x01 <= date.v[INDEX_DAY]) && (date.v[INDEX_DAY] <= 0x31)))		return 0;

    return date.Val;
}
//*/


/****************************************************************************
  Function:
	DWORD GetUserTime( void )

  Description:
	This function extracts a user entered time from the command line and
	places it in a DWORD that matches the format required for the RTCC.  The
	required format is:
						HH:MM:SS
	in 24-hour format.  If the time is not in a valid format, 0 is returned.

  Precondition:
	commandInfo.buffer and commandInfo.index are valid;
	commandInfo.index points to the first character of the time

  Parameters:
	None

  Return Values:
	If the project is built for a PIC24F, this function returns a DWORD in
	the format <00><HH><MM><SS>.  If it is built for a PIC32MX, this function
	returns a DWORD in the format <HH><MM><SS><00>.

  Remarks:
	The values from the RTCC are assumed to be in BCD format.

	The two architectures have different formats for the date. The index
	values are set above accordingly.
  ***************************************************************************/

DWORD GetUserTime(void) {
    DWORD_VAL   time;

    time.Val = 0;

    // Get the hours.
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_HOURS] |= (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_HOURS] |= (commandInfo.buffer[commandInfo.index++] - '0');
    if(time.v[INDEX_HOURS] > 0x23)						 return 0;

    if(commandInfo.buffer[commandInfo.index++] != ':')	 return 0;

    // Get the minutes.
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_MINUTES] |= (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_MINUTES] |= (commandInfo.buffer[commandInfo.index++] - '0');
    if(time.v[INDEX_MINUTES] > 0x59)					 return 0;

    if(commandInfo.buffer[commandInfo.index++] != ':')	 return 0;

    // Get the seconds.
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_SECONDS] |= (commandInfo.buffer[commandInfo.index++] - '0') << 4;
    if(!IsNum(commandInfo.buffer[commandInfo.index])) return 0;
    time.v[INDEX_SECONDS] |= (commandInfo.buffer[commandInfo.index++] - '0');
    if(time.v[INDEX_SECONDS] > 0x59)					 return 0;

    return time.Val;
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

void InitializeCommand(void (*PrintString)(char *), void (*PutChar)(char)) {
    char            buffer[50];

    if(loggerStatus.mediaPresent) {
        buffer[0] = 0;
        if(volume.valid) {
            PrintString(volume.label);
            PutChar(':');
        }

        FSgetcwd(buffer, 50);
        PrintString(buffer);
    }

    PrintString("> ");

    commandInfo.command     = COMMAND_NO_COMMAND;
    commandInfo.index       = 0;
    commandInfo.reading     = TRUE;

    memset(commandInfo.buffer, 0x00, MAX_COMMAND_LENGTH);
}




/****************************************************************************
  Function:
	void PrintFileInformation( SearchRec searchRecord )

  Description:
	This function prints the file information that is contained in
	searchRecord.  Information printed is:
				<date> <time> [<DIR>] [<size>] <name>

  Precondition:
	None

  Parameters:
	SearchRec searchRecord  - File information

  Returns:
	None

  Remarks:
	The timestamp is assumed to be in the following format:
			Date format:    Bits 15-9:  Year (0 = 1980, 127 = 2107)
							Bits 8-5:   Month (1 = January, 12 = December)
							Bits 4-0:   Day (1 - 31)

			Time format:    Bits 15-11: Hours (0-23)
							Bits 10-5:  Minutes (0-59)
							Bits 4-0:   Seconds/2 (0-29)
  ***************************************************************************/

void PrintFileInformation(SearchRec searchRecord, void (*PrintString)(char *)) {
    char        buffer[20];

    // Display the file's date/time stamp.
    sprintf(buffer, "%04d-%02d-%02d ", ((((DWORD_VAL)(searchRecord.timestamp)).w[1] & 0xFE00) >> 9) + 1980,
            (((DWORD_VAL)(searchRecord.timestamp)).w[1] & 0x01E0) >> 5,
            ((DWORD_VAL)(searchRecord.timestamp)).w[1] & 0x001F);
    PrintString(buffer);
    sprintf(buffer, "%02d:%02d:%02d ", (((DWORD_VAL)(searchRecord.timestamp)).w[0] & 0xF800) >> 11,
            (((DWORD_VAL)(searchRecord.timestamp)).w[0] & 0x07E0) >> 5,
            (((DWORD_VAL)(searchRecord.timestamp)).w[0] & 0x001F) << 1);
    PrintString(buffer);

    // Display the file size.  If the file is actually a directory, display an indication.
    if(searchRecord.attributes & ATTR_DIRECTORY) {
        PrintString("<DIR>           ");
    } else {
        sprintf(buffer, "     %10ld ", ((DWORD_VAL)(searchRecord.filesize)).Val);
        PrintString(buffer);
    }


    // Display the file name.
    PrintString(searchRecord.filename);
    PrintString("\r\n");
}


/*********************************************************************************************/
// For dirEx
void PrintFileFirstStr(SearchRec searchRecord, void (*PrintString)(char *), void (*PutChar)(char)) {
    char        buffer[20], oneChar;
    FSFILE              *filePointer1;

    if(searchRecord.attributes & ATTR_DIRECTORY) {
        //PrintString( "<DIR>           " );
        return;
    }


    if((filePointer1 = FSfopen(searchRecord.filename, "r")) == NULL) {
        PrintString(" - Error opening file\r\n");
    } else {

        while(!FSfeof(filePointer1)) {

            // This is just a quick example.  It's not very efficient to
            // read only one byte at a time!
            if(FSfread(&oneChar, 1, 1, filePointer1) == 1) {
                if(oneChar == '\r') break;
                PutChar(oneChar);
            } else {
                PrintString("\r\n - Error reading file\r\n");
                break;
            }
        }

        //PrintString( "\r\n" );
        FSfclose(filePointer1);
    }




    PrintString("\r\n");
}

/****************************************************************************
  Function:
	void WriteOneBuffer( FSFILE *fptr, BYTE *data, WORD size )

  Description:
	This function writes one log buffer to the indicated file.  It then
	advances the pointer to the current buffer to write.

  Precondition:
	None

  Parameters:
	FSFILE *fptr    - Pointer to an open file
	BYTE *data      - Pointer to data to write
	WORD size       - How many bytes of data to write

  Return Values:
	None

  Remarks:
	None
  ***************************************************************************/
/*
void WriteOneBuffer( FSFILE *fptr, BYTE *data, WORD size )
{
	if (FSfwrite( data, 1, size, fptr) != size)
	{
		PrintString( "! Error writing log file\r\n" );
	}
	logData[logBufferWriting].bufferFull    = FALSE;
	logData[logBufferWriting].index         = 0;

	logBufferWriting++;
	if (logBufferWriting >= NUM_LOG_BUFFERS)
	{
		logBufferWriting = 0;
	}
}
*/


/****************************************************************************
  Function:
	void RedoCommandPrompt( void )

  Description:
	This function is called when the user either removes or inserts media.
	We want to let the user know right away that something has changed, so
	we change the command prompt immediately.  If the user is entering a
	command, we rebuild his command.

  Precondition:
	None

  Parameters:
	None

  Returns:
	None

  Remarks:
	None
  ***************************************************************************/

void RedoCommandPrompt(void (*PrintString)(char *), void (*PutChar)(char)) {
    int i;

    PrintString("\r\n");

    if(volume.valid) {
        PrintString(volume.label);
    }

    PrintString(":\\> ");

    if(commandInfo.reading) {
        for(i = 0; i < commandInfo.index; i++) {
            PutChar(commandInfo.buffer[i]);
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
        commandInfo.buffer[commandInfo.index++] = oneChar;
    }
}

/****************************************************************************
  Function:
	void EraseCommandLine( void )

  Description:
	This function erases the current command line.  It works by sending a
	backspace-space-backspace combination for each character on the line.

  Precondition:
	commandInfo.index must be valid.

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

    lineLength = commandInfo.index;
    for(i = 0; i < lineLength; i++) {
        PutChar(0x08);
        PutChar(' ');
        PutChar(0x08);
    }
    commandInfo.index = 0;
}


/**********************************************************************************************************************/
void MonitorUser(void (*PrintString)(char *), void (*PutChar)(char), char(*GetChar)(void), char(*InBuff)(void)) {
    char    oneChar;

    if(InBuff() > 0) {
        oneChar = GetChar();

        // If we are currently processing a command, throw the character away.
        if(commandInfo.reading) {
            if(commandInfo.escNeedSecondChar) {
                if(commandInfo.escFirstChar == 0x5B) {
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
                commandInfo.escNeedSecondChar   = FALSE;
            } else if(commandInfo.escNeedFirstChar) {
                commandInfo.escFirstChar        = oneChar;
                commandInfo.escNeedFirstChar    = FALSE;
                commandInfo.escNeedSecondChar   = TRUE;
            } else {
                if(oneChar == 0x1B) {	   // ESC - an escape sequence
                    commandInfo.escNeedFirstChar = TRUE;
                } else if(oneChar == 0x08) {	  // Backspace
                    if(commandInfo.index > 0) {
                        commandInfo.index --;
                        PutChar(0x08);
                        PutChar(' ');
                        PutChar(0x08);
                    }
                } else if((oneChar == 0x0D) || (oneChar == 0x0A)) {
                    PrintString("\r\n");
                    commandInfo.buffer[commandInfo.index]   = 0; // Null terminate the input command
                    commandInfo.reading                     = FALSE;
                    commandInfo.escNeedFirstChar            = FALSE;
                    commandInfo.escNeedSecondChar           = FALSE;

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
                    strcpy(&(commandBuffer.lines[commandBuffer.newest][0]), commandInfo.buffer);
                } else if((0x20 <= oneChar) && (oneChar <= 0x7E)) {
                    oneChar = UpperCase(oneChar);	// To make later processing simpler
                    if(commandInfo.index < MAX_COMMAND_LENGTH) {
                        commandInfo.buffer[commandInfo.index++] = oneChar;
                    }
                    PutChar(oneChar);	     // Echo the character
                } else if(('А' <= oneChar) && (oneChar <= 'я')) {  //кирилица

                    if(commandInfo.index < MAX_COMMAND_LENGTH) {
                        commandInfo.buffer[commandInfo.index++] = oneChar;
                    }
                    PutChar(oneChar);	     // Echo the character
                }
            }
        }
    }
}


// ******************************************************************************************************
char MonitorMedia(void (*PrintString)(char *), void (*PutChar)(char)) {
    BYTE            mediaPresentNow;
    BYTE            mountTries;
    SearchRec       searchRecord;


    mediaPresentNow = MDD_MediaDetect();
    if(mediaPresentNow != loggerStatus.mediaPresent) {
        if(mediaPresentNow) {
            mountTries = 10;
            while(!FSInit() && mountTries--);
            if(!mountTries) {
                PrintString("Could not mount media\r\n");
                loggerStatus.mediaPresent = FALSE;
            } else {
                loggerStatus.mediaPresent = TRUE;

                // Find the volume label.  We need to do this here while we are at the
                // root directory.
                if(!FindFirst("*.*", ATTR_VOLUME, &searchRecord)) {
                    strcpy(volume.label, searchRecord.filename);
                    volume.valid = TRUE;
                }

                RedoCommandPrompt(PrintString, PutChar);
            }
        } else {
            loggerStatus.mediaPresent   = FALSE;
            volume.valid                = FALSE;

            RedoCommandPrompt(PrintString, PutChar);
        }
    }

    return mediaPresentNow;
}


// ******************************************************************************************************
char IsMediaOk(void) {
    BYTE            mediaPresentNow;
    BYTE            mountTries;


    mediaPresentNow = MDD_MediaDetect();
    if(mediaPresentNow != loggerStatus.mediaPresent) {
        if(mediaPresentNow) {
            mountTries = 10;
            while(!FSInit() && mountTries--);
            if(!mountTries) {
                //PrintString( "Could not mount media\r\n" );
                //loggerStatus.mediaPresent = FALSE;
                return 0;
            } else {
                //Ok
            }
        }
    }

    return 1;
}





