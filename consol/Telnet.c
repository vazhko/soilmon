

#define __TELNET_C

#include "TCPIPConfig.h"

#if defined(STACK_USE_TELNET_SERVER)

#include "TCPIP Stack/TCPIP.h"

// Set up configuration parameter defaults if not overridden in 
// TCPIPConfig.h
#if !defined(TELNET_PORT)
    // Unsecured Telnet port
	#define TELNET_PORT			23
#endif
#if !defined(TELNETS_PORT)	
    // SSL Secured Telnet port (ignored if STACK_USE_SSL_SERVER is undefined)
	#define TELNETS_PORT		992	
#endif
#if !defined(MAX_TELNET_CONNECTIONS)
    // Maximum number of Telnet connections
	#define MAX_TELNET_CONNECTIONS	(3u)
#endif
#if !defined(TELNET_USERNAME)
    // Default Telnet user name
	#define TELNET_USERNAME		"admin"
#endif
#if !defined(TELNET_PASSWORD)
    // Default Telnet password
	#define TELNET_PASSWORD		"terex"
#endif

// Demo title string
static ROM BYTE strTitle[]			= "\x1b[2J\x1b[33m\x1b[1m"	// 2J is clear screen, 31m is y, 1m is bold
									  "Terex Telnet Server 1.1\x1b[0m\r\n"	// 0m is clear all attributes
									  "(for this demo, type 'admin' for the login and 'terex' for the password.)\r\n"
								  	  "Login: ";
// Demo password
static ROM BYTE strPassword[]		= "Password: \xff\xfd\x2d";	// DO Suppress Local Echo (stop telnet client from printing typed characters)
// Access denied message
static ROM BYTE strAccessDenied[]	= "\r\nAccess denied\r\n\r\n";
// Successful authentication message
/*
static ROM BYTE strAuthenticated[]	= "\r\nLogged in successfully\r\n\r\n"
									  "\r\nPress 'q' to quit\r\n";
*/									  
static ROM BYTE strAuthenticated[]	= "\r\nLogged in successfully\r\n\r\n";
// Demo output string

static ROM BYTE strDisplay[]		= "\r\nSNTP Time:    (disabled)"
									  "\r\nAnalog:             1023"
									  "\r\nButtons:         3 2 1 0"
									  "\r\nLEDs:    7 6 5 4 3 2 1 0";
									  
// String with extra spaces, for Demo
static ROM BYTE strSpaces[]			= "          ";
// Demo disconnection message
static ROM BYTE strGoodBye[]		= "\r\n\r\nGoodbye!\r\n";

extern BYTE AN0String[8];



// ******************************************************************************************************
void TCPPutChar(char data);
void TCPPrintString(char *data);
char TCPGetChar(void);
char TCPRxInBuf(void);
// ******************************************************************************************************




/*********************************************************************
 * Function:        void TelnetTask(void)
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Performs Telnet Server related tasks.  Contains
 *                  the Telnet state machine and state tracking
 *                  variables.
 *
 * Note:            None
 ********************************************************************/
 TCP_SOCKET	MySocket;
 
void TelnetTask(void)
{
	BYTE 		i;
	BYTE		vTelnetSession;
	WORD		w, w2;
	//TCP_SOCKET	MySocket;
	enum
	{
		SM_HOME = 0,
		SM_PRINT_LOGIN,
		SM_GET_LOGIN,
		SM_GET_PASSWORD,
		SM_GET_PASSWORD_BAD_LOGIN,
		SM_AUTHENTICATED,
		SM_REFRESH_VALUES
	} TelnetState;
	static TCP_SOCKET hTelnetSockets[MAX_TELNET_CONNECTIONS];
	static BYTE vTelnetStates[MAX_TELNET_CONNECTIONS];
	static BOOL bInitialized = FALSE;

	// Perform one time initialization on power up
	if(!bInitialized)
	{
		for(vTelnetSession = 0; vTelnetSession < MAX_TELNET_CONNECTIONS; vTelnetSession++)
		{
			hTelnetSockets[vTelnetSession] = INVALID_SOCKET;
			vTelnetStates[vTelnetSession] = SM_HOME;
		}
		bInitialized = TRUE;
	}


	// Loop through each telnet session and process state changes and TX/RX data
	for(vTelnetSession = 0; vTelnetSession < MAX_TELNET_CONNECTIONS; vTelnetSession++)
	{
		// Load up static state information for this session
		MySocket = hTelnetSockets[vTelnetSession];
		TelnetState = vTelnetStates[vTelnetSession];

		// Reset our state if the remote client disconnected from us
		if(MySocket != INVALID_SOCKET)
		{
			if(TCPWasReset(MySocket))
				TelnetState = SM_PRINT_LOGIN;
		}
	
		// Handle session state
		switch(TelnetState)
		{
			case SM_HOME:
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen(0, TCP_OPEN_SERVER, TELNET_PORT, TCP_PURPOSE_TELNET);
				
				// Abort operation if no TCP socket of type TCP_PURPOSE_TELNET is available
				// If this ever happens, you need to go add one to TCPIPConfig.h
				if(MySocket == INVALID_SOCKET)
					break;
	
				// Open an SSL listener if SSL server support is enabled
				#if defined(STACK_USE_SSL_SERVER)
					TCPAddSSLListener(MySocket, TELNETS_PORT);
				#endif
	
				TelnetState++;
				break;
	
			case SM_PRINT_LOGIN:
				#if defined(STACK_USE_SSL_SERVER)
					// Reject unsecured connections if TELNET_REJECT_UNSECURED is defined
					#if defined(TELNET_REJECT_UNSECURED)
						if(!TCPIsSSL(MySocket))
						{
							if(TCPIsConnected(MySocket))
							{
								TCPDisconnect(MySocket);
								TCPDisconnect(MySocket);
								break;
							}	
						}
					#endif
						
					// Don't attempt to transmit anything if we are still handshaking.
					if(TCPSSLIsHandshaking(MySocket))
						break;
				#endif
				
				// Make certain the socket can be written to
				if(TCPIsPutReady(MySocket) < strlenpgm((ROM char*)strTitle))
					break;
				
				// Place the application protocol data into the transmit buffer.
				TCPPutROMString(MySocket, strTitle);
	
				// Send the packet
				TCPFlush(MySocket);
				TelnetState++;
	
			case SM_GET_LOGIN:
				// Make sure we can put the password prompt
				if(TCPIsPutReady(MySocket) < strlenpgm((ROM char*)strPassword))
					break;
	
				// See if the user pressed return
				w = TCPFind(MySocket, '\n', 0, FALSE);
				if(w == 0xFFFFu)
				{
					if(TCPGetRxFIFOFree(MySocket) == 0u)
					{
						TCPPutROMString(MySocket, (ROM BYTE*)"\r\nToo much data.\r\n");
						TCPDisconnect(MySocket);
					}
	
					break;
				}
			
				// Search for the username -- case insensitive
				w2 = TCPFindROMArray(MySocket, (ROM BYTE*)TELNET_USERNAME, sizeof(TELNET_USERNAME)-1, 0, TRUE);
				if((w2 != 0u) || !((sizeof(TELNET_USERNAME)-1 == w) || (sizeof(TELNET_USERNAME) == w)))
				{
					// Did not find the username, but let's pretend we did so we don't leak the user name validity
					TelnetState = SM_GET_PASSWORD_BAD_LOGIN;	
				}
				else
				{
					TelnetState = SM_GET_PASSWORD;
				}
	
				// Username verified, throw this line of data away
				TCPGetArray(MySocket, NULL, w + 1);
	
				// Print the password prompt
				TCPPutROMString(MySocket, strPassword);
				TCPFlush(MySocket);
				break;
	
			case SM_GET_PASSWORD:
			case SM_GET_PASSWORD_BAD_LOGIN:
				// Make sure we can put the authenticated prompt
				if(TCPIsPutReady(MySocket) < strlenpgm((ROM char*)strAuthenticated))
					break;
	
				// See if the user pressed return
				w = TCPFind(MySocket, '\n', 0, FALSE);
				if(w == 0xFFFFu)
				{
					if(TCPGetRxFIFOFree(MySocket) == 0u)
					{
						TCPPutROMString(MySocket, (ROM BYTE*)"Too much data.\r\n");
						TCPDisconnect(MySocket);
					}
	
					break;
				}
	
				// Search for the password -- case sensitive
				w2 = TCPFindROMArray(MySocket, (ROM BYTE*)TELNET_PASSWORD, sizeof(TELNET_PASSWORD)-1, 0, FALSE);
				if((w2 != 3u) || !((sizeof(TELNET_PASSWORD)-1 == w-3) || (sizeof(TELNET_PASSWORD) == w-3)) || (TelnetState == SM_GET_PASSWORD_BAD_LOGIN))
				{
					// Did not find the password
					TelnetState = SM_PRINT_LOGIN;	
					TCPPutROMString(MySocket, strAccessDenied);
					TCPDisconnect(MySocket);
					break;
				}
	
				// Password verified, throw this line of data away
				TCPGetArray(MySocket, NULL, w + 1);
	
				// Print the authenticated prompt
				TCPPutROMString(MySocket, strAuthenticated);
				TelnetState = SM_AUTHENTICATED;
				// No break
		
			case SM_AUTHENTICATED:
				if(TCPIsPutReady(MySocket) < strlenpgm((ROM char*)strDisplay) + 4)
					break;
	
				//TCPPutROMString(MySocket, strDisplay);
				TelnetState++;
	
				// All future characters will be bold
				TCPPutROMString(MySocket, (ROM BYTE*)"\x1b[1m");
	
			case SM_REFRESH_VALUES:
			
					//TCPPrintString("ura!");
					
					//if(TCPRxInBuf() > 0){
					//	TCPPutChar(TCPGetChar() + 1);
					//}
					
					if(TCPIsPutReady(MySocket) >= 100u){					
				        MonitorMedia(&TCPPrintString, &TCPPutChar);
	        			MonitorUser(&TCPPrintString, &TCPPutChar, &TCPGetChar, &TCPRxInBuf);    
	        			console(&TCPPrintString, &TCPPutChar, &TCPGetChar, &TCPRxInBuf);
					}

/*	
				if(TCPIsGetReady(MySocket))
				{
					TCPGet(MySocket, &i);
					switch(i)
					{
						case '\r':
						case 'q':
						case 'Q':
							if(TCPIsPutReady(MySocket) >= strlenpgm((ROM char*)strGoodBye))
								TCPPutROMString(MySocket, strGoodBye);
							TCPDisconnect(MySocket);
							TelnetState = SM_PRINT_LOGIN;							
							break;
					}
				}
*/				
	
				break;
		}


		// Save session state back into the static array
		hTelnetSockets[vTelnetSession] = MySocket;
		vTelnetStates[vTelnetSession] = TelnetState;
	}
}

WORD wFreeTXSpace;
// ******************************************************************************************************
void TCPPutChar(char data){
	
	
	wFreeTXSpace = TCPIsPutReady(MySocket);
	
	//TCP_SOCKET	MySocket;
	//while (TCPIsPutReady(MySocket) < 150u) Nop();
	TCPPut(MySocket, data);
	TCPFlush(MySocket);
	TCPFlush(MySocket);
	
}

// ******************************************************************************************************
void TCPPrintString(char *data){

	//WORD wFreeTXSpace;
	wFreeTXSpace = TCPIsPutReady(MySocket);
	
	//if(wFreeTXSpace == 0u)
	//{
	//	TCPFlush(MySocket);
	//	return 0;
	
    //while (TCPIsPutReady(MySocket) < 150u) Nop();
    
    while (*data) TCPPut(MySocket, *data++); 
    
	TCPFlush(MySocket);
	TCPFlush(MySocket);

}

// ******************************************************************************************************
char TCPGetChar(void){
	BYTE i;
	
	if(TCPIsGetReady(MySocket))				{
		TCPGet(MySocket, &i);
		Nop();
	}
    return i;   
}

// ******************************************************************************************************
char TCPRxInBuf(void){
    BYTE numBytesRead;

    numBytesRead = TCPIsGetReady(MySocket);
    Nop();

    return (numBytesRead);    
}

#endif	//#if defined(STACK_USE_TELNET_SERVER)
