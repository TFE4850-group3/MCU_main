/*
 * GccApplication2.c
 *
 * Created: 17.02.2016 10.04.47
 * Author : tgarp
 */ 
#define	F_CPU (20000000UL)
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


volatile uint8_t data_send;
//---Message receive and message sent---
//tx_msg contains data that you want to send
uint8_t tx_msg[22]={0xA0, 0xA1, 0x00, 0x0F, 0x01, 0x04, 0x07, 0xDD, 0x09, 0x1B, 0x06, 0x29, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x0D, 0x0A};
//rx_msg contains data that you receive
int *rx_msg;
//---Counters---
//tx_count keeps track of where you are in the message	
volatile uint8_t tx_count = 0;
//rx_count keeps track of how big the message you receive is
volatile uint8_t rx_count = 0;
int data[22] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int main(void)
{
	
	DDRB = 0xff;
	PORTB = 0x00;
	
	/* Set baud rate */
	UBRR0H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR0L = (unsigned char)BAUD_PRESCALE;
	UBRR1H = (unsigned char)(BAUD_PRESCALE>>8);
	UBRR1L = (unsigned char)BAUD_PRESCALE;
	/* Enable receiver and transmitter */
	UCSR0B |= (1<<RXEN0)|(1<<TXEN0);
	UCSR1B |= (1<<RXEN1)|(1<<TXEN1);

	/* Set frame format: 8data, 2stop bit */
	UCSR0C |= (1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
	UCSR1C |= (1<<USBS1)|(1<<UCSZ10)|(1<<UCSZ11);
	//Enable Global interrupt
	
	//Enable Recieve interrupt
	UCSR0B |= (1<<RXCIE0);
	//Enable interrupt if UDRE0 is empty. If UDRE0 is one, the buffer is empty, and ready to be written to.
	//UCSR0B |= (1<<UDRIE0);
	//Enable Recieve interrupt
	UCSR1B |= (1<<RXCIE1);
	//Enable interrupt if UDRE0 is empty. If UDRE0 is one, the buffer is empty, and ready to be written to.
	//UCSR1B |= (1<<UDRIE1);
	sei();

    while (1) 
    {
		int a;
		a = sizeof(data)/sizeof(data[0]);
		for (int i=0; i<22; i++) {
			UDR0=tx_msg[i];
		}
		/* Wait for empty transmit buffer */
		//if(!(UCSR0A & (1<<UDRE0))){
		//	PORTB = 0x01;
		//}
		//PORTB = 0x01;
		//UDR0 = tx_msg[tx_count];
		//tx_count = tx_count + 1;
		//while ( !(UCSR1A & (1<<RXC1)) );
		/* Get and return received data from buffer */
		
		//rx_msg = malloc(sizeof(data));
		//rx_msg[rx_count]=data;
		//rx_count=rx_count+1;
		//PORTB ^= 0xff;
		/* Put data into buffer, sends the data */
		if(rx_count==22){
			asm("nop");
		}
		//}
		asm("nop");
    }
}

//USART0 Rx Complete interrupt. Will only activated if RXC0 flag in UCSR0A is set
ISR(USART0_RX_vect){
	//uint8_t data = UDR0;
	
	rx_msg = malloc(sizeof(data));
	rx_msg=data;
	rx_count = rx_count + 1;

}
//USART0 Rx Complete interrupt. Will only activated if RXC0 flag in UCSR0A is set
ISR(USART1_RX_vect){
	//PORTB = 0x01;
	data[rx_count]= UDR1;
	//data[rx_count]= UDR1;
	//rx_msg = malloc(sizeof(data));
	//rx_msg[rx_count]=data;
	rx_count=rx_count+1;
}

//USART0 Data Register Empty interrupt. Will only be activated if UDRE0 flag in UCSR0A is set.
ISR(USART0_UDRE_vect){
	//int n = sizeof(tx_msg)/sizeof(tx_msg[0]);
	data_send = tx_msg[tx_count];
	UDR0=tx_msg[tx_count];
	if(tx_count==22){
		tx_count = 0;
	} else{
		tx_count = tx_count + 1;	
	}
	UCSR0A &= ~(1<<UDRE0); 
}

//USART0 Tx Complete
//ISR(USART0_TX_vect){
//	
//}