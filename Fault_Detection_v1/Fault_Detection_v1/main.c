/*
 * Fault_Detection_v1.c
 *
 * Created: 2019-09-05 1:03:03 PM
 * Author : jones
 */ 

#include <avr/io.h>
#define F_CPU 20000000/6	//20MHz oscillator with a Div of 6 = 3 333 333 Hz
#include <util/delay.h>
#include <stdio.h>


int main(void)
{
	/* initialization of registers */
	//PORTA.DIRSET = 0x20;	//sets PIN5 as output (not sure of needed)
	PORTB.DIRSET = 0x04;	//sets PB2 as output 
	PORTA.IN = 0xDF;		//sets all A pins to be inputs except for PIN5
	
	ADC0.CTRLA |= 0x01;		//enables ADC
	ADC0.CTRLC |= 0x10;		//sets ADC reference to VCC
	ADC0.INTCTRL = 0x01;	//sets write interrupt
	
	//see page 289 in manual (23.3.1) for initialization
	USART0.BAUD = 9600;		//set baud
	USART0.CTRLC &= 0x37;	//set first two bits 00 to ensure asynchronous and set 1 stop bit
	USART0.CTRLC |= 0x03;	//set character size to 8 bit
	
	float ADCResult = 0; 
	float Voltage = 0;		//ADC raw output voltage adjusted to readable value
	float Voltage_Current_Sensor = 0;		//ADC voltage output from current sensor adjusted to readable value
	float Current = 0;
	float Current_Sensor_Volt_Offset = 0.485;
	float Current_Sensor_Sensitivity = 0.08;	//80mV/A
	float BAUD = 0;
	float UART = 0;
	float clkb = 0;
	char tx[] =  "grantjones" ;
	float tx_before = 0;
	float tx_after = 0;
	int tx_length = 0;
	int tx_counter = 0;


	while (1)
	{
		//LED ON - get voltage
		PORTA.PIN5CTRL = 0x08;		//enables pull up for PIN4 (LED)
		
		ADC0.MUXPOS = 0x04;			//sets which pin gets the analog signal PA4 (PIN2)
		ADC0.COMMAND |= 0x01;		//begins single time conversion
			while (ADC0.COMMAND == 0x01){}
		ADCResult = ADC0.RES;
		Voltage = ((ADCResult)*5)/1023;		//high and low bytes of 10 bit result converted to volts
		//		_delay_ms(200);
		
		
		//LED OFF - get current
		PORTA.PIN5CTRL = 0x00;		//disables pull up for PIN4
		
		ADC0.MUXPOS = 0x02;			//sets which pin gets the analog signal on PA2 (PIN18)
		ADC0.COMMAND |= 0x01;		//begins single time conversion
			while (ADC0.COMMAND == 0x01){}	//could be an issue
		ADCResult = ADC0.RES;
		Voltage_Current_Sensor = (((ADCResult)*5)/1023) - Current_Sensor_Volt_Offset;		//Voltage = Vin*1023/Vref
		Current = (Voltage_Current_Sensor)/Current_Sensor_Sensitivity;  //<-- offset
		//		_delay_ms(200);				//0.535A correct, compared to 0.658A being read by the sensor/MCU
		
		
		/*send over uart*/
		BAUD = USART0.BAUD;
		UART = USART0.CTRLC;
		clkb = CLKCTRL.MCLKCTRLB;
		tx_before = USART0.TXDATAL;
		tx_counter = 0;
		tx_length = sizeof(tx);
		
		if (!((USART0.STATUS & 0x20) == 0)) /*if ready*/ {
			USART0.CTRLB |= 0x40;			//tx enable
			
			while (tx_length > tx_counter) {
				while ((USART0.STATUS & 0x20) == 0){}
				USART0.TXDATAL = (tx[tx_counter]);
				tx_counter++;
				
			}
			
					PORTA.PIN5CTRL = 0x00;		//disables pull up for PIN4
					_delay_ms(1000);
					PORTA.PIN5CTRL = 0x08;		//enables pull up for PIN4 (LED)
				
		}
		
		tx_after = USART0.TXDATAL;

	}
}

