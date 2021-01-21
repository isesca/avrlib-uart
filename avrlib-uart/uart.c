#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "uart.h"


/************************************************************** DEFINITIONS ***/

// Berechnung
#define UBRR_VAL (( F_CPU + UART_BAUD * 8) / (UART_BAUD * 16) - 1 )

// Reale Baudrate
#define BAUD_REAL ( F_CPU / ( 16 * ( UBRR_VAL + 1 )))

// Fehler in Promille, 1000 = kein Fehler
#define BAUD_ERROR (( BAUD_REAL * 1000) / UART_BAUD )

#if (( BAUD_ERROR < 990 ) || ( BAUD_ERROR > 1010 ))
	#error Systematischer Fehler der Baudrate grösser 1% und damit zu hoch!
#endif


#ifdef __AVR_ATmega328P__
	/* UART register aliases */
	#define UART_BAUDRATE_REG        UBRR0
	#define UART_DATA_REG            UDR0
	#define UART_STATE_REG           UCSR0A // Status
	#define UART_CONTROL_REG         UCSR0B // Einstellung
	#define UART_RUNMODE_REG         UCSR0C // Configuration

	/* UART bits status and control register UCSR0A */
	#define UART_RX_COMPLETE         RXC0
	#define UART_TX_COMPLETE	     TXC0
	#define UART_DATA_REG_EMPTY      UDRE0
	#define UART_FRAME_ERROR         FE0
	#define UART_OVERRUN_ERROR       DOR0
	#define UART_PARITY_ERROR        UPE0

	/* UART bits status and control register UCSR0B */
	#define UART_RX_ENABLE           RXEN0
	#define UART_TX_ENABLE           TXEN0
	#define UART_RX_INTERRUPT_ENABLE RXCIE0
	#define UART_TX_INTERRUPT_ENABLE TXCIE0

#endif

#define RX_BUFFER_MASK    (RX_BUFFER_SIZE) - 1
#define TX_BUFFER_MASK    (TX_BUFFER_SIZE) - 1
#define BUFFER_FAIL	      0
#define BUFFER_SUCCESS    1

struct buffer_t {
	volatile uint8_t *data;
	volatile uint8_t write;
	volatile uint8_t read;
	const 	 uint8_t mask;
};



/****************************************************************** GLOBALS ***/

volatile uint8_t rx_buffer[RX_BUFFER_SIZE] = {};
volatile uint8_t tx_buffer[TX_BUFFER_SIZE] = {};

struct buffer_t receive_buffer  = {rx_buffer, 0, 0, RX_BUFFER_MASK };
struct buffer_t transmit_buffer = {tx_buffer, 0, 0, TX_BUFFER_MASK };


/*************************************************************** PROTOTYPES ***/
uint8_t buffer_in(struct buffer_t *buffer, uint8_t byte);
uint8_t buffer_out(struct buffer_t *buffer, uint8_t *pByte);
void uart_putc_blocking(uint8_t c);

/********************************************************************** ISR ***/
ISR(USART_RX_vect) {
	uint8_t next;

	uart_error.overrun = UART_STATE_REG & ( 1 << UART_OVERRUN_ERROR );
	uart_error.frame   = UART_STATE_REG & ( 1 << UART_FRAME_ERROR );

	next = UART_DATA_REG;

	uart_error.parity  = UART_STATE_REG & ( 1 << UART_PARITY_ERROR );

	if ( uart_error.overrun | uart_error.frame | uart_error.parity ) {
		uart_error.active = 1;
	}

	buffer_in(&receive_buffer, next);
}


/********************************************************* PUBLIC FUNCTIONS ***/
void uart_init(void) {
	UART_BAUDRATE_REG = UBRR_VAL;

	// UART TX einschalten
	UART_CONTROL_REG |= ( 1 << UART_TX_ENABLE ); // Transmit enable
	UART_CONTROL_REG |= ( 1 << UART_RX_ENABLE ); // Receive enable
	UART_CONTROL_REG |= ( 1 << UART_RX_INTERRUPT_ENABLE ); // Receive complete interrput enable einschalten

	// Frame Format
	UART_RUNMODE_REG = UART_FRAME_FORMAT;
}

uint8_t uart_getc(uint8_t *pByte) {
	return buffer_out(&receive_buffer, pByte);
}

uint8_t uart_gets(uint8_t *buffer, uint8_t len, uint8_t eol) {
	uint8_t c, i;
	static uint8_t idx;

	if ( uart_getc(&c) ) {
		buffer[idx] = c;
		idx++;

		// Buffer vor dem Überlauf bewahren
		if (idx > (len - 1) ) {
			idx = 0;
			// TODO: Buffer overflow.
			return 0;
		}

		// Zeilenende erkannt. Die Message zurücksenden.
		if (c == eol) {

			for (i = (idx+1); i <= (len-1); i++)
				buffer[idx] = 0;

			idx = 0;
			return 1;
		}
	}

	return 0;
}

void uart_putc(uint8_t byte) {
	buffer_in(&transmit_buffer, byte);
}

void uart_puts(uint8_t *s, uint8_t len, uint8_t eol) {
	uint8_t byte, i;

	for (i = 0; i < len; i++) {
		byte = s[i];

		if ( byte == eol )
			return;

		buffer_in(&transmit_buffer, byte);
	}
}

void uart_transmit_buffer(void) {
	uint8_t c;

	if ( buffer_out(&transmit_buffer, &c) ) {
		uart_putc_blocking(c); // Senden muss blokierend zu einem passenden Zeitpunkt erfolgen.
	}
}

void uart_error_reset(void) {
	uart_error.active     = 0;
	uart_error.frame      = 0;
	uart_error.overrun    = 0;
	uart_error.parity     = 0;
	uart_error.rx_overrun = 0;
	uart_error.tx_overrun = 0;
}


/******************************************************** PRIVATE FUNCTIONS ***/
uint8_t buffer_in(struct buffer_t *buffer, uint8_t byte) {

	// Determine the index of the current data field
	uint8_t index = ( (buffer->write + 1) & buffer->mask );

	// Check if there is space for the current data field
	if ( buffer->read == index ) {
		return BUFFER_FAIL;
		// Buffer Overflow
		//TODO: Error Handling
	}

	// Write data to buffer. (The mask avoids errors when the buffer is not initialized correctly)
	buffer->data[buffer->write & buffer->mask] = byte;

	// Write complete => increase index
	buffer->write = index;

	return BUFFER_SUCCESS;
}

uint8_t buffer_out(struct buffer_t *buffer, uint8_t *pByte) {

	// No data available.
	if ( buffer->read == buffer->write ) {
		return BUFFER_FAIL;
		// TODO: Error Handling
	}

	// Save value from buffer to output variable
	*pByte = buffer->data[buffer->read];

	// read action successful => increase index.
	buffer->read = (buffer->read + 1) & buffer->mask;

	return BUFFER_SUCCESS;
}

void uart_putc_blocking(uint8_t c) {

	while ( !(UART_STATE_REG & ( 1 << UART_DATA_REG_EMPTY ) ) ) {
		/* Warten bis UART Modul bereit zum senden. */
	}

	UART_DATA_REG = c;
}
















