

//------------------------------------------------------------------------------------------------------------------
//----------------------------------------------Declecration of global variables -----------------------------------
//------------------------------------------------------------------------------------------------------------------
//---------Variables USART0-----------
volatile int tx0_count = 0;
volatile int rx0_count = 0;
volatile int buf0_count = 0;
char data0[300];
//---------Variables USART1-----------
volatile int rx1_count = 0;
volatile int buf1_count = 0;
char data1[300];
//--------- Variables msg ------------
volatile int msg_check = 0;

//------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------Decleration of functions-------------------------------------
//------------------------------------------------------------------------------------------------------------------
void USART_init();
void master_spi_init();
int rx_routine(uint8_t *rx_mod, char *data, int count);

//---------------------------------------------------------------------------------------------
//---------------------------------DEFINE MESSAGE TO SEND -------------------------------------
//---------------------------------------------------------------------------------------------

/* Define which message you want to send to GPS module. Defining message is done by for example writing */
/* #define QSV to query software version. This must be done before #include "header.h" in main.c file */
uint8_t tx_msg[] = {
	0xA0, 0xA1,

#ifdef EMPTY	
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00,
#endif

#ifdef SR			/*SYSTEM RESTART*/
					//Not yet configured
					/*0x00, 0x02, 0x02, 0x00, 0x02,*/
#endif

#ifdef QSV			/*QUERY SOFTWARE VERSION*/		
	0x00, 0x02, 
	0x02, 0x00, 
	0x02,
#endif
#ifdef CRC			/*QUERY SOFTWARE CRC*/		
	0x00, 0x02, 
	0x03, 0x00, 
	0x03,
#endif
#ifdef DEF			/*SET FACTORY DEFAULTS*/	
	//reboots after setting to factory defaults	
	0x00, 0x02, 
	0x04, 0x01, 
	0x05,
#endif
#ifdef SER			/*CONFIGURE SERIAL PORT*/
	//set baud rate.
	0x00, 0x04, 
	0x05, 0x00, 
	0x06, 0x00, 
	0x03,
#endif
#ifdef NMEA_CONFIG	/*CONFIGURE NMEA MESSAGE*/
	// Sets GGA interval and turns off all other NMEA messages.
	0x00, 0x09, 
	0x08, 0x01, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x09,
#endif 

#ifdef NMEA_CONFIG	/*CONFIGURE SYSTEM POSITION RATE*/
	//set update rate to 50hz
	0x00, 0x03,
	0x0E, 0x32,
	0x00, 0x3f,
#endif
	0x0D, 0x0A
};

//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------
