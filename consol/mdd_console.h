

// *****************************************************************************
// *****************************************************************************
// Constants
// *****************************************************************************
// *****************************************************************************
#define MAX_BUFFERED_COMMANDS           8           // Must be a power of 2
#define MAX_COMMAND_LENGTH              70
//#define MAX_LOG_BUFFER_SIZE           512
//#define NUM_LOG_BUFFERS               2
#define COPY_BUFFER_SIZE                512         // Increase to improve copy efficiency
#define MAX_ALLOWED_CURRENT             500         // Maximum power we can supply in mA
#define VERSION                         "v0.9.0"

#define DEFAULT_YEARS               0x0007
#define DEFAULT_MONTH_DAY           0x0815
#define DEFAULT_WEEKDAY_HOURS       0x0304
#define DEFAULT_MINUTES_SECONDS     0x2100

#define INDEX_HOURS                 2
#define INDEX_MINUTES               1
#define INDEX_SECONDS               0
#define INDEX_YEAR                  2
#define INDEX_MONTH                 1
#define INDEX_DAY                   0

// *****************************************************************************
// *****************************************************************************
// Data Structures
// *****************************************************************************
// *****************************************************************************

typedef struct _COMMAND
{
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


typedef enum _COMMANDS
{
    COMMAND_NO_COMMAND,
    COMMAND_ADUC,
    COMMAND_ATTRIB,
    COMMAND_BOOT,
    COMMAND_CD,
    COMMAND_COPY,
    COMMAND_DATE,
    COMMAND_DEL,
    COMMAND_DIR,
    COMMAND_DIREX,
    COMMAND_FORMAT,
    COMMAND_HELP,
    COMMAND_GET,
    COMMAND_LOG,
    COMMAND_MD,
    COMMAND_RD,
    COMMAND_REN,
    COMMAND_SET, 
    COMMAND_STEP1,
    COMMAND_STEP2,
    COMMAND_TIME,
    COMMAND_TYPE,
    COMMAND_U1,
    COMMAND_U2,
    COMMAND_WORK,
    COMMAND_UNKNOWN
} COMMANDS;




typedef struct _OLD_COMMANDS
{
    char        lines[MAX_BUFFERED_COMMANDS][MAX_COMMAND_LENGTH];
    BYTE        oldest;
    BYTE        newest;
    BYTE        showing;
} OLD_COMMANDS;


typedef struct _LOGGER_STATUS{

    union
    {
        BYTE    value;
        struct
        {
            BYTE        mediaPresent            : 1;
            BYTE        readingPotentiometer    : 1;
            BYTE        readingTemperature      : 1;
        };
    };

} LOGGER_STATUS;





typedef struct _VOLUME_INFO
{
    char        label[12];
    BYTE        valid;
} VOLUME_INFO;


// *****************************************************************************
// *****************************************************************************
// Internal Function Prototypes
// *****************************************************************************
// *****************************************************************************

BYTE    GetCommand( void );

void    GetOneWord( char *buffer );



void    InitializeCommand( void (*PrintString)( char * ), void (*PutChar)( char ) );
void    PrintFileInformation( SearchRec searchRecord, void (*PrintString)( char * ));
void    PrintFileFirstStr( SearchRec searchRecord, void (*PrintString)( char * ), void (*PutChar)( char ));
void    RedoCommandPrompt( void (*PrintString)( char * ), void (*PutChar)( char ) );
void    ReplaceCommandLine( void (*PutChar)( char ) );
void	EraseCommandLine( void (*PutChar)( char ));
void    WriteOneBuffer( FSFILE *fptr, BYTE *data, WORD size );


void console_init(void);
void console (void (*PrintString)( char * ), void (*PutChar)( char ), char (*GetChar)(void), char (*InBuff)(void));
void MonitorUser(void (*PrintString)( char * ), void (*PutChar)( char ),char (*GetChar)(void), char (*InBuff)(void));
char MonitorMedia( void (*PrintString)( char * ), void (*PutChar)( char ) );

char IsMediaOk( void );




// *****************************************************************************
// *****************************************************************************
// Macros
// *****************************************************************************
// *****************************************************************************

#define IsNum(c)            ((('0' <= c) && (c <= '9')) ? TRUE : FALSE)
#define UpperCase(c)        (('a'<= c) && (c <= 'z') ? c - 0x20 : c)
#define SkipWhiteSpace()    { while (commandInfo.buffer[commandInfo.index] == ' ') commandInfo.index++; }



