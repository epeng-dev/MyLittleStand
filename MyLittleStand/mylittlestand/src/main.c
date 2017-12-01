#define F_CPU 16000000

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdio.h>
#include "light_ws2812.h"

#define BUF 20
/* Register */
#define DDR_LED DDRB
#define DDR_LCD DDRA
#define DDR_LCD_PREPARE DDRD
#define PORT_LED PORTB
#define PORT_LCD PORTA

#define MAXPIX 10
#define MAX_MODE 3

#define lcd_rs_on()   PORTD|=0x10
#define lcd_rw_on()   PORTD|=0x20
#define lcd_e_on()    PORTD|=0x40
#define lcd_rs_off()   PORTD&=~(0x10)
#define lcd_rw_off()  PORTD&=~(0x20)
#define lcd_e_off()    PORTD&=~(0x40)

void DDR_register_init();
void led_init();
void led_output();
void led_on();
void led_off();
void led_bright_up();
void led_bright_down();

char rx_char(void);
void rx_str(char buf[BUF]);
void USART0_init(unsigned int UBRR0);

void command(unsigned char CMD);
void lcd_char(unsigned char CMD);
void lcd_str(char* string);
void lcd_init();
void lcd_change();

struct cRGB white[MAXPIX];
unsigned char bright = 100;

char buf1[BUF];
char buf2[BUF];

char pairflag = 0;
int main(void)
{
	char flag;
	char data[BUF];
	DDR_register_init();
	USART0_init(103);
	led_init();
	lcd_init();
	led_output();
	lcd_str("My Little Stand");
	command(0xC0);
	sprintf(buf1, "Bright : %d", bright/50);
	lcd_str(buf1);
	while(1)
	{
		flag = rx_char();
		switch (flag)
		{
			case '1':
				pairflag = 1;
				led_off();
				lcd_change();
				break;
			case '2':
				if(bright != 0){
					led_bright_up();
					lcd_change();
				}
				break;
			case '3':
				if(bright != 0){
					led_bright_down();
					lcd_change();
				}
				break;
			case '4':
				led_on();
				lcd_change();
				break;
			case '5':
				led_off();
				lcd_change();
				break;
		}
	}
}

void DDR_register_init()
{
	DDR_LED |= 0x01;
	DDR_LCD = 0xFF;
	DDR_LCD_PREPARE = 0xF0;
	EICRA = 0xFF;
	EIMSK = 0x0F;
	sei();
}

void led_init()
{
	int i = 0;
	for(i = 0; i < MAXPIX; i++){
		white[i].r=bright; white[i].g=bright; white[i].b=bright;
	}
}

void led_on()
{
	bright = 100;
	led_init();
	led_output();
}

void led_off()
{
	bright = 0;
	led_init();
	led_output();
}

void led_bright_up()
{
	if(bright < 250)
	{
		bright += 50;
		led_init();
		led_output();
	}
}

void led_bright_down()
{
	if(bright > 90)
	{
		bright -= 50;
		led_init();
		led_output();
	}
}

void led_output()
{
	ws2812_sendarray(white, MAXPIX*3);
}

char rx_char(void) {
	while((UCSR0A&0x80) == 0);
	return UDR0;
}

void rx_str(char* buf) {
	int i = 0;

	while((buf[i] = rx_char()) != '\n'){
		i+=1;
	}
	buf[i] = '\0';
}

void USART0_init(unsigned int UBRR0){
	UBRR0H=(unsigned char)(UBRR0 >> 8);
	UBRR0L=(unsigned char)UBRR0;
	UCSR0B=(1<<RXEN0);
}

void command(unsigned char CMD){
	PORT_LCD = CMD;
	lcd_e_on();
	_delay_ms(3);
	lcd_e_off();
}

void lcd_char(unsigned char CMD){
	lcd_rs_on();
	lcd_rw_off();
	command(CMD);
	lcd_rs_off();
}

void lcd_str(char* string){
	int i = 0;
	while(string[i]!='\0')
	lcd_char(string[i++]);
}

void lcd_init()
{
	lcd_rs_off();
	lcd_rw_off();

	command(0x38);
	command(0x38);
	command(0x0C);
	command(0x06);
	command(0x01);
}

void lcd_change()
{	
	command(0x01);
	lcd_str("My Little Stand");
	command(0xC0);
	sprintf(buf1, "Bright : %d", bright/50);
	lcd_str(buf1);
	if(pairflag){
		lcd_str(" Paring");
	}
}

ISR(INT0_vect)
{
	led_on();
	lcd_change();
}

ISR(INT1_vect)
{
	led_off();
	lcd_change();
}

ISR(INT2_vect)
{
	if(bright != 0){
		led_bright_up();
		lcd_change();
	}
}

ISR(INT3_vect)
{
	if(bright != 0)
	{
		led_bright_down();
		lcd_change();
	}
}