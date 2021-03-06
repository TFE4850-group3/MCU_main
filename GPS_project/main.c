/*
 * GccApplication2.c
 *
 * Created: 17.02.2016 10.04.47
 * Author : tgarp
 */ 


#define	F_CPU (8000000UL)
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

/* Define message you wish to be sent to controller. Check header.h for message specifications */
/* Defining message to be sent to GPS module should be done before the inclusion of header.h */
#define EMPTY

#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "header.h"

int main(void)
{
    /*------------------------------- INITIALIZATION PHASE -------------------------- */
    /* ------------------------------------------------------------------------------ */
	int par_check = 0;
	/* Set PORTA as output for parallel data out */
	DDRA = 0xFF;
	//set PB0 as output (connected to reset on GPS-module)
	DDRB = (1<<DDB0);
	//set PB2 and PB3 as input, sensing for framestart
	DDRB &= ~(1<<DDB2);
	DDRB &= ~(1<<DDB3);
	//USART initialization
	USART_init();
	//Enable Global interrupt	
	sei();
	
	/*------------------------------- MAIN PHASE ------------------------------------ */
    /* ------------------------------------------------------------------------------ */
	while (1) 
	{   
	  /* If Framestart (PB3 and PB1) is set, send parallel data out on PORTA */
	  /* If data already sent during same framestart interval, do not send more data */

	  if((PINB & (1<<PINB3))&&((PINB & (1<<PINB2))&&(par_check == 0))){
			/* Turn of global interrupt because we don't want data storage in data buffer to
			be messed up when altering data buffer*/
			cli();
			par_check == 1;
			PORTA = data0[0];
			//Delete the sent data in databuffer, and shift array
			for (int i = 0; i<(sizeof(data0)/sizeof(data0[0]));i++){
				data0[i]=data0[i+1];
				if(rx0_count >0){
					rx0_count = rx0_count - 1;
				}
			}
	      sei();
	  }else{
	    par_check =0;
	  }
	  asm("nop");
	}
}

//USART0 Rx Complete interrupt. Will only activated if RXC0 flag in UCSR0A is set
ISR(USART0_RX_vect){
	cli();
	rx0_count = rx_routine(&UDR0, &data0, rx0_count);
	sei();
}

//USART0 UDRIE interrupt. Will only activate if transmit register is empty.
ISR(USART0_UDRE_vect){
	cli();
	UDR0= tx_msg[tx0_count];
	//Turn off transmitt interrupt when message is sent
	if(tx0_count==(sizeof(tx_msg)/sizeof(tx_msg[0]))-1){
	  msg_check = 1;
	  tx0_count = 0;
	  UCSR0B &= ~(1<<UDRIE0);
	}else{
	  tx0_count = tx0_count + 1;
	}
	sei();
	
}

int rx_routine(uint8_t *rx_mod, char *data, int count)
{
	char temp;
	temp = *rx_mod;
	
	//Dump all data when receiving command response
	if((temp==0xa1)&&(data[count-1]==0xa0)){			
		data[0]=data[count-1];
		count=1;		
	}
	data[count]= temp;
	count = count + 1;
	//Dump data when data buffer is too big
	if(count==290){
		count = 0;
	}
	return count;
}

void USART_init()
{
	//set GPS in reset mode (Reset is set to low)
	PORTB &= ~(1<<DDB0);		
	/* Set baud rate */
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	/* Enable receiver and transmitter */
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	/* Set frame format: 8data, 1stop bit */
	UCSR0C |= (1<<UCSZ00)|(1<<UCSZ01);
	//Enable Recieve interrupt for USART0
	UCSR0B |= (1<<RXCIE0);
	//Enable interrupt when transmit register is ready to be written to
	UCSR0B |= (1<<UDRIE0);	
	//Activate GPS (Reset is set to high)	
	PORTB |= 0x01;
	_delay_ms(500);
	asm("nop");
}
