#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Define baud rate
#define USART_BAUDRATE 9600   
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
//#define BAUD_PRESCALE ((F_CPU / (16 * USART_BAUDRATE)) - 1)
//#define BAUD_PRESCALE 51

volatile char receivedBuffer;

ISR(USART_RX_vect){
    receivedBuffer = UDR0;
    WATCHDOG_RESET
}

void USART_Init(void){
   // Set baud rate
   UBRR0H = (BAUD_PRESCALE >> 8); // Load upper 8-bits into the high byte of the UBRR register
   UBRR0L = BAUD_PRESCALE; // Load lower 8-bits into the low byte of the UBRR register
   // Default frame format is 8 data bits, no parity, 1 stop bit
 // to change use UCSRC 

 //   UBRR0H = 0;
 //   UBRR0L = 207;

/*
   UCSR0B = (0 << UCSZ02);
    UCSR0C = 6;//((1 << UCSZ01) | (1 << UCSZ00));
*/

    // Configure port
    DDRD = 0;
    PORTD = 0;
    DDRD = 2;

    // Enable receiver and transmitter and receive complete interrupt 
    UCSR0B = ((1 << TXEN0) | (1 << RXEN0) | (1 << RXCIE0));
}


void USART_SendByte(uint8_t u8Data){

    // Wait until last byte has been transmitted
    while((UCSR0A &(1<<UDRE0)) == 0);

    // Transmit data
    UDR0 = u8Data;
}

void USART_SendString(uint8_t * str){
    while(*str) {
        USART_SendByte(*str++);
    }
}

uint8_t USART_ReceiveByte(){
    while((UCSR0A &(1<<RXC0)) == 0);
    return UDR0;
}
