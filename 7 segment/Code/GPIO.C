#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"

#define  SEG1           P06
#define  SEG2           P15

#define  D_A            P04
#define  D_B            P03
#define  D_C            P00
#define  D_D            P10
#define  D_E            P11
#define  D_F            P05
#define  D_G            P01
#define  RX_FRAME_LENGTH  32

void Display_Digit(unsigned char Digit);
void Display_Data(unsigned char val);
void UART1_Tx_Init(void);
void Send_String_To_UART1(const char *text);
void Reset_Command_Parser(void);
void Process_Command_Frame(void);

volatile unsigned char display_value = 0;
volatile unsigned char frame_index = 0;
volatile bit frame_active = 0;
volatile char rx_frame[RX_FRAME_LENGTH];

void UART1_Tx_Init(void)
{
	InitialUART1_Timer3(9600);

	RI_1 = 0;
	TI_1 = 0;
}

void Send_String_To_UART1(const char *text)
{
	while(*text != '\0')
	{
		Send_Data_To_UART1(*text);
		text++;
	}
}

void Reset_Command_Parser(void)
{
	frame_active = 0;
	frame_index = 0;
}

void Process_Command_Frame(void)
{
	unsigned char i;

	for(i = 0; i < frame_index; i++)
	{
		if(rx_frame[i] == ':')
		{
			if((i + 2 < frame_index) &&
			   (rx_frame[i + 1] >= '0') && (rx_frame[i + 1] <= '9') &&
			   (rx_frame[i + 2] >= '0') && (rx_frame[i + 2] <= '9'))
			{
				display_value = ((rx_frame[i + 1] - '0') * 10) + (rx_frame[i + 2] - '0');
			}
			break;
		}
	}
}

void NUM_0()
{
	D_A=0; D_B=0; D_C=0; D_D=0;
	D_E=0; D_F=0; D_G=1;
}

void NUM_1()
{
	D_A=1; D_B=0; D_C=0; D_D=1;
	D_E=1; D_F=1; D_G=1;
}

void NUM_2()
{
	D_A=0; D_B=0; D_C=1; D_D=0;
	D_E=0; D_F=1; D_G=0;
}

void NUM_3()
{
	D_A=0; D_B=0; D_C=0; D_D=0;
	D_E=1; D_F=1; D_G=0;
}

void NUM_4()
{
	D_A=1; D_B=0; D_C=0; D_D=1;
	D_E=1; D_F=0; D_G=0;
}

void NUM_5()
{
	D_A=0; D_B=1; D_C=0; D_D=0;
	D_E=1; D_F=0; D_G=0;
}

void NUM_6()
{
	D_A=0; D_B=1; D_C=0; D_D=0;
	D_E=0; D_F=0; D_G=0;
}

void NUM_7()
{
	D_A=0; D_B=0; D_C=0; D_D=1;
	D_E=1; D_F=1; D_G=1;
}

void NUM_8()
{
	D_A=0; D_B=0; D_C=0; D_D=0;
	D_E=0; D_F=0; D_G=0;
}

void NUM_9()
{
	D_A=0; D_B=0; D_C=0; D_D=0;
	D_E=1; D_F=0; D_G=0;
}

void clear_NUMS()
{
	D_A=1; D_B=1; D_C=1; D_D=1;
	D_E=1; D_F=1; D_G=1;
}

void Display_Digit(unsigned char Digit)
{
	switch(Digit)
	{
		case 0: NUM_0(); break;
		case 1: NUM_1(); break;
		case 2: NUM_2(); break;
		case 3: NUM_3(); break;
		case 4: NUM_4(); break;
		case 5: NUM_5(); break;
		case 6: NUM_6(); break;
		case 7: NUM_7(); break;
		case 8: NUM_8(); break;
		case 9: NUM_9(); break;
	}
}

void Display_Data(unsigned char val)
{
	clear_NUMS();

	SEG2 = 0;
	SEG1 = 1;
	Display_Digit(val % 10);
	Timer0_Delay1ms(2);

	clear_NUMS();

	SEG1 = 0;
	SEG2 = 1;
	Display_Digit(val / 10);
	Timer0_Delay1ms(2);
}

void UART0_Init(void)
{
	InitialUART0_Timer1(9600);

	RI = 0;
	TI = 0;

	set_ES;
	set_EA;
}

void UART0_ISR(void) interrupt 4
{
	char rx;

	if(RI)
	{
		RI = 0;

		rx = SBUF;
		Send_Data_To_UART1(rx);

		if(rx == '@')
		{
			Reset_Command_Parser();
			frame_active = 1;
			rx_frame[frame_index++] = rx;
		}
		else if(frame_active)
		{
			if(frame_index < (RX_FRAME_LENGTH - 1))
			{
				rx_frame[frame_index++] = rx;
			}
			else
			{
				Reset_Command_Parser();
			}

			if(frame_active && rx == '\n')
			{
				rx_frame[frame_index] = '\0';
				Process_Command_Frame();
				Reset_Command_Parser();
			}
		}
	}

	if(TI)
	{
		TI = 0;
	}
}

void main(void)
{
	unsigned char value;

	Set_All_GPIO_Quasi_Mode;

	P15_PushPull_Mode;
	P06_PushPull_Mode;

	P04_PushPull_Mode;
	P03_PushPull_Mode;
	P00_PushPull_Mode;
	P10_PushPull_Mode;
	P11_PushPull_Mode;
	P05_PushPull_Mode;
	P01_PushPull_Mode;

	UART0_Init();
	UART1_Tx_Init();
	Send_String_To_UART1("UART Ready\r\n");

	while(1)
	{
		value = display_value;

		Display_Data(value);
	}
}
