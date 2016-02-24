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
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

long test = 0;
//---Message receive and message sent---
//tx_msg contains data that you want to send
//---------USART0 message------------
uint8_t tx_msg0[22]={0xA0, 0xA1, 0x00, 0x0F, 0x01, 0x04, 0x07, 0xDD, 0x09, 0x1B, 0x06, 0x29, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x0D, 0x0A};
//---------USART1 message------------
uint8_t tx_msg1[22]={0xA0, 0xA1, 0x00, 0x0F, 0x01, 0x04, 0x07, 0xDD, 0x09, 0x1B, 0x06, 0x29, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x0D, 0x0A};
//---------Counters USART0-----------
volatile uint8_t tx_count0 = 0;
volatile uint8_t rx_count0 = 0;
//---------Counters USART1-----------
volatile uint8_t tx_count1 = 0;
volatile uint8_t rx_count1 = 0;
int data0[22] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
int data1[22] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
//--------Decleration of functions-------
void init();	
	
int main(void)
{
	
	DDRB = 0xff;
	init();
    while (1) 
    {
		asm("nop");
		test = test +1;
		//for (int i=0; i<22; i++) {
		//	UDR0=tx_msg0[i];
		//}
		asm("nop");
    }
}

//USART0 Rx Complete interrupt. Will only activated if RXC0 flag in UCSR0A is set
ISR(USART0_RX_vect0){
	PORTB |= 0x02;
	data0[rx_count0]= UDR0;
	rx_count0 = rx_count0 + 1;

}

ISR(USART1_RX_vect){
	
	data1[rx_count1]= UDR1;
	rx_count1=rx_count1+1;
}

void init()
{
	//set GPS in reset mode (Reset is set to low)
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
		
	//Enable Recieve interrupt for USART0
	UCSR0B |= (1<<RXCIE0);
	//Enable Recieve interrupt for USART1
	UCSR1B |= (1<<RXCIE1);
	//Enable Global interrupt
	sei();
	//Activate GPS (Reset is set to high)
	PORTB |= 0x01;
	
}