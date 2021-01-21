/*****************************************************
This program was produced by the
CodeWizardAVR V2.03.4 Standard
Automatic Program Generator
Â© Copyright 1998-2008 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project :
Version :
Date    : 5/20/2010
Author  :
Company :
Comments:


Chip type           : ATmega32L
Program type        : Application
Clock frequency     : 12.000000 MHz
Memory model        : Small
External RAM size   : 0
Data Stack size     : 512
*****************************************************/

#include <avr/io.h>
#include <avr/interrupt.h>


#include "uart.h"

int main(void) {
	uint8_t c;
	uint8_t idx = 0;
	char str[RX_BUFFER_SIZE];

	DDRC  = (1<<PC2); // Alle GPIOs als Ausgänge
	PORTC = (1<<PC2); // Pin 1 an Port C auf HI

	uart_init(); // 9 bit mode is not supported

	sei(); //Interrput muss eingeschaltet werden + header eingebunden!

	uart_puts("AVR Ready\n\0", 10, '\0');

	while(1) {

		if( uart_error.active ) {
			if ( uart_error.frame ) {
				//This bit is set if the next character in the receive buffer had a frame error when received. I.e., when the first stop bit of the
				//next character in the receive buffer is zero. This bit is valid until the receive buffer (UDRn) is read. The FEn bit is zero when
				//the stop bit of received data is one. Always set this bit to zero when writing to UCSRnA.
			}
			if ( uart_error.overrun ) {
				//This bit is set if a data overrun condition is detected. A data overrun occurs when the receive buffer is full (two characters), it
				//is a new character waiting in the receive shift register, and a new start bit is detected. This bit is valid until the receive buffer
				//(UDRn) is read. Always set this bit to zero when writing to UCSRnA.
			}
			if ( uart_error.parity ) {
				// This bit is set if the next character in the receive buffer had a parity error when received and the parity checking was enabled
				//at that point (UPMn1 = 1). This bit is valid until the receive buffer (UDRn) is read. Always set this bit to zero when writing to
				//UCSRnA.
			}
			if ( uart_error.rx_overrun ) {}
			if ( uart_error.tx_overrun ) {}

			uart_error_reset();
		}

		// auslesen einzelner Zeichen
		//if ( uart_getc(&c) ) {
		//	uart_putc(c);
		//}

		// Auslesen einer Zeichenkette
		if ( uart_gets(str, sizeof(str), ';') ) {
			uart_puts(str, sizeof(str), ';');
		}

		uart_transmit_buffer();






	}

	return 0;
}
