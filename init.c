/*
 * init.c
 *
 *  Created on: 2 lis 2019
 *      Author: mateuszbystronski
 */
#include <avr/io.h>
#include <avr/interrupt.h>

void uC_init(void)
{
	DDRB|= 0b00111110; /* PB0- wyjście led z hc05; PB1,2,3,4- sterowanie kierunkiem mostku;  PB5-*/
	DDRD|= 0b01100000;  /* PD5-6- silniki*/
	PORTB= 0;
}

void ADC_init(void)
{
	ADMUX|= ( _BV(REFS0) );		/*napięcie referencyjne z pinu AVCC*/
	ADCSRA|= ( _BV(ADEN) | _BV(ADIE) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0));  /*włącz ADC i przerwania ADC, preskaler 128*/
	DIDR0 |= 0x3F; /*wyłącz cyfrowe działanie pinów*/
	ADCSRA&= ~(_BV(ADIE));	/*Tryb ciągłej konwersji*/
}

void Timer_init(void)
{
	TCCR0A|= (_BV(COM0A1) | _BV(COM0B1) | _BV(WGM00) | _BV(WGM01));  /*Fast PVM, wyzeruj piny OC0A i OC0B przy zdarzeniu comapre match*/
	TCCR0B|= (_BV(CS00)); /*Bez preskalera*/
	TCCR1B|= (_BV(WGM13) | _BV(WGM12) | _BV(CS12) | _BV(CS10)); /*Tryb CTC (ICR), preskaler 1024*/
	OCR1AH|= 0x10;
	ICR1H = 0x00;
	ICR1L = 0x80 & 0xFF;
	TIMSK1|= _BV(ICIE1); /*PRzerwanie output compare match A*/
	GTCCR=_BV(PSRSYNC);  /*wyzeruj preskaler*/
}

