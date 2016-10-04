/*****************************************************************************
 * Author               Date        Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Vlad Vazhko		01/05/11
 *****************************************************************************/

#define THIS_IS_STACK_APPLICATION
#define CONSOLE

#include "MainPult.h"

// Configuration bits
_CONFIG1(JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF & FWDTEN_OFF & ICS_PGx3)
_CONFIG2(0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV2 & IOL1WAY_OFF)

#ifdef __DEBUG
//_FWDT(FWDTEN_OFF);
//#error   IS DEBUG!
#endif

#ifdef __RELEASE
//_FWDT(FWDTEN_ON & WINDIS_OFF & WDTPRE_PR128 & WDTPOST_PS512);
//#error  IS REALEASE!
#endif



/////////////////////////////////////////////////////////////////////////////
//                            LOCAL PROTOTYPES
/////////////////////////////////////////////////////////////////////////////
void StartScreen();                                // draws intro screen
void TickInit(void);                               // starts tick counter
void proc_btn(void);

WORD PultWorkFlow(void);

static void InitializeSystem(void);
static void InitAppConfig(void);

void USBProcessIO(void);
void USBDeviceTasks(void);

void USBUARTPrintString(char*);
void USBUARTPutChar(char);
char USBUARTGetChar(void);
char USBUARTRxInBuf(void);



/////////////////////////////////////////////////////////////////////////////
//                       GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////
volatile QWORD  tick = 0;                                   // tick counter
extern volatile SHORT get_touch;

//volatile char fat_buffer[4096];


char USB_In_Buffer[64];
char USB_Out_Buffer[64];

BYTE g_debug = 0;

//BYTE gState;

PULT_STATES pultState = PULT_INIT;

WORD shots_point = 0; // debug

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
static unsigned short wOriginalAppConfigChecksum;   // Checksum of the ROM defaults for AppConfig

/////////////////////////////////////////////////////////////////////////////
//                            EXTERN PROTOTYPES
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//                            MAIN
///////////////////////////////////////////////////////////////////////////// 
int main(void)
{
	static QWORD    prevTick;
    GOL_MSG msg;
    
    

    InitializeSystem();
    
	if ((!BTN_S1)&&(!BTN_S2)) {
	        g_debug = 1;
	}    

    
    ParamLoad();
    
    GOLInit();    
    CreateSchemes(); 
    

    
    
    //SetColor(WHITE);
    //ClearDevice();
    //WAIT_UNTIL_FINISH(PutImage(80, 60, (void *) &terex_icon_small, 1));    
    //DelayMs(1000);
    WAIT_UNTIL_FINISH(PutImage(0, 0, (void *) &terex_icon_small, 2));    
    DelayMs(1000);
    
/*
    SetColor(BRIGHTBLUE);
    SetFont((void *) &GOLFontDefault);
    MoveTo((GetMaxX() - GetTextWidth((XCHAR *)text, (void *) &GOLFontDefault)), 215);   
*/    
    
    
   	USBDeviceInit();
   	
   	
#if defined(USB_INTERRUPT)
       //if (USB_BUS_SENSE && (USBGetDeviceState() == DETACHED_STATE)) {
           USBDeviceAttach();
       //}
#endif


/*
    if ((!BTN_S1)&&(!BTN_S2)) {
        TouchCalibration();
        TouchStoreCalibration();        
    } 
*/       
    TouchLoadCalibration();
    
#ifdef CONSOLE
    console_init();
    //MonitorMedia(&USBUARTPrintString, &USBUARTPutChar);
#endif

    if (smd004_init() == 0){
	    ///Varning((XCHAR *)VarnSMDStr);
	}
	
    if (aduc_init() == 0){
	    if (aduc_init() == 0){
	    	//Varning((XCHAR *)VarnADUCStr);
	    }
	}
		

#ifdef USE_ETH_TELNET   
    InitAppConfig();
    StackInit();
#endif    
    
    
    
    sprintf(gShift.name1, "Имя");
    sprintf(gShift.name2, " не ");
    sprintf(gShift.name3, "указано");
    sprintf(gShift.shift, "Не_указана");    
    sprintf(gSample.sample, "XXXXXX-Y-Z");
    
    Beep();


    while (1) {

        ClrWdt();
/*        
        if (process_messages()){
	        	PultWorkFlow();
				//GOLMsg(&msg);
	    }

*/        
        
        if(get_touch){
    		TouchProcessTouch();
    		get_touch = 0;
     	}

        if (GOLDraw()) {
                       
           TouchGetMsg(&msg);
           
           GOLMsg(&msg);
           
			if((tick - prevTick) > 10){
				SideButtonsMsg(&msg);   // Get message from side buttons
				/*	
				if (screenState == DISPLAY_WF) {		                      	
		                      	if(shots_point < 305){
			                      	g_curr_shot.last.steps ++;
									AddShotPoint(&g_curr_shot);
									AddWfPointLb(&g_curr_shot); 
									shots_point ++;
								} else if(shots_point == 305){
									SaveShotFile(&g_curr_shot);
									shots_point ++;
								}
		       }
		       */			
				GOLMsg(&msg);
			}	            


           PultWorkFlow();
           GOLMsg(&msg);          

        }
        

		

#ifdef USE_ETH_TELNET
	    StackTask();
	    StackApplications();
#endif 	    

        USBProcessIO();


    }

    return 1;
}






//********************************************************************
WORD process_messages(/*GOL_MSG *pMsg*/ void){
	
	static GOL_MSG msg;
	static QWORD    prevTick;
	
	if(get_touch){
   		TouchProcessTouch();
   		get_touch = 0;
    }

   if (GOLDraw()) {
                      
		TouchGetMsg(&msg);
		
		GOLMsg(&msg);
          
		if((tick - prevTick) > 10){
			SideButtonsMsg(&msg);   // Get message from side buttons
			
			GOLMsg(&msg);
		}	            

	
		return 1;

	}
	
	return 0;
	
}

/*********************************************************************
* Function: Timer4 ISR
* Overview: increments tick counter. Tick is approx. 1 ms.
********************************************************************/
#define __T4_ISR    __attribute__((interrupt, shadow, auto_psv))
void __T4_ISR _T4Interrupt(void)
{
    tick++;        
    IFS1bits.T4IF = 0;    
    //Nop();
    

}
//********************************************************************
DWORD TickGet(void) {
    return(DWORD)tick;
}
//********************************************************************
DWORD TickGetDiv256(void) {
    return(DWORD)(tick >> 8);
}

/*********************************************************************
* Function: void TickInit(void)
* Overview: sets tick timer
********************************************************************/
// for a system clock of 32 MHz
#define TICK_PERIOD GetPeripheralClock()/1000
void TickInit(void)
{   
    // Initialize Timer4
    TMR4 = 0;
    PR4 = TICK_PERIOD;
    IFS1bits.T4IF = 0;  //Clear flag
    IEC1bits.T4IE = 1;  //Enable interrupt
    T4CONbits.TON = 1;  //Run timer
}


/**********************************************************************/
static void InitializeSystem(void)
{

    //vv
    AD1CON1 = 0;
    AD1PCFGL = 0xffff;
    AD1PCFGH = 0xffff;

    TRISB = 0xffff;
    TRISD = 0xffff;  

    _CN59PUE = 1;
    _CN60PUE = 1;
    _CN61PUE = 1;
    _CN62PUE = 1;
    _CN63PUE = 1;   
    _CN11PUE = 1; 



    // Configure SPI1 PPS pins (ENC28J60/ENCX24J600/MRF24WB0M or other PICtail Plus cards)
    RPOR14bits.RP29R = 8;       // Assign RP29 to SCK1 (output)	
    RPOR0bits.RP1R = 7; // Assign RP1 to SDO1 (output)	
    RPINR20bits.SDI1R = 13; // Assign RP13 to SDI1 (input) 

    _TRISB15 = 0;
    _TRISB1 = 0;
    _TRISB2 = 1;

    ENC_CS_IO = 1;
    ENC_CS_TRIS = 0;



    //spi2
    _TRISG6 = 0;    
    _TRISG7 = 0;    
    _TRISG8 = 1;   

    RPINR22 = 19; // assign RP19 for SDI2
    RPOR10bits.RP21R = 11;                  // assign RP21 for SCK2
    RPOR13bits.RP26R = 10;                   // assign RP19 for SDO2  

    SPI2CON1 = 0b0000100000;    
    SPI2CON1bits.CKE = 0;
    SPI2CON1bits.CKP = 1;
    SPI2CON1bits.SMP = 0;    
    SPI2STAT = 0x8000;
    
    //MonitorMedia(&USBUARTPrintString, &USBUARTPutChar);
    
    
    InitI2C(); 
	RTCCInit();
	TouchInit();
    BeepInit();    
    TickInit();    

    ///StartScreen();
    ///SetColor(BLACK);
    ///ClearDevice();
		
	
 	///for( i = 0; i < 32; i ++) HDByteReadI2C(0xa0, 0, i, &i2c_r[i], 1);
 	Nop();  
  
    //U1

    _LATB7 = 0;
    _TRISB6 = 1;    
    _TRISB7 = 0;
    UART1Init();        
    _U1RXR = 6;
    _RP7R = 3;  


    //U2	
    _U2RXR = 10;
    _RP17R = 5; 
    _TRISF5 = 0;
    _TRISF4 = 1;
    UART2Init();


    // перенаправил U2 на U1
    //_U2RXR = 6;	// Assign RF4/RP10 to U2RX (input)
    //_RP7R = 5;	// Assign RF5/RP17 to U2TX (output)    
 
                     
                        
}//end InitializeSystem












/********************************************************************
 * Function:        void ProcessIO(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        This function is a place holder for other user
 *                  routines. It is a mixture of both USB and
 *                  non-USB tasks.
 *
 * Note:            None
 *******************************************************************/
void USBProcessIO(void){   

    char    status;
    
/*   
#if defined(USB_INTERRUPT)
       if (USB_BUS_SENSE && (USBGetDeviceState() == DETACHED_STATE)) {
           USBDeviceAttach();
       }
#endif
*/
    
    

    if ((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    if ((USBUSARTIsTxTrfReady()) && (/*FirstSendState == 1*/1)) {
#ifdef CONSOLE 
IEC0bits.T3IE = 0; 
        status = MonitorMedia(&USBUARTPrintString, &USBUARTPutChar);
IEC0bits.T3IE = 1;        
        MonitorUser(&USBUARTPrintString, &USBUARTPutChar, &USBUARTGetChar, &USBUARTRxInBuf);
        console(&USBUARTPrintString, &USBUARTPutChar, &USBUARTGetChar, &USBUARTRxInBuf);
       
#endif 
    }


    CDCTxService();


}   //end ProcessIO




// ******************************************************************************************************
// ************** PultWorkFlow()*************************************************************************
// ******************************************************************************************************
#define MES_COUNT_VAH_IN_POINT 1
#define MES_COUNT_UES_IN_POINT 3

WORD PultWorkFlow(void){

    static QWORD    prevTick, prevTickInt;      // keeps previous value of tick 
    static PULT_STATES i_prev_state;
    static BYTE interr = 0xff, prev_interr = 0x00, motor_status = 0;
    static BYTE tryes, over, too_high; 
    static BYTE mes_in_point, mesvah_in_point; // счетчики измерений в одной точке
    static ADUC_CURRENT aduc_curr = CURR_10UA;
    static double u_pos, u_neg, u, u_type, u_type_buff[MES_COUNT_VAH_IN_POINT];
    static WORD steps, microsteps, timetick;
    static char ctype = ' ', ctype_buff[MES_COUNT_VAH_IN_POINT];
    
    static BYTE	btemp, index; //static !!    
    BUTTON  *pBtn;
    
        
    



	if(smd004_error_msg != NULL){
		
		StSetText(pStVarning, (XCHAR*)smd004_error_msg);
		SetState(pStVarning, ST_DRAW );
		pultState = PULT_IDLE;
		smd004_error_msg = NULL;
		
	} else if (aduc_error_msg != NULL){
	
		StSetText(pStVarning, (XCHAR*) aduc_error_msg);
		SetState(pStVarning, ST_DRAW );
		aduc_error_msg = NULL;		
	} 
	
	// опред. где надо быстро опрашивать SMD
	if (pultState == WF_MES_TYPE){
		timetick = 250;
	} else {
    	timetick = 500;
    }
    
    // опрос концевиков
	if(((tick - prevTickInt) > timetick) && ((screenState == DISPLAY_WF) || (screenState == DISPLAY_WIN_UES))) {
		
			smd004_get_status(&motor_status, &interr);
			
			//process_messages();
			
			if(((TSTBIT(interr, SMD004_INT11)) == 0) && (screenState == DISPLAY_WF) && (pultState == PULT_IDLE)){			
				aduc_set_current(0);
				aduc_get_vah(&u_type);
				
				g_curr_shot.last.ut = u_type;				
				g_curr_shot.last.rs = 0.0;
				g_curr_shot.last.un = 0.0;
				g_curr_shot.last.up = 0.0;
				g_curr_shot.last.attempt = 0;
				g_curr_shot.last.curr = 0;
				g_curr_shot.last.steps = 0;
	
				update_win_wf(&g_curr_shot);				 

			}		
	    	

			if((TSTBIT(interr, SMD004_INT21) == 0) || (TSTBIT(interr, SMD004_INT22) == 0)){
		    	StSetText(pStVarning, (XCHAR *)StopRusStr);
            	SetState(pStVarning, ST_DRAW );
            	

				if(prev_interr != interr){
					//отводим зонды
					smd004_set_mode(1, 0x81, 500);
					smd004_start(1);
					smd004_step2_set_keep(0);
				}
				
            	// чтобы дать стартануть
            	if((pultState == PULT_MOVE))
            		pultState = PULT_IDLE;
		    } else {
		    	// выводить ссбщения о состоянии движения		    		    	
		    }
		    
		    prev_interr = interr;
//	    }
	    
	    
		if((screenState == DISPLAY_WIN_UES)){
			update_winUES(interr);
		}
		
 	    
	    prevTickInt = tick;
	    
	} else {	
		
	}
	
	


    switch(pultState)  {
        
        default:
        
        case PULT_INIT:                	
        	smd004_set_mode(1, 0x81, 500);
			smd004_start(1);
			DelayMs(600);
			smd004_set_mode(1, 0x01, 100);
			smd004_start(1);
			
			// определения нуля в ВАХ
			#ifdef THERMO
				aduc_set_unshorted_vah(0);
			#else
				aduc_set_unshorted_vah(1);
			#endif
					
			DelayMs(300);
						
			aduc_calibrate_vah();
			aduc_set_unshorted_vah(1);
			
			smd004_set_mode(1, 0x81, 500);
			smd004_start(1);			
			
        	
        	prevTick = tick;
        	prevTickInt = tick;
        	
        	pultState = PULT_IDLE;
        	break;
        	
        	
        case PULT_IDLE:
        	prevTick = tick;
        	break;
        	
        	
        case CMD_MOVE_TO_START_POINT:        
       		smd004_set_begin();
        	pultState = PULT_MOVE;
        	StSetText(pStVarning, (XCHAR *)ReturnStr);
           	SetState(pStVarning, ST_DRAW );

        	prevTick = tick;
        	break;        	

               	       
        case CMD_WF_START:        	
        	
        	
        	// запрещаем кнопку старт
        	pBtn = (BUTTON *)GOLFindObject(ID_BUTTON_START);
			SetState(pBtn, BTN_DRAW | BTN_DISABLED);  
			      	
        	smd004_step2_set_keep(1);
        	microsteps = 0;
        	steps = 0;
        	
        	g_curr_shot.last.un = 0.0;
        	g_curr_shot.last.up = 0.0; 
        	g_curr_shot.last.rs = 0.0; 
        	g_curr_shot.last.ues = 0.0; 
        	
        	update_win_wf(&g_curr_shot);
        	
        	//просто отшагиавем от концевика на 10        	
        	if (TSTBIT(interr, SMD004_INT21) == 0){
	        	smd004_set_mode(2, 0x01, FIRST_STEP);
	        	steps += FIRST_STEP;       	
	        	smd004_start(2);
        	}
        	
        	
        	pultState = WF_FIRST_STEP;
        	prevTick = tick;
        	break;
        	
        	
        case WF_FIRST_STEP:
        	
        	//ждем отшагивария и устанавливаем маршевый шаг         	     	
        	if((tick - prevTick) < 500)  break; 
        	
        	StSetText(pStVarning, (XCHAR *)MoveStr);
            SetState(pStVarning, ST_DRAW );
            
        	// устанавливаем, на сколько шагать
			smd004_set_mode(2, 0x01, g_microsteps);
			///smd004_set_mode(2, 0x01, g_steps);
        	prevTick = tick;
        	pultState = WF_MOVE_TO_SAMPLE;
        	break;
        	
        	
        case WF_ONE_STEP:
        	//ждем отъезда и шагаем
        	if((tick - prevTick) < 250)  break;
			
			
			mesvah_in_point = 0;			
			
			// не отказ от типа
			///steps += g_microsteps;
        	///aduc_set_current(0);        	
        	///pultState = WF_MES_TYPE;
        	        	
        	
        	// отказ от типа
        	steps += g_steps;
        	pultState = WF_WAIT_ONE_STEP; 
        	smd004_set_mode(2, 0x01, g_steps);
        	
        	
        	
        	smd004_start(2); 
        	       	
        	prevTick = tick;
        	
        	break;
        	
        case WF_WAIT_ONE_STEP: // ждем отшаг.
        	if((tick - prevTick) < 1000)  break;
        	pultState = WF_MOVE_TO_SAMPLE; 
        	prevTick = tick;
        	break;         	
     	
        	
        case WF_MES_TYPE:        	
        	
        	//ждем
        	if((tick - prevTick) < 50)  break;
        	
        	// отработка концевиков
			// если наехали на левый, то запись и возврат
			if(TSTBIT(interr, SMD004_INT22) == 0){
				pultState = CMD_WF_AUTO_STOP;
				break;
			}
	        	
		    if (motor_status != 0) {
	        	prevTick = tick;
	        	break;
	        } 
        	
        	// определяем тип проводимости в 3 точках!     	        	
        	aduc_get_vah(&u_type);         	
        	ctype_buff[mesvah_in_point]= get_semi_type(u_type);
        	mesvah_in_point ++;
        	
        	if(mesvah_in_point >= MES_COUNT_VAH_IN_POINT){
	        	//обработка	устанавливаем тип проводимости (приоритет N)       	
	        	btemp = calc_semi_type(&ctype_buff[0]);
	        	mesvah_in_point = 0;	        	
	        } else {	        	
	        	prevTick = tick;
	        	break;
	        }
        	
        	g_curr_shot.last.un = 0.0;
        	g_curr_shot.last.up = 0.0;       	
        	g_curr_shot.last.ut = u_type; 
        	// устанавливаем тип проводимости
        	///g_curr_shot.last.type = btemp;		
			g_curr_shot.last.steps = steps;
			g_curr_shot.last.rs = NO_UES;
									
        	update_win_wf(&g_curr_shot);
        	

        	///btemp = get_semi_type(u_type);
        	// если тип вдруг стал отличаться, то регистрируем точку
        	if (ctype != btemp){
	        	ctype = btemp;	        	
	        	AddShotPoint(&g_curr_shot);
				AddWfPointLb(&g_curr_shot);
	        	
	        } 
        	
        	
			microsteps += g_microsteps;
			// пора ли измерить УЭС?
			if(microsteps >= g_steps) {
				microsteps = 0;
				pultState = WF_MOVE_TO_SAMPLE; 
				prevTick = tick;
			}	else {
				pultState = WF_ONE_STEP;
				prevTick = tick;
				prevTick += 400;
			}			
			
			
			
			too_high = 0;
        	
        break;       		
       	
        	
         case WF_TRY_AGAN:
         	// повторяем измерение - отводим 
			smd004_set_mode(1, 0x81, 500);
			smd004_start(1);		
			
        	prevTick = tick;
        	// и отъезжаем        	
        	pultState = WF_TRY_AGAN2;
        	break;
        	
        	                	
         case WF_TRY_AGAN2:
         	// повторяем измерение - отъезжаем 
			if((tick - prevTick) < 200)  break;
			// если g_microsteps большой
			
			
			smd004_set_mode(2, 0x01, g_microsteps);
			smd004_start(2);
			microsteps += g_microsteps;
			steps += g_microsteps;
        	prevTick = tick;
        	// и наезжаем        	
        	///pultState = WF_MOVE_TO_SAMPLE;
        	// нет сначала измеряем тип
        	microsteps = g_steps;
        	//pultState = WF_MES_TYPE;
        	pultState = WF_MOVE_TO_SAMPLE;
        	break;
        	
        	
         case WF_MOVE_TO_SAMPLE:
			//ждем, пока отшагает и даем команду наехать зондами к образцу			
        	//ждем
        	if((tick - prevTick) < 100)  break;        	
        	// отработка концевиков
			// если наехали на левый, то запись и возврат
			if(TSTBIT(interr, SMD004_INT22) == 0){
				pultState = CMD_WF_AUTO_STOP;
				break;
			}
			
	        /*	
		    if (motor_status != 0) {
	        	prevTick = tick;
	        	break;
	        }
	        */
			
			smd004_set_mode(1, 0x01, 500);
			smd004_start(1);			
			pultState = CMD_WF_MESUARE;
        	prevTick = tick;        	
        	break;
        	
        	 
        case CMD_WF_MESUARE:
        	// ждем, когда наедет и стартем измерения
        	if((tick - prevTick) < 500)  break;
        	
        	aduc_calibrate_vah();        	
        	aduc_set_unshorted(1);
        	
			// ток на исходный уровень						
			tryes = 0;
			over = 0;
			aduc_curr = CURR_10UA;       	
        	        	
        	pultState = WF_SET_MESUARE;
        	prevTick = tick;
        	break;
        	
        	
         case WF_SET_MESUARE:
			
			tryes ++;
						
			aduc_set_current(aduc_curr);
						
			aduc_set_direction(0);
			aduc_get_ues(&u_pos);
						
			aduc_set_direction(1);
			aduc_get_ues(&u_neg);
			
		
			if (tryes > 5){
				
				too_high = 1;
				// todo: обработка неудачные измерения				
				pultState = WF_GET_MESUARE;								
			} else 	if((fabs(u_pos) < U_MIN) && (fabs(u_neg) < U_MIN) && (over == 0)){
				if(aduc_curr < CURR_1A) {
					aduc_curr ++;
					pultState = WF_SET_MESUARE;
				}
			} else if((fabs(u_pos) > U_SUPERMAX) || (fabs(u_neg) > U_SUPERMAX) || (((fabs(u_pos) + fabs(u_neg))/2.0) > U_MAX)){
				if(aduc_curr > CURR_100UA) {
					aduc_curr --;
					over= 1;
					pultState = WF_SET_MESUARE;
				} else {
					if((fabs(u_pos) < U_SUPERMAX) && (fabs(u_neg) < U_SUPERMAX)) pultState = WF_GET_MESUARE;
				}

			} else {
				// ток установлен, измеряем				
				pultState = WF_GET_MESUARE;
			}
			
        	prevTick = tick;
        	break;      	
		
		 
		 	
         case WF_GET_MESUARE:			
			
			if(/*(u_pos == TOO_HI) && (u_neg == TOO_HI)*/too_high == 1){
				u = TOO_HI;
				
			} else {
				// УЭС корректный
				u = (fabs(u_pos) + fabs(u_neg))/2.0;
				// экв. сопротивление
				u = u / fCurrentValue[aduc_curr];				
			}
			
			too_high = 0;
			


			g_curr_shot.last.rs = u;			
			get_ues(&g_curr_shot);			
			g_curr_shot.last.aues[mes_in_point] = g_curr_shot.last.ues; //add
			
			g_curr_shot.last.un = u_neg;
			g_curr_shot.last.up = u_pos;
			g_curr_shot.last.attempt = tryes;
			g_curr_shot.last.curr = aduc_curr;
			g_curr_shot.last.steps = steps;
			g_curr_shot.last.ut = u_type; // 		
			
			
			//!!
			///AddShotPoint(&g_curr_shot);
			// надо проанализировать и ввести только одно значение
			
			
			//AddWfPointLb(&g_curr_shot);
			update_win_wf(&g_curr_shot);		
			
			
			
			
			// повторяем измерение
			if(u == TOO_HI){
				pultState = WF_TRY_AGAN;
			} else {
				// измеряем дальше в нормальном цикле
				pultState = WF_MOVE_FROM_SAMPLE;
				Beep();			
				mes_in_point ++;
			}
			
			aduc_set_current(0);
        	
        	prevTick = tick;
        	
        	break;
        	
        	
         case WF_MOVE_FROM_SAMPLE:
			if((tick - prevTick) < 10)  break;
			smd004_set_mode(1, 0x81, 500);
			smd004_start(1);
        	
        	// делаем измерения по трем точкам
        	if(	mes_in_point > 2){
        		pultState = WF_ONE_STEP;
        		mes_in_point = 0;
        		
        		//add analys 3 points
        		if(g_curr_shot.last.aues[0] < g_curr_shot.last.aues[1]) index = 0; else index = 1;
        		if(g_curr_shot.last.aues[index] > g_curr_shot.last.aues[2]) index = 2;
        		if(g_curr_shot.last.aues[index] != 0.0)
        			g_curr_shot.last.ues = g_curr_shot.last.aues[index];
        			
        			
        		// если новое измерение больше в 5 раз, то не запис и повторяем	
        		if((g_curr_shot.last.prev_ues  * 3.0) < g_curr_shot.last.ues){
	        		g_curr_shot.last.prev_ues = g_curr_shot.last.ues;
	        		g_curr_shot.last.ues = NO_UES; //add
	        		pultState = WF_TRY_AGAN;
	        		prevTick = tick;
        			break;
	        	}
        		
        		g_curr_shot.last.prev_ues = g_curr_shot.last.ues;
        		
        		// добавляем в массив
        		AddShotPoint(&g_curr_shot);
        		// обновляем на экране
        		AddWfPointLb(&g_curr_shot);
				update_win_wf(&g_curr_shot);
        		
        		
        	}else{
        		pultState = WF_ONE_MORE;
        	}
        		
        	prevTick = tick;	
        		
        	break; 
              	
        	
        case WF_ONE_MORE: // доп. измерения УЭС
        	if((tick - prevTick) < 200)  break;
        	pultState = WF_MOVE_TO_SAMPLE; 
        	prevTick = tick;
        	break;   	  
        	    	
        	         	      	        	
       case CMD_STOP:        	
			StSetText(pStVarning, (XCHAR *)StopRusStr);
            SetState(pStVarning, ST_DRAW );
            smd004_stop(3);
            smd004_step2_set_keep(0);
            
            // разрешаем кнопку пуск
			pBtn = (BUTTON *)GOLFindObject(ID_BUTTON_START);
			ClrState(pBtn, BTN_DISABLED);			
			SetState(pBtn, BTN_DRAW);
            
			pultState = PULT_IDLE;
			prevTick = tick;
        	break;
        	
       case CMD_WF_AUTO_STOP:
      	    // останов по концевику
			smd004_step2_set_keep(0);
			SaveShotFile(&g_curr_shot);
			
			// разрешаем кнопку пуск
			pBtn = (BUTTON *)GOLFindObject(ID_BUTTON_START);
			ClrState(pBtn, BTN_DISABLED);			
			SetState(pBtn, BTN_DRAW);
						
			pultState = CMD_MOVE_TO_START_POINT;			
			
			prevTick = tick;
       		break;
        	        	
       case CMD_WF_STOP:        	
			smd004_set_mode(1, 0x81, 500);
			smd004_start(1);
			smd004_step2_set_keep(0);
			
					
			pultState = WAIT_WF_STOP;
			prevTick = tick;
        	break;
        	
        	
       case WAIT_WF_STOP:        	
			if((tick - prevTick) < 1000)  break;
			
			smd004_step2_set_keep(0);			
			
			pultState = CMD_STOP;
			prevTick = tick;			
        	break;
        	
        	
        case CMD_MOVE_UP:
            smd004_set_mode(1, 0x01, 1000);
			smd004_start(1);
        	prevTick = tick;
			pultState = PULT_MOVE;			
        	break;
        	
        	
         case CMD_MOVE_DN:
            smd004_set_mode(1, 0x81, 1000);
			smd004_start(1);
        	prevTick = tick;
			pultState = PULT_MOVE;			
        	break; 
        	
        	      	
        case CMD_MOVE_LT:        
        	StSetText(pStVarning, (XCHAR *)MoveStr);
            SetState(pStVarning, ST_DRAW );
            smd004_step2_set_keep(0);
            smd004_set_mode(2, 0x81, 5000);
			smd004_start(2);
        	prevTick = tick;
			pultState = PULT_MOVE;			
        	break;
        	
        	
         case CMD_MOVE_RT:        
        	StSetText(pStVarning, (XCHAR *)MoveStr);
            SetState(pStVarning, ST_DRAW );
            smd004_step2_set_keep(0);
            smd004_set_mode(2, 0x01, 5000);
			smd004_start(2);
        	prevTick = tick;
			pultState = PULT_MOVE;			
        	break;        	

        	        	         	        	
         case PULT_MOVE:
        	prevTick = tick;			
        	break;       	
        	
        	
        		
        
    }	
	
}






// ******************************************************************************************************
// ************** USB Console Functions *****************************************************************
// ******************************************************************************************************

// ******************************************************************************************************
void USBUARTPutChar(char data){
    WORD i = 0;

    for (i = 10000; i > 0; --i) {
        if (mUSBUSARTIsTxTrfReady()) break;
        CDCTxService();
    }
    
    //if(i == 0) Varning((XCHAR *)ErrorStr);

    if (USBUSARTIsTxTrfReady()) {
        USB_In_Buffer[0] = data;
        putUSBUSART(USB_In_Buffer, 1);      

    }

    CDCTxService(); 

}

// ******************************************************************************************************
void USBUARTPrintString(char *data){
    WORD i = 0;

    for (i = 10000; i > 0; --i) {
        if (mUSBUSARTIsTxTrfReady()) break;
        CDCTxService();
    }

    i = 0;
    while (*data) USB_In_Buffer[i++] = *data++;
    putUSBUSART(USB_In_Buffer, i);

    CDCTxService();


}

// ******************************************************************************************************
char USBUARTGetChar(void){

    return USB_Out_Buffer[0];   
}

// ******************************************************************************************************
char USBUARTRxInBuf(void){
    BYTE numBytesRead;

    numBytesRead = getsUSBUSART(USB_Out_Buffer,64);
    CDCTxService();

    return numBytesRead;    
}



// ******************************************************************************************************
// ******************************************************************************************************
void set_fs_time(void){
    RTCC dt;

    RTCCGetTime(&dt);
    SetClockVars(dt.yr+2000, dt.mth, dt.day, dt.hr, dt.min, dt.sec);    
}





// ******************************************************************************************************
// ******************************************************************************************************
// ******************************************************************************************************

static ROM BYTE SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void){


    while (1) {
        // Start out zeroing all AppConfig bytes to ensure all fields are 
        // deterministic for checksum generation
        memset((void*)&AppConfig, 0x00, sizeof(AppConfig));

        AppConfig.Flags.bIsDHCPEnabled = TRUE;
        AppConfig.Flags.bInConfigMode = TRUE;
        memcpypgm2ram((void*)&AppConfig.MyMACAddr, (ROM void*)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
//		{
//			_prog_addressT MACAddressAddress;
//			MACAddressAddress.next = 0x157F8;
//			_memcpy_p2d24((char*)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
//		}
        AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2<<8ul | MY_DEFAULT_IP_ADDR_BYTE3<<16ul | MY_DEFAULT_IP_ADDR_BYTE4<<24ul;
        AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
        AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2<<8ul | MY_DEFAULT_MASK_BYTE3<<16ul | MY_DEFAULT_MASK_BYTE4<<24ul;
        AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
        AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2<<8ul | MY_DEFAULT_GATE_BYTE3<<16ul | MY_DEFAULT_GATE_BYTE4<<24ul;
        AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2<<8ul  | MY_DEFAULT_PRIMARY_DNS_BYTE3<<16ul  | MY_DEFAULT_PRIMARY_DNS_BYTE4<<24ul;
        AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2<<8ul  | MY_DEFAULT_SECONDARY_DNS_BYTE3<<16ul  | MY_DEFAULT_SECONDARY_DNS_BYTE4<<24ul;

        // Load the default NetBIOS Host Name
        memcpypgm2ram(AppConfig.NetBIOSName, (ROM void*)MY_DEFAULT_HOST_NAME, 16);
        FormatNetBIOSName(AppConfig.NetBIOSName);

        // Compute the checksum of the AppConfig defaults as loaded from ROM
        wOriginalAppConfigChecksum = CalcIPChecksum((BYTE*)&AppConfig, sizeof(AppConfig));

        break;
    }
}





