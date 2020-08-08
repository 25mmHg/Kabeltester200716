/*
 * Kabeltester191026.c
 * LFUSE=0xFF
 * HFUSE=0xD9
 * Created: 17.07.2020 18:44:35
 * Author : 25mmHg / STLEU
 */ 

/*                                  Messpin                        Messpin
                        _____    LED          .-----.   .-----.           LED      _____
                VCC----|_____|--->|---08------|PB0  |   |  PA0|------00---|<------|_____|----VCC
                VCC---    *       *   09------|PB1  '---'  PA1|------01    *         *    ---VCC
                VCC---    *       *   10------|PB2         PA2|------02    *         *    ---VCC
                VCC---    *       *   11------|PB3    A    PA3|------03    *         *    ---VCC
                VCC---    *       *   12------|PB4    T    PA4|------04    *         *    ---VCC
                VCC---    *       *   13------|PB5    m    PA5|------05    *         *    ---VCC
                VCC---  __*__    LED  14------|PB6    e    PA6|------06   LED      __*__  ---VCC
                VCC----|_____|--->|---15------|PB7    g    PA7|------07---|<------|_____|----VCC
                                            --|!RESET a   AREF|-
                                     VCC------|               |------GND
                                     GND------|       1   AVCC|------VCC  LED      _____
                                            --|XTAL2  6    PC7|------16---|<------|_____|----VCC
                                            --|XTAL1       PC6|------17    *         *    ---VCC
                                         .----|PD0 LED     PC5|------18    *         *    ---VCC
                                         |.---|PD1 TXD     PC4|------19    *         *    ---VCC
                                         ||.--|PD2 SP1     PC3|------20    *         *    ---VCC
                        _____    LED     |||.-|PD3 SP2     PC2|------21    *         *    ---VCC
                VCC----|_____|--->|---24-((((-|PD4         PC1|------22    *         *    ---VCC
                VCC---  __*__    LED  25-((((-|PD5         PC0|------23   LED      __*__  ---VCC
                VCC----|_____|--->|---26-((((-|PD6         PD7|------27---|<------|_____|----VCC
                                         |||| '---------------'
                                         ||||
                                         |||'------------------------.
                                         |||                         |
                                  .------'|'-----.    _              |
                                  |       |      |   | |     _____   |
                                  |       |      o--|| ||---|_____|--o 
                                  |       |      |   |_|      470    |
                                 .-.      |     .-. PIEZO           .-.
                                 | |      |     | |                 | |
                                 | |      |     | | 470             | | 470
                                 '-'      |     '-'                 '-'
                                  |       |      |                   |
                                  |       |      |                   |
                                  V LED   |      \                   \
                                  -       |       \                   \
                                  |       o      |                   |
                                 GND   UART TX  GND                 GND

                           NOTE: CICUIT SCHEME WITHOUT POWER AND XTAL 16MHz
*/

/*
 * TO DO List
 * error() erzeugt nur Sound, Einfluss auf Programmablauf?
 * Englisches oder Deutsches UI 
 */

#define F_CPU 16000000UL
#define USART_BAUDRATE 38400UL
#define USART_BAUD_CALC (F_CPU/(USART_BAUDRATE*8UL))-1 // ***without U2X use 16UL
#define MAXPINS 28
#define WAITTIME_MS 40
#define HALF1WAVE_US 66
#define HALF2WAVE_US 100
#define LEDPIN          (1<<0)
#define SOUNDPIN1       (1<<2)
#define SOUNDPIN2       (1<<3)
#define SET4BUTTONS     (DDRD  &= ~(SOUNDPIN1 | SOUNDPIN2), PORTD |= (SOUNDPIN1 | SOUNDPIN2))
#define NOBUTTON        (PIND  & (1<<2))
#define ISRECMODE       (PIND  & (1<<3))
#define SET4SOUNDS      (DDRD  |= (SOUNDPIN1 | SOUNDPIN2), PORTD |= SOUNDPIN1, PORTD &= ~SOUNDPIN2)
#define SET4LED         (DDRD  |=  LEDPIN)
#define LEDON           (PORTD |=  LEDPIN)
#define LEDOFF          (PORTD &= ~LEDPIN)

#include <avr/io.h>
#include <util/delay.h>

/*global Variables*/

uint32_t  istValue[28];
uint32_t sollValue[] = {
//    1zu1-Verdrahtung mit 14 Adern paarweise
//    PIND7-4|PINC0-7|PINB7-0|PINA7-0|
//    XXXX 4 |   8   |   8   |   8   | 
    0b00000000000000000000000000000011, //pin 00+01
    0b00000000000000000000000000000011, //pin 01+00
    0b00000000000000000000000000001100, //pin 02+03
    0b00000000000000000000000000001100, //pin 03+02
    0b00000000000000000000000000110000, //pin 04+05
    0b00000000000000000000000000110000, //pin 05+04
    0b00000000000000000000000011000000, //pin 06+07
    0b00000000000000000000000011000000, //pin 07+06
    0b00000000000000000000001100000000, //pin 08+09
    0b00000000000000000000001100000000, //pin 09+08
    0b00000000000000000000110000000000, //pin 10+11
    0b00000000000000000000110000000000, //pin 11+10
    0b00000000000000000011000000000000, //pin 12+13
    0b00000000000000000011000000000000, //pin 13+12
    0b00000000000000001100000000000000, //pin 14+15
    0b00000000000000001100000000000000, //pin 15+14
    0b00000000000000110000000000000000, //pin 16+17
    0b00000000000000110000000000000000, //pin 17+16
    0b00000000000011000000000000000000, //pin 18+19
    0b00000000000011000000000000000000, //pin 19+18
    0b00000000001100000000000000000000, //pin 20+21
    0b00000000001100000000000000000000, //pin 21+20
    0b00000000110000000000000000000000, //pin 22+23
    0b00000000110000000000000000000000, //pin 23+22
    0b00000011000000000000000000000000, //pin 24+25
    0b00000011000000000000000000000000, //pin 25+24
    0b00001100000000000000000000000000, //pin 26+27
    0b00001100000000000000000000000000};//pin 27+26
	
uint16_t passcount=0;
uint16_t failcount=0;

/* subroutines */

void usart_init()
{
	//SET USART BAUDRATE
	UBRRH = (uint8_t) ((USART_BAUD_CALC) >> 8);
	UBRRL = (uint8_t) USART_BAUD_CALC;
	//ENABLE  DOUBLE PRECISION 4 HIGH BAUDRATES, LOOK ***
	UCSRA = 1 << U2X;
	//ENABLE TRANSMITTER
	UCSRB = 1 << TXEN;
	//ENABLE RECEIVER
	//UCSRB = 1 << RXEN;
	//SET  FRAME  FORMAT  8N1
	UCSRC = (1 << URSEL) | (0 << USBS) | (1 << UCSZ1) | (1 << UCSZ0);
}

void usart_TX_char(char payload)
{
	//WAIT FOR EMPTY BUFFER
	while(!(UCSRA & (1 << UDRE)));
	UDR = payload;
}

void usart_TX_string(char *payload_ptr)
{
	//WAIT FOR  STRING END
	while(*payload_ptr)
	{
		usart_TX_char(*payload_ptr);
		payload_ptr++;
	}
}

void usart_TX_hex(uint16_t payload)
{
	//DISPLAY NUMBERS SIMPLE BUT JUST NOT USED THERE!
	//CONVERT UNSIGNED INTEGER 2 HEX NUMBERS AND SHOW IT
	const uint16_t mask = 0xF;
	for(uint8_t i=0; i<4; i++)
	{
		uint8_t temp = (uint8_t)((payload >> (4*(3-i))) & mask);
		usart_TX_char(temp < 0xA ? temp+'0' : temp-0xA+'A');
	}
}

void usart_TX_dec(uint16_t payload)
{
	//DISPLAY NUMBERS THE HARD WAY!
	//CONVERT UNSIGNED INTEGER 2 DECIMAL NUMBERS AND SHOW IT IN 5 DIGITS
	uint8_t n = 4; //Anzahl der Nullen (Dezimalstellen -1)
	uint32_t temp = payload;
	char null = ' '; //autospace vor 1.Ziffer
	while(n--)
	{
		// DIV 16,8,4,2
		temp >>= n;
		uint8_t m = n;
		while(m--)
		{
			// DIV 625, 125, 25, 5 
			temp++;
			temp *=  0x333U;
			temp >>= 12;
		}
		m = n;
		if(!temp && n)
		{
			usart_TX_char(null);
		}
		else
		{
			usart_TX_char(temp + '0');
			null = '0';
		}
		// * 10k, 1k, 100, 10
		while(m--) temp *= 10;
		temp = payload - temp;
	}
}

void showNet2Serial(uint32_t net[])
{
	usart_TX_string("\n");
	usart_TX_string("o:drive G:got E:expects X:match\n");
	usart_TX_string("Pin 0.........................27\n");
	usart_TX_string("Pin 0    5    0    5    0    5 7\n");
	for(int8_t i=0; i<MAXPINS; i++)
	{
		uint32_t mark = 1;
		uint8_t  temp = 0;
		usart_TX_char(' ');
		if(i < 10)
		{
			usart_TX_char(' ');
			temp = i;
		}
		else
		{
			if(i < 20)
			{
				usart_TX_char('1');
				temp = i - 10;
			}
			else
			{
				usart_TX_char('2');
				temp = i - 20;
			}
		}
		usart_TX_char(temp+'0');
		usart_TX_char(' ');
		for(int8_t j=0; j<MAXPINS; j++)
		{
			if(net[i] & mark && sollValue[i] & mark && i==j) usart_TX_char('o');
			else
			{
				if(net[i] & mark && sollValue[i] & mark) usart_TX_char('X');
				else
				{
					if(net[i] & mark) usart_TX_char('G');
					else
						{
						if(sollValue[i] & mark) usart_TX_char('E');
						else usart_TX_char('.');
						}
				}
			}
			mark <<= 1;
		}
		usart_TX_string("\n");
	}
	usart_TX_string("END OF NET\n");
}

void init()
{
	SET4BUTTONS;
	SET4LED;
}

void done()
{
	usart_TX_string("DONE \n");
	SET4SOUNDS;
	for(uint16_t i = 0; i < 4000; i++)
	{
		PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
		_delay_us(HALF2WAVE_US);
	}
	for(uint8_t i = 0; i < 200; i++)
	{
		PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
		_delay_us(HALF1WAVE_US);
	}
	SET4BUTTONS;
}

void pass()
{
	usart_TX_string("PASS \n");
	SET4SOUNDS;
	for(uint16_t i = 0; i < 1000; i++)
	{
		PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
		_delay_us(HALF2WAVE_US);
	}
	for(uint8_t i = 0; i < 200; i++)
	{
		PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
		_delay_us(HALF1WAVE_US);
	}
	SET4BUTTONS;
}

void fail()
{
	usart_TX_string("FAIL \n");
	SET4SOUNDS;
	for(uint8_t k = 0; k < 16; k++)
	{
		for(uint8_t i = 0; i < 100; i++)
		{
			PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
			_delay_us(HALF1WAVE_US);
		}
		_delay_ms(40);
	}
	SET4BUTTONS;
}

void error()
{
	usart_TX_string("ERROR \n");
	SET4SOUNDS;
	for(uint16_t i = 0; i < 1000; i++)
	{
		PORTD ^= (SOUNDPIN1 | SOUNDPIN2);
		_delay_us(HALF2WAVE_US);
	}
	SET4BUTTONS;	
}

void writeByte2EEPROM(uint16_t adress, uint8_t data)
{
	while(EECR & (1<<EEWE));//wait4completion of previous write	
	EEAR = adress;
	EEDR = data;
	EECR |= (1<<EEMWE);
	EECR |= (1<<EEWE);//start EEPROM write
}

uint8_t readEEPROM2Byte(uint16_t adress)
{
	while(EECR & (1<<EEWE));//wait4completion of previous write	
	EEAR = adress;
	EECR |= (1<<EERE);
	return EEDR;//read EEPROM
}	

void writeEEPROM()
{
	uint16_t adress = 0;
	for(uint8_t i = 0; i < MAXPINS; i++)
	{
		for(uint8_t j = 0; j < 4; j++)
		{
			//for MSB left (3-j)<<3=24,16,8,0 and cast to uint8_t
			uint8_t temp = (uint8_t)(sollValue[i] >> ((3-j)<<3) & 0xFF);
			writeByte2EEPROM(adress, temp);
			adress ++;
		}
	}
}

void readEEPROM()
{
	uint16_t adress = 0;
	for(uint8_t i = 0; i < MAXPINS; i++)
	{
		uint32_t temp = 0;
		for(uint8_t j = 0; j < 4; j++)
		{
			//for MSB left (3-j)<<3=24,16,8,0 and cast to uint32_t
			temp += (uint32_t)readEEPROM2Byte(adress) << ((3-j)<<3);
			adress ++;
		}
		sollValue[i] = temp;
	}
}

void clearAllPins(){
	DDRA   =  0;
	PORTA  =  0xFF;
	DDRB   =  0;
	PORTB  =  0xFF;
	DDRC   =  0;
	PORTC  =  0xFF;
	DDRD  &=  0x0F; // unteres Nibbel für UI
	PORTD |=  0xF0; // unteres Nibbel für UI
}
	
void setPin(uint8_t sendPin)
{
	if(sendPin < 8)
	{
		DDRA  |=   (1<<sendPin);
		PORTA &=  ~(1<<sendPin);
	}
	else if(sendPin < 16)
	{
		DDRB  |=   (1<<(sendPin-8));
		PORTB &=  ~(1<<(sendPin-8));
	}
	else if(sendPin < 24)
	{
		DDRC  |=  (128>>(sendPin-16));// geändert von DDRC  |=   (1<<(sendPin-16)); 
		PORTC &= ~(128>>(sendPin-16));// geändert von PORTC &=  ~(1<<(sendPin-16)); 
	}
	else if(sendPin < MAXPINS)
	{
		DDRD  |=   (1<<(sendPin-20));
		PORTD &=  ~(1<<(sendPin-20));
	}
	else error();
} 

uint32_t getPins()
{
	uint8_t    a = ~PINA;
	uint16_t   b = ~PINB;
	//uint32_t c = ~PINC; // geändert siehe unten und https://www.mikrocontroller.net/topic/64608?goto=new#new
	uint8_t  c_t = ~PINC;
	//Reihenfolge der Bits vertauschen
	c_t = ((c_t & 0xF0)>>4 | (c_t & 0x0F)<<4);
	c_t = ((c_t & 0xCC)>>2 | (c_t & 0x33)<<2);
	c_t = ((c_t & 0xAA)>>1 | (c_t & 0x55)<<1);
	uint32_t   c = c_t;
	uint32_t   d = (~PIND & 0xF0); // überspringe unteres Nibbel für UI
	return((d<<20) + (c<<16) + (b<<8) + a);	
}

uint8_t getErrors()
{
	uint8_t errors = 0;
	for(uint8_t i = 0; i < MAXPINS; i++)
	{								
		errors += istValue[i] !=  sollValue[i];
	}
	return(errors);
}

uint8_t getNet(uint8_t isrec)
{	
	uint8_t wirecnt = 0;
	usart_TX_string("\n\n\n");
	if(isrec) usart_TX_string("RECORDMODE\n");
	else    usart_TX_string("TESTMODE\n");
	for(uint8_t i = 0; i < MAXPINS; i++)
	{
		clearAllPins();
		setPin(i);
		_delay_ms(WAITTIME_MS);
		uint32_t temp = getPins();
		if(isrec) sollValue[i] = temp;
		istValue[i] = temp;
		if(!temp)  error(); //kein setPin gefunden
		else
		{
			while(temp)
			{
				wirecnt += (temp & 1);
				temp  >>= 1;
			}
			wirecnt--; //das setPin subtrahieren
		}
	}
	if(isrec) writeEEPROM();
	return(wirecnt);
}

int main(void)
{
	init();
	usart_init();
	usart_TX_string("\n");
	usart_TX_string("************************************\n");
	usart_TX_string("***WIRING*COMPARATOR*FOR*28*NODES***\n");
	usart_TX_string("*********VERSION*2020/07/17*********\n");
	usart_TX_string("***©EBD-PW-EM*2020*(STLEU,25mmHg)***\n");
	usart_TX_string("************************************\n\n");
	usart_TX_string("STARTUP: SERIAL OUT 38,4kBaud 8N1\n");
	usart_TX_string("STARTUP: RESET COUNTERS\n");
	if(NOBUTTON)
	{
		readEEPROM();
		usart_TX_string("STARTUP: READ LAST RECORD FROM EEPROM\n");
	}
	else usart_TX_string("STARTUP: LOAD REFERENCE NET (12,34,56...)\n");
	showNet2Serial(sollValue);
	done();
	while (1)
	{
		while(NOBUTTON)
		{
			if(ISRECMODE) LEDON;
			else LEDOFF;
			_delay_ms(WAITTIME_MS);
		}
		if(ISRECMODE)
		{
			uint8_t wires = getNet(1);
			usart_TX_dec(wires);
			usart_TX_string(" wires\n");
			showNet2Serial(istValue);
			passcount = 0;
			failcount = 0;
			usart_TX_string("RESET COUNTERS\n");
			done();
			usart_TX_string("\n\n\n");
		}
		else
		{
			uint8_t wires = getNet(0);
			usart_TX_dec(wires);
			usart_TX_string(" wires\n");
			uint8_t errors = getErrors();
			usart_TX_dec(errors);
			usart_TX_string(" errors ");
			if(errors)
			{
				fail();
				failcount++;
			}
			else
			{
				pass();
				passcount++;
			}
			showNet2Serial(istValue);
			usart_TX_string("\n");
			usart_TX_string("TOTAL PASS: ");
			usart_TX_dec(passcount);
			usart_TX_string("\n");
			usart_TX_string("TOTAL FAIL: ");
			usart_TX_dec(failcount);
			usart_TX_string("\n");
			usart_TX_string("TOTAL: ");
			usart_TX_dec(passcount+failcount);
			usart_TX_string("\n");
		}
	}
}