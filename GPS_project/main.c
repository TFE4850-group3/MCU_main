/*
 * GccApplication2.c
 *
 * Created: 17.02.2016 10.04.47
 * Author : tgarp
 */ 

#define	F_CPU (8000000UL)
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "header.h"

//---------------------------------DEFINE MESSAGE TO SEND ------------------------------------
//---------------------------------------------------------------------------------------------
#define SER
// If SER is defined; define NEW_USART_BAUDRATE for microcontroller. 
#define NEW_USART_BAUDRATE 230400
#define NEW_BAUD_PRESCALE (((F_CPU / (NEW_USART_BAUDRATE * 16UL))) - 1)
uint8_t check = 0;

uint8_t tx_msg1[] = {
	0xA0, 0xA1,
	0x00, 0x03,
	0x0E, 0x32,
	0x00, 0x3f,
	0x0D, 0x0A
};

uint8_t tx_msg[] = {
	0xA0, 0xA1,

#ifdef TEST
0x2B, 0x2B, 0x2B, 0x2B, 0x2B,0x2B, 0x2B, 0x2B, 0x2B, 0xd, 0xa, 0x2B, 0x2B, 0x2B, 0x2B, 0x2B,0x2B, 0x2B, 0x2B, 0x2B, 0xd, 0xa
#endif	
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
 
uint8_t usart_reinit = 0;


int main(void)
{
	
	DDRB = 0xff;
	//master_spi_init();
	USART_init();
	//Enable Global interrupt	
	sei();
    while (1) 
    {			
		//When changing Baud Rate we have to use this delay in order to "catch up" with faster Baud Rate.
		_delay_ms(100);
		
		//Set new values in UBRRxH and UBRRxL using NEW_BAUD_PRESCALE calculated earlier.
		if((msg_check==1)&&(usart_reinit==1)){
			
			cli();
			UBRR0H = 0x00;
			UBRR0L = 0x00;
			UBRR1H = 0x00;
			UBRR1L = 0x00;
			UBRR0H = (unsigned char)(NEW_BAUD_PRESCALE>>8);
			UBRR0L = (unsigned char)NEW_BAUD_PRESCALE;
			UBRR1H = (unsigned char)(NEW_BAUD_PRESCALE>>8);
			UBRR1L = (unsigned char)NEW_BAUD_PRESCALE;
			sei();
			if(check==0){
				msg_check = 0;
			}
			
			check = 1;
		
		}
		
		/* Start transmission */
		//SPDR = data0[rx0_count];
		//SPDR = 0x24;
		/* Wait for transmission complete */
		//while(!(SPSR & (1<<SPIF)))
		//;
		
		asm("nop");
    }
}

//USART0 Rx Complete interrupt. Will only activated if RXC0 flag in UCSR0A is set
ISR(USART0_RX_vect){
	cli();
	rx0_count = rx_routine(&UDR0, &data0, rx0_count);
	sei();
}

//USART1 Rx Complete interrupt. Will only activated if RXC1 flag in UCSR1A is set
ISR(USART1_RX_vect){
	cli();
	rx1_count = rx_routine(&UDR1, &data1, rx1_count);
	sei();
}

//USART0 UDRIE interrupt. Will only activate if transmit register is empty.
ISR(USART0_UDRE_vect){
	cli();
	
	UDR0= tx_msg[tx0_count];
	//Turn off transmitt interrupt when message is sent
	if(check==0){
		if(tx0_count==(sizeof(tx_msg)/sizeof(tx_msg[0]))-1){
			msg_check = 1;
			tx0_count = 0;
			//UCSR0B &= ~(1<<UDRIE0);
		}
	}else if(check==1){
		if(tx0_count==(sizeof(tx_msg1)/sizeof(tx_msg1[0]))-1){
			msg_check = 1;
			tx0_count = 0;
			UCSR0B &= ~(1<<UDRIE0);
		}
	}
	
	tx0_count = tx0_count + 1;
	sei();
}

int rx_routine(uint8_t *rx_mod, char *data, int count)
{
	char temp;
	temp = *rx_mod;
	/*Check for NMEA message start and add '\0' into end of previous NMEA message.
	This is done because we want it to be a string*/ 
	if((temp == '$')&&(count>0)){						
		data[count]='\0';
		count = count + 1;
	//Dump all data when receiving command response
	}else if((temp==0xa1)&&(data[count-1]==0xa0)){			
		data[0]=data[count-1];
		count=1;		
	}/*else if((temp==0x0a)&&(data[count-1]==0x0d)){
		count =0;
	}*/
	data[count]= temp;
	count = count + 1;
	//Dump data when data buffer is too big
	if(count==290){
		count = 0;
	}
	return count;
}

void master_spi_init()
{
	/* Set MOSI, SS and SCK output, all others input */
	DDRB = 0xB0;
	//PORTB |= 0x10;
	SPSR = (1<<SPI2X);
	/* Enable SPI, Master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR);
}

void USART_init()
{
	//set GPS in reset mode (Reset is set to low)
	PORTB = 0x00;
	
	#ifdef SER
	usart_reinit = 1;
	#endif
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	UBRR1H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR1L = (unsigned char)BAUD_PRESCALE;
	/* Enable receiver and transmitter */
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR1B |= (1<<RXEN1)|(1<<TXEN1);

	/* Set frame format: 8data, 1stop bit */
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);
	UCSR1C |= (1<<UCSZ10)|(1<<UCSZ11);
		
	//Enable Recieve interrupt for USART0
	UCSR0B |= (1<<RXCIE0);
	//Enable Recieve interrupt for USART1
	UCSR1B |= (1<<RXCIE1);
	//Enable interrupt when transmit register is ready to be written to
	UCSR0B |= (1<<UDRIE0);	
	//Activate GPS (Reset is set to high)	
	PORTB |= 0x01;

	_delay_ms(500);
	asm("nop");
	
	
	
}