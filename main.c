/*
 * Program do sterowania zdalnego samochodzikiem wyposazonego w 3 czujniki analogowe
 * Szczegołowe załozenia konstrukcyjne znajdują sie w pliku Załozenia_Konstrukcyjne.txt
 * 
 * Samochodzik jest sterowany zdalnie przez bluetooth, przez cyfrowe urządzenie streujące
 * urządzenie sterujące przetwarza dane z czujnikow, samochodzik je tylko wysyła
 * 
 * Samochodzik moze mieć ustawiony poziom szybkości, jest to ustawiane przy starcie samochodzika
 * pozwala to osobom ostrozniejszym zachować większą kontrolę pojazdu
 * 
 * Samochodzik skręca podobnie do czołgow
 */



#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "uart.h"
#include "init.h"

#define F_CPU 7372800UL
#define __AVR_ATmega88P__
#define UART_BAUD_RATE      38400

volatile signed int adc=0;
volatile unsigned char adc1, adc2;
volatile uint8_t MUX_tmp=0;


/*Przerwanie wywołane końcem konwersji ADC*/
ISR(ADC_vect)
{
	adc=ADC;
}

/*
 * Przerwanie wywołane zdarzeniem timera
 * Słuzy wysyłaniu co pewien czas do
 * urządzenia sterującego sygnału
 * z czujnikow, przy czym wysyłana
 * informacja pochodzi z jednego czujnika
 * na jedno zdarzenie timera
 */
ISR(TIMER1_CAPT_vect)
{
	adc=ADC;

	MUX_tmp=ADMUX & 0xF;
	
	if(MUX_tmp <= 3)
	{
		ADMUX=(MUX_tmp+1) | _BV(REFS0);
		MUX_tmp= ADMUX & 0xF;
	}
	else
	{
		ADMUX=_BV(REFS0);
	}
	PORTB=0;
	adc1=(unsigned char) ((adc>>8) & 0xFF);
	adc2=(unsigned char) (adc & 0xFF);
	if(adc>=300)
	{
		uart_putc(adc1);
		uart_putc(adc2);
	}
}

int main(void)
{
	unsigned int uart1=0; 
	unsigned int k;
	unsigned int uart2;
	char ADCtmp1, ADCtmp2;
	double ocr0a=0;
	double ocr0b=0;

	/*Inicjacja procesora oraz rejestru ADC*/
	uC_init();
	ADC_init();
						
	/*Odblokowanie przerwań*/
	sei();

	/*Oczekiwanie na ustawienie wyjścia led modułu HC05 (podłączonego do PB0)*/
	uint8_t PORTBtmp=0;
	while(PORTBtmp==0) {PORTBtmp=PINB & 1;} 

	/*
	 * Wysłanie do urządzenia sterującego informacji o gotowości
	 * do odebrania informacji o podstawowej szybkości
	 */
	const char ready=254;		
	uart_putc((const unsigned char)ready);

	/*Ustawienie szybkości pojazdu*/
	while((uart1==0) || (uart1 & UART_NO_DATA) || (uart1 & UART_FRAME_ERROR) || (uart1 & UART_OVERRUN_ERROR) || (uart1 & UART_BUFFER_OVERFLOW) )
    {
		uart1 = uart_getc();

		if ( uart1 & UART_NO_DATA )	{}
		else
		{
			if ( uart1 & UART_FRAME_ERROR )
			{
				uart_puts_P("UART Frame Error: ");

			}
			if ( uart1 & UART_OVERRUN_ERROR )
			{
				uart_puts_P("UART Overrun Error: ");
			}
			if ( uart1 & UART_BUFFER_OVERFLOW )
			{
				uart_puts_P("Buffer overflow error: ");
			}

       }
    }

	char bonus = uart1 & 0xFF;
	int pwm= 50 + bonus;
	double ocr= (double) pwm / 100;
	unsigned int mr = 0;
	unsigned int ml = 0;

	/*
	 * Ustawienie timera, dopiero teraz zaczynamy
	 * wysyłanie informacji z czujnikow
	 */
	Timer_init();
	
	/*Petla głowna*/
	while(1)
	{

		uart1=0;
		while((uart1==0) || (uart1 & UART_NO_DATA) || (uart1 & UART_FRAME_ERROR) || (uart1 & UART_OVERRUN_ERROR) || (uart1 & UART_BUFFER_OVERFLOW) )
		{
			uart1 = uart_getc();

			if ( uart1 & UART_NO_DATA )	{}
			else if ( uart1 & UART_FRAME_ERROR )
			{
				uart_puts_P("UART Frame Error: ");
			}
			else if ( uart1 & UART_OVERRUN_ERROR )
			{
				uart_puts_P("UART Overrun Error: ");
			}
			else if ( uart1 & UART_BUFFER_OVERFLOW )
			{
				uart_puts_P("Buffer overflow error: ");
			}
			else
			{
				uart2=0;
				/* 
					* Wysłanie potwierdzenia gotowości do odbioru
					* drugiej częsci informacji
					*/
				uart_putc((const unsigned char)ready); 
				uart2 = uart_getc();
				while((uart2 & UART_NO_DATA) || (uart2 & UART_FRAME_ERROR) || (uart2 & UART_OVERRUN_ERROR) || (uart2 & UART_BUFFER_OVERFLOW))
				{
					PORTB=1;
					uart2= uart_getc();  
				}
			}
		}
		/* uart2 jest wartoscia mr/ml, uart1 okresla kierunki etc*/

		/* najmlodszy bit starszego bajtu mowi nam o tym ktory silnik ustawiamy, dla 1 prawy*/
		if(uart1 & 0x01) 
		{
			mr= uart2 & 0xFF;
		}
		else
		{
			ml = uart2 & 0xFF;
		}

		/* drugi bit starszego bajtu mowi o kierunku, 1 to do przodu*/
		if(uart1 & 0x02) 
		{
			if(uart1 & 0x01)
			{
				/*ustaw kierunek prawego silnika w przód*/
				PORTB|= _BV(PB1);
				PORTB&= ~(_BV(PB2));
			}
			else
			{
				/*ustaw kierunek lewego silnika do przodu*/
				PORTB|= _BV(PB2);
				PORTB&= ~(_BV(PB1));
			}
		}
		else
		{
			if(uart1 & 0x01)
			{
				/*ustaw kierunek prawego silnika w tył*/
				PORTB|= _BV(PB3);
				PORTB&= ~(_BV(PB4));
			}
			else
			{
				/*ustaw kierunek lewego silnika do tyłu*/
				PORTB|= _BV(PB4);
				PORTB&= ~(_BV(PB3));
			}
		}

		/*Ustawienie mocy silnikow*/
		ocr0a= (double) ocr*mr;
		ocr0b= (double) ocr*ml;
		OCR0A= (int) ocr0a;
		OCR0B= (int) ocr0b;
	}
} 
