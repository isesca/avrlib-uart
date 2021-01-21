#ifndef UART_H_
#define UART_H_

#ifndef F_CPU
	//#warning "F_CPU war noch nicht definiert. Dies wird nun nachgeholt"
	#define F_CPU 16000000UL
#endif

#define F_CPU 16000000UL

#define UART_BAUD 9600UL

#ifdef __AVR_ATmega328P__

	#define UART_DATA_5BITS    ((0 << UCSZ01) | (0 << UCSZ00))
	#define UART_DATA_6BITS    ((0 << UCSZ01) | (1 << UCSZ00))
	#define UART_DATA_7BITS    ((1 << UCSZ01) | (0 << UCSZ00))
	#define UART_DATA_8BITS    ((1 << UCSZ01) | (1 << UCSZ00))

	#define UART_PARITY_NONE    ((0 << UPM01) | (0 << UPM00))
	#define UART_PARITY_ODD     ((1 << UPM01) | (1 << UPM00))
	#define UART_PARITY_EVEN    ((1 << UPM01) | (0 << UPM00))

	#define UART_STOP_1BIT      (0 << USBS0)
	#define UART_STOP_2BIT      (1 << USBS0)

#endif

#define RX_BUFFER_SIZE    16 // max. 256 (8, 16, 32, 64)
#define TX_BUFFER_SIZE    16

/* UART bits status and control register UCSR0C */
#define UART_FRAME_FORMAT    UART_DATA_8BITS | UART_PARITY_NONE | UART_STOP_1BIT;


static struct uart_error_state_t {
	uint8_t active;
	uint8_t frame;
	uint8_t overrun;
	uint8_t parity;
	uint8_t rx_overrun;
	uint8_t tx_overrun;
} uart_error = {0, 0, 0, 0, 0, 0};


/*************************************************************** PROTOTYPES ***/

void uart_init(void);

uint8_t uart_getc(uint8_t *pByte);

uint8_t uart_gets(uint8_t *buffer, uint8_t len, uint8_t eol);

void uart_putc(uint8_t byte);

void uart_puts(uint8_t *s, uint8_t len, uint8_t eol);

void uart_transmit_buffer(void);

void uart_error_reset(void);


#endif /* UART_H_ */
