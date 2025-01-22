/*
 * usart.c
 *
 * Created: 12/30/2016 11:21:41 PM
 *  Author: jcaf
 */
#include "../system.h"
#include "usart.h"
#include "../serial/serial.h"

void USART_Init( unsigned int ubrr)
{
    /*Set baud rate */
    UBRRH = (unsigned char)(ubrr>>8);
    UBRRL = (unsigned char)ubrr;
    /*Enable receiver and transmitter */
    UCSRB = (0<<RXEN)|(1<<TXEN) | (0<<RXCIE);//Enable interrupt;
    /* Set frame format: 8data, 1stop bit */
    //UCSRC = (0<<USBS)|(3<<UCSZ0);
    //UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);
    UCSRB = (1<<RXEN)|(1<<TXEN);
    /* Set frame format: 8data, 2stop bit */
    UCSRC = (1<<URSEL)|(0<<USBS)|(3<<UCSZ0);

    //
    //UCSRA = 1<<U2X;
}
void USART_Transmit( unsigned char data )
{
    /* Wait for empty transmit buffer */
    while ( !( UCSRA & (1<<UDRE)) )
        ;
    /* Put data into buffer, sends the data */
    UDR = data;
}
unsigned char USART_Receive( void )
{
    /* Wait for data to be received */
    while ( !(UCSRA & (1<<RXC)) )
        ;
    /* Get and return received data from buffer */
    return UDR;
}
//void USART_Flush( void )
//{
//    unsigned char dummy;
//    while ( UCSR0A & (1<<RXC0) ) dummy = UDR0;
//}

ISR(USART_RX_vect)
{
    //uint8_t _udr_rx = UDR0;
    //USART_Transmit(_udr_rx);
    //uint8_t u = UDR;
    rx_handler();
}
//////////////////////////////////////////////

void usart_print_string(const char *p)
{
    while (*p)
    {
        USART_Transmit(*p);
        p++;
    }
}

void usart_println_string(const char *p)
{
    usart_print_string(p);
    USART_Transmit('\n');
}

#if defined(__GNUC__) && defined(__AVR__)
    #include <avr/pgmspace.h>

    void usart_print_PSTRstring(const char *p)
    {
        char c;

        while (1)
        {
            c = pgm_read_byte_near(p);
            if (c == '\0')
                break;
            else
                USART_Transmit(c);
            p++;
        }
    }

    void usart_println_PSTRstring(const char *p)
    {
        usart_print_PSTRstring(p);
        USART_Transmit('\n');
    }

#endif

