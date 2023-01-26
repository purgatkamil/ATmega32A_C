#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "HD44780.h"
#include "USART.h"
#include "uart.h"
#include "SPI.h"
#include "TWI.c"
#include "rprintf.h"

int ReceivedTemperature = 0;
char TemperatureString[10];

unsigned int W = 0;
unsigned int adc = 0;
char VoltageString[20];

int B1, B2, B3;
int Hours = 0;
int Minutes = 0;
int Seconds = 0;
char Time[7];
char TimeString[20];

int PreviousState = 0;
int DisplayMode = 1;

int watchdog = 0;

char *ReceivedData;

void SPI_Init(void){
	DDRB = (1<<PB7)|(1<<PB4);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void ADC_Init(void){
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE);
	ADCSRA |= (1 << ADATE) | (1 << ADSC); //Auto-trigger
}

void Timer1_Init(){
	TCCR1B |= (1 <<CS12) | (1 << WGM12) | (1<<CS10);
	TIMSK |= (1 << OCIE1A);
	OCR1A = 1563;
}

void USART_Int_Init(){
	UCSRB |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}

char SPI_Transmit(char cData){
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

void adc_read(){
	W = ADCL;
	W |= (ADCH << 8);
	adc = (W * 5 * 10) / 1024;
}

void setTime(int Hours, int Minutes, int Seconds){
	int OnesHours = Hours % 10;
	int TensHours =  (Hours - OnesHours) / 10;
	int ResultHours = 0;
	TensHours = TensHours << 4;
	ResultHours = TensHours + OnesHours;
	
	int OnesMinutes = Minutes % 10;
	int TensMinutes =  (Minutes - OnesMinutes) / 10;
	int ResultMinutes = 0;
	TensMinutes = TensMinutes << 4;
	ResultMinutes = TensMinutes + OnesMinutes;
	
	int OnesSeconds = Seconds % 10;
	int TensSeconds =  (Seconds - OnesSeconds) / 10;
	int ResultSeconds = 0;
	TensSeconds = TensSeconds << 4;
	ResultSeconds = TensSeconds + OnesSeconds;
	
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select hours register                                           // write address for reading data
	TWI_Write(ResultSeconds);			// read hours register
	TWI_Write(ResultMinutes);			// read hours register                                        // write address for reading data
	TWI_Write(ResultHours);				// read hours register
	TWI_Stop();							// stop transmission 
}

void getTime(int *hr,int *min,int *sec){
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select hours registers
	TWI_Start();                        // repeated start
	TWI_Write(0xD1);                    // write address for reading data
	*sec = TWI_Read(ACK);                // read hours register                   // write address for reading data
	*min = TWI_Read(ACK);                // read hours register                  // write address for reading data
	*hr= TWI_Read(NACK);                // read hours register
	TWI_Stop();                         // stop transmission
	
}

enum TemperatureStringScales{Celsius = 0, Fahrenheit = 1};

int getTemp(int t){
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


void displayData(int DisplayMode, int x, int y){
	LCD_Clear();
	switch (DisplayMode){
		case 1:
			
			LCD_GoTo(x,y);
			LCD_WriteText("Voltage: ");
			
			if (adc >= 10){
				itoa(adc, VoltageString, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(VoltageString[0]);
				uartAddToTxBuffer('.');
				uartAddToTxBuffer(VoltageString[1]);
				uartSendTxBuffer();
				
				LCD_GoTo(x+9,y);
				LCD_WriteData(VoltageString[0]);
				LCD_GoTo(x+10,y);
				LCD_WriteText(".");
				LCD_GoTo(x+11,y);
				LCD_WriteData(VoltageString[1]);
			}
			else{
				itoa(adc, VoltageString, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(VoltageString[0]);

				uartSendTxBuffer();
				
				LCD_GoTo(x+9,y);
				LCD_WriteText("0");
				LCD_GoTo(x+10,y);
				LCD_WriteText(".");
				LCD_GoTo(x+11,y);
				LCD_WriteData(VoltageString[0]);
			}
			break;
			
		case 2:
				
				LCD_GoTo(x, y);
				LCD_WriteText("Temp: ");
				
				itoa(ReceivedTemperature, TemperatureString, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(TemperatureString[0]);
				uartAddToTxBuffer(TemperatureString[1]);
				uartAddToTxBuffer('.');
				uartAddToTxBuffer(TemperatureString[2]);
				uartSendTxBuffer();
				LCD_GoTo(x+7, y);
				LCD_WriteData(TemperatureString[0]);
				LCD_GoTo(x+8, y);
				LCD_WriteData(TemperatureString[1]);
				LCD_GoTo(x+9, y);
				LCD_WriteText(".");
				LCD_GoTo(x+10, y);
				LCD_WriteData(TemperatureString[2]);
				break;
		
		case 3:
				
				B2 = ((Hours>>4)&7);     // convert seconds to ASCII char
				B3 = (Hours&15);         // convert seconds to ASCII char
				itoa(B2, TimeString, 10);
				LCD_GoTo(x, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(TimeString[0]);
				
				itoa(B3, TimeString, 20);
				LCD_GoTo(x+1, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer(TimeString[0]);

				LCD_GoTo(x+2,y);
				LCD_WriteText(":");
				uartAddToTxBuffer(':');

				B2 = ((Minutes>>4)&7);     // convert seconds to ASCII char
				B3 = (Minutes&15);         // convert seconds to ASCII char
				itoa(B2, TimeString, 20);
				LCD_GoTo(x+3, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer(TimeString[0]);
				itoa(B3, TimeString, 20);
				LCD_GoTo(x+4, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer(TimeString[0]);
				
				LCD_GoTo(x+5, y);
				LCD_WriteText(":");
				uartAddToTxBuffer(':');

				B2 = ((Seconds>>4)&7);     // convert seconds to ASCII char
				B3 = (Seconds&15);         // convert seconds to ASCII char
				itoa(B2, TimeString, 20);
				LCD_GoTo(x+6, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer(TimeString[0]);
				itoa(B3, TimeString, 20);
				LCD_GoTo(x+7, y);
				LCD_WriteData(TimeString[0]);
				uartAddToTxBuffer(TimeString[0]);
				
				uartSendTxBuffer();
				break;
	}
}

void checkBtn(){
	if (!(PINB & 1)){
		if(PreviousState == 0){
			DisplayMode = DisplayMode + 1;
			LCD_WriteCommand(HD44780_CLEAR);
			PreviousState = 1;
			if(DisplayMode >= 4){
				DisplayMode = 1;
			}
		}
	}
	else{
		PreviousState = 0;
	}
}

void USART_Receiving(){
		cBuffer* UBuffer = uartGetRxBuffer();
		UBuffer->dataptr[UBuffer->datalength - 1] = NULL;
		ReceivedData = UBuffer->dataptr;
		UBuffer->datalength = 0;
		if(strcmp(ReceivedData, "volt") == 0){
			LCD_WriteText("Volt");
			DisplayMode = 1;
		}
		else if(strcmp(ReceivedData, "temp") == 0){
			LCD_WriteText("Temp");
			DisplayMode = 2;
		}
		else if(strcmp(ReceivedData, "time") == 0){
			LCD_WriteText("Time");
			DisplayMode = 3;
			watchdog = 1;
			while(1);
			
		}
		else if(strcmp(ReceivedData, "settime") == 0){
			cli();	
			USART_GetString(&Time);
			sei();
			int hours = ((Time[0] - 48) * 10) + (Time[1] - 48);		
			int minutes = ((Time[2] - 48) * 10) + (Time[3] - 48);
			int seconds = ((Time[4] - 48) * 10) + (Time[5] - 48);
			if((hours > 23) || minutes > 59 || seconds > 59){
				USART_PutString("Wrong time format\r");	
				DisplayMode = 3;
			}
			else{
				setTime(hours, minutes, seconds);
				DisplayMode = 3;
			}
		}
		displayData(DisplayMode, 0, 1);		
}

ISR(TIMER1_COMPA_vect){
	getTime(&Hours, &Minutes, &Seconds);
	ReceivedTemperature = getTemp(Celsius);
	if(watchdog == 0)
		wdt_reset();
}

ISR(ADC_vect){
	adc_read();
}

int main(void){
DDRD = 0xFF;

LCD_Initialize();
ADC_Init();
SPI_Init();
TWI_Init();
Timer1_Init();

setTime(16, 17, 18);

uartInit();
uartSetBaudRate(9600);
rprintfInit(uartSendByte);
cBuffer* UBuffer = uartGetRxBuffer();    

USART_Int_Init();

set_sleep_mode(SLEEP_MODE_IDLE);
sleep_enable();
wdt_enable(7);
sei();

while(1){
	while(UBuffer->dataptr[UBuffer->datalength - 1] != 13){
		sleep_cpu();	
	}
	USART_Receiving();
	}
return 0;
}

