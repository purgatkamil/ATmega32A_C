#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include "HD44780.h"

#include "USART.h"
#include "SPI.h"

#include "TWI.c"

int temperaturka = 0;

unsigned int W = 0;
unsigned int adc = 0;

char str[20];
char voltage_t[20];
char temperature[10];

int Godziny = 0;
int Minuty = 0;
int Sekundy = 0;

unsigned int volt = 0;

int B1, B2, B3;

int previous = 0;
int mode = 1;

void adc_read()
{
	W = ADCL;
	W |= (ADCH << 8);
	adc = (W * 5 * 10) / 1024;
}

void SPI_Init(void)
{
	DDRB = (1<<PB7)|(1<<PB4);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void ADC_Init(void)
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE) ;
}

void AutoTrigger_Init(void)
{
	ADCSRA |= (1 << ADATE);
	ADCSRA |= (1 << ADSC);
}

char SPI_Transmit(char cData)
{
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

ISR(ADC_vect)
{
	adc_read();
}

void setTime(int Hours, int Minutes, int Seconds)
{
	int jednosciHours = Hours % 10;
	int dziesiatkiHours =  (Hours - jednosciHours) / 10;
	int wynikHours = 0;
	dziesiatkiHours = dziesiatkiHours << 4;
	wynikHours = dziesiatkiHours + jednosciHours;
	
	int jednosciMinutes = Minutes % 10;
	int dziesiatkiMinutes =  (Minutes - jednosciMinutes) / 10;
	int wynikMinutes = 0;
	dziesiatkiMinutes = dziesiatkiMinutes << 4;
	wynikMinutes = dziesiatkiMinutes + jednosciMinutes;
	
	int jednosciSeconds = Seconds % 10;
	int dziesiatkiSeconds =  (Seconds - jednosciSeconds) / 10;
	int wynikSeconds = 0;
	dziesiatkiSeconds = dziesiatkiSeconds << 4;
	wynikSeconds = dziesiatkiSeconds + jednosciSeconds;
	
	TWI_Start();                // start transmission
	TWI_Write(0xD0);            // write addres of M41T00
	TWI_Write(0x00);            // select seconds register
	TWI_Write(wynikSeconds);	// read hours register
	TWI_Write(wynikMinutes);	// read hours register
	TWI_Write(wynikHours);		// read hours register
	TWI_Stop();					// stop transmission 
}

void getTime(int *hr,int *min,int *sec)
{
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select seconds registers
	TWI_Start();                        // repeated start
	TWI_Write(0xD1);                    // write address for reading data
	*sec = TWI_Read(ACK);               // read seconds register                   
	*min = TWI_Read(ACK);               // read minutes register                 
	*hr= TWI_Read(NACK);                // read hours register
	TWI_Stop();                         // stop transmission	
}

enum TemperatureScales{Celsius = 0, Fahrenheit = 1};

int getTemp(int t)
{
	int A1,A2,A3;
	
	PORTB &= ~(1 << PB4); // Enable CS
	A1 = SPI_Transmit(0); // read byte
	A2 = SPI_Transmit(0); // read byte
	PORTB |= (1 << PB4);  // Disable CS
	
	A1 = A1 * 256;
	A2 = A2 - 7;
	A1 = (A1 + A2);
	A1 = A1 >> 3;
	A3 = A1;
	A3 *= 0.625;
	
	if(t == 0)
	{
		return A3;
	}
	else if(t == 1) 
	{
		A3 = ((A3 * 9) / 5 ) + 32;
		return A3;
	}
	else
	{
		A3 = 0;
		return A3;
	}
}

void displayData(int mode, int x, int y)
{
	switch (mode)
	{
	case 1:
	LCD_GoTo(x,y);
	LCD_WriteText("Voltage: ");
	
	if (adc >= 10)
	{
	itoa(adc, voltage_t, 10);
	LCD_GoTo(x+9,y);
	LCD_WriteData(voltage_t[0]);
	LCD_GoTo(x+10,y);
	LCD_WriteText(".");
	LCD_GoTo(x+11,y);
	LCD_WriteData(voltage_t[1]);
	}
	else
	{
	itoa(adc, voltage_t, 10);
	LCD_GoTo(x+9,y);
	LCD_WriteText("0");
	LCD_GoTo(x+10,y);
	LCD_WriteText(".");
	LCD_GoTo(x+11,y);
	LCD_WriteData(voltage_t[0]);
	}
	break;
	
	case 2:
	
	LCD_GoTo(x, y);
	LCD_WriteText("Temp: ");
	
	itoa(temperaturka, temperature, 10);
	LCD_GoTo(x+7, y);
	LCD_WriteData(temperature[0]);
	LCD_GoTo(x+8, y);
	LCD_WriteData(temperature[1]);
	LCD_GoTo(x+9, y);
	LCD_WriteText(".");
	LCD_GoTo(x+10, y);
	LCD_WriteData(temperature[2]);
	break;
	
	case 3:
	
	B2 = ((Godziny>>4)&7);     // convert seconds to ASCII char
	B3 = (Godziny&15);         // convert seconds to ASCII char
	itoa(B2, str, 10);
	LCD_GoTo(x, y);
	LCD_WriteData(str[0]);
	itoa(B3, str, 20);
	LCD_GoTo(x+1, y);
	LCD_WriteData(str[0]);

	LCD_GoTo(x+2,y);
	LCD_WriteText(":");

	B2 = ((Minuty>>4)&7);     // convert seconds to ASCII char
	B3 = (Minuty&15);         // convert seconds to ASCII char
	itoa(B2, str, 20);
	LCD_GoTo(x+3, y);
	LCD_WriteData(str[0]);
	itoa(B3, str, 20);
	LCD_GoTo(x+4, y);
	LCD_WriteData(str[0]);
	
	LCD_GoTo(x+5, y);
	LCD_WriteText(":");

	B2 = ((Sekundy>>4)&7);     // convert seconds to ASCII char
	B3 = (Sekundy&15);         // convert seconds to ASCII char
	itoa(B2, str, 20);
	LCD_GoTo(x+6, y);
	LCD_WriteData(str[0]);
	itoa(B3, str, 20);
	LCD_GoTo(x+7, y);
	LCD_WriteData(str[0]);
	break;
	}
}

void checkBtn()
{
	if (!(PINB & 1))
	{
		if(previous == 0)
		{
			mode = mode + 1;
			LCD_WriteCommand(HD44780_CLEAR);
			previous = 1;
			if(mode >= 4)
			{
				mode = 1;
			}
		}
	}
	else
	{
		previous = 0;
	}
}

int main(void)
{

//DDRD |= (1 << PD1);

	LCD_Initialize();
	ADC_Init();
	AutoTrigger_Init();
	SPI_Init();
	TWI_Init();
	sei();
	setTime(16, 17, 18);

	while(1)
	{
		getTime(&Godziny, &Minuty, &Sekundy);
		temperaturka = getTemp(Celsius);
		checkBtn();
		displayData(mode, 0, 1);
		_delay_ms(100);
	}
	return 0;
}
