/**
 * Implements a digital thermometer with AVR ATmega8, 3 digit segment display and LM35 temperature sensor
 *
 */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


void display_number(uint16_t num, uint8_t g_dot_pos);
void avr_init(void);
uint16_t adc_samp(uint8_t ch);
void num_to_digits(uint16_t num, uint8_t *digits_buf);

/* Port and Pin definitions for Digit segments */
#define SEG_DDR		DDRB
#define SEG_PORT	PORTB
#define SEG_A		(1 << PB6)
#define SEG_B		(1 << PB7)
#define SEG_C		(1 << PB5)
#define SEG_D		(1 << PB4)
#define SEG_E		(1 << PB2)
#define SEG_F		(1 << PB0)
#define SEG_G		(1 << PB1)
#define SEG_H		(1 << PB3)

/* Port and pin definitions for Digit select pins */
#define SEL_DDR		DDRD
#define SEL_PORT	PORTD
#define SEL_0		(1 << PD7)
#define SEL_1		(1 << PD6)
#define SEL_2		(1 << PD5)

/* Segment definitions for digits */
enum digits_enum {
	DIGIT_0	= (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F),
	DIGIT_1	= (SEG_E|SEG_F),
	DIGIT_2 = (SEG_A|SEG_C|SEG_D|SEG_F|SEG_G),
	DIGIT_3	= (SEG_A|SEG_D|SEG_E|SEG_F|SEG_G),
	DIGIT_4 = (SEG_B|SEG_G|SEG_F|SEG_E),
	DIGIT_5 = (SEG_A|SEG_B|SEG_G|SEG_E|SEG_D),
	DIGIT_6 = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_G),
	DIGIT_7 = (SEG_A|SEG_F|SEG_E),
	DIGIT_8 = (SEG_A|SEG_B|SEG_C|SEG_D|SEG_E|SEG_F|SEG_G),
	DIGIT_9 = (SEG_A|SEG_B|SEG_D|SEG_G|SEG_F|SEG_E),
	DIGIT_NULL = 0
};

/* Segment patterns for all digits  */
static uint8_t g_digits_arr[] = { 
					DIGIT_0, 
					DIGIT_1, 
					DIGIT_2, 
					DIGIT_3, 
					DIGIT_4, 
					DIGIT_5, 
					DIGIT_6, 
					DIGIT_7, 
					DIGIT_8, 
					DIGIT_9
				};

static uint8_t g_num_digits[3]; /* Individual segment pattern of 3 digit number*/
static uint8_t g_dot_pos;
volatile static uint16_t adc_val;

/* Main code */
int main(void)
{
	uint16_t degree;

	/* Initializations */
	avr_init();
	
	/* Enable interrupts globally */
	sei(); 

	while(1) {

		adc_val = adc_samp(0);
		/* Code to degree celcius: Voltage = (2.56V/1024) * CODE; 
		   Celcius = Voltage/10mV = (256/1024*)CODE => CODE/4 (resolution = 1LSB = 0.25 celc)*/
		degree = adc_val>>2;
		
		/* Allow a hysteresis of 1LSB from X.75 cel to X.75+0.25 celc ; This allows smooth transition from X to X+1 in diplay */
		if((adc_val - (degree<<2)) != 3) { 
			num_to_digits(degree, g_num_digits);
		}

		_delay_ms(500);
	}
}

/* Initialization for AVR chip */
void avr_init(void)
{
	/* Segment port is output */
	SEG_DDR = 0xFF;
	SEG_PORT = 0xFF; // all segments OFF
	/* Select pins are output */
	SEL_DDR |= (SEL_0|SEL_1|SEL_2);
	SEL_PORT &= ~(SEL_0|SEL_1|SEL_2); // No digit selected
	
	/* Start Timer0 with presclar = 8 (1MHz clock gives overflow interrupts every 2 ms) */
	TCCR0 = 0x2;
	TIMSK |= (1 << TOIE0); // Enable overflow interrupt for TImer0 */
	
	/* Enable ADC with internal reference voltage; prescalar: 8 => 1MHz/8=128kHz */
	ADCSRA = (1 << ADEN)|0x3;
}


/* Do one ADC sample at specified channel, return 10-bit result */
uint16_t adc_samp(uint8_t ch)
{
	/* Reference is internal 2.56V */
	ADMUX = (3 << 6)|(ch & 0x7);
	
	/* Start a conversion */
	ADCSRA |= (1 << ADSC);
	while( !(ADCSRA & (1 << ADIF)) )
		;
	ADCSRA |= (1 << ADIF); /* Clear flag */
	
	/* Read results */
	return ADC;
}

/* Converts a number from to digit pattern
 *		num : number to be converted
 *      digits_buf : array to store digit pattern for each digit of the number
 *	NOTE: Current implementation is limited to 3 digit numbers
 */
void num_to_digits(uint16_t num, uint8_t *digits_buf)
{
	uint8_t digit;
	uint8_t lead_zero = 0;
	
	digit = num/100;
	if(!digit) {
		digits_buf[2] = DIGIT_NULL; /* Don't display leading zeros */
		lead_zero = 1;
	}
	else {
		digits_buf[2] = g_digits_arr[digit];
	}
	num -= (digit*100);
	digit = num/10;
	if(!digit && lead_zero) {
		digits_buf[1] = DIGIT_NULL; /* Don't display leading zeros */
	}
	else {
		digits_buf[1] = g_digits_arr[digit];
	}
	num -= (digit*10);
	digit = num;
	digits_buf[0] = g_digits_arr[digit];
}

		
/* ISR for TImer0 overflow interrupt (should come here every 2 ms) */
ISR(TIMER0_OVF_vect)
{
	static uint8_t pos;
	
	if(pos == 0) {
		SEL_PORT &= ~(SEL_1|SEL_2);
		SEL_PORT |= SEL_0;
	}
	else if(pos == 1) {
		SEL_PORT &= ~(SEL_0|SEL_2);
		SEL_PORT |= SEL_1;
	}
	else {
		SEL_PORT &= ~(SEL_0|SEL_1);
		SEL_PORT |= SEL_2;
	}
	SEG_PORT = (uint8_t)~g_num_digits[pos]; /* Common anode display (LOW bit = ON segment)*/
	if(g_dot_pos == pos) {
		SEG_PORT |= SEG_H;
	}
	
	pos++;
	if(pos > 2) {
		pos = 0;
	}
}


