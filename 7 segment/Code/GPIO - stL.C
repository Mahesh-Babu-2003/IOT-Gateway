/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2017 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

//***********************************************************************************************************
//  Website: http://www.nuvoton.com
//  E-Mail : MicroC-8bit@nuvoton.com
//  Date   : Jan/21/2017
//***********************************************************************************************************

//***********************************************************************************************************
//  File Function: N76E003 GPIO demo code
//***********************************************************************************************************
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"
#include "math.h"

#define  SEG1           P04
#define  SEG2           P15
#define  D_A            P03
#define  D_B 						P05
#define  D_C            P12
#define  D_D            P11
#define  D_E            P10 
#define  D_F            P00
#define  D_G            P01
 
//////////////////////////////////////
void Display_Data(unsigned char val);
void Display_Digit(unsigned char Digit);


//*****************  The Following is in define in Fucntion_define.h  ***************************
//****** Always include Function_define.h call the define you want, detail see main(void) *******
//***********************************************************************************************
#if 0
////------------------- Define Port as Quasi mode  -------------------
//#define P00_Quasi_Mode				P0M1&=~SET_BIT0;P0M2&=~SET_BIT0
//#define P01_Quasi_Mode				P0M1&=~SET_BIT1;P0M2&=~SET_BIT1
//#define P02_Quasi_Mode				P0M1&=~SET_BIT2;P0M2&=~SET_BIT2
//#define P03_Quasi_Mode				P0M1&=~SET_BIT3;P0M2&=~SET_BIT3
//#define P04_Quasi_Mode				P0M1&=~SET_BIT4;P0M2&=~SET_BIT4
//#define P05_Quasi_Mode				P0M1&=~SET_BIT5;P0M2&=~SET_BIT5
//#define P06_Quasi_Mode				P0M1&=~SET_BIT6;P0M2&=~SET_BIT6
//#define P07_Quasi_Mode				P0M1&=~SET_BIT7;P0M2&=~SET_BIT7
//#define P10_Quasi_Mode				P1M1&=~SET_BIT0;P1M2&=~SET_BIT0
//#define P11_Quasi_Mode				P1M1&=~SET_BIT1;P1M2&=~SET_BIT1
//#define P12_Quasi_Mode				P1M1&=~SET_BIT2;P1M2&=~SET_BIT2
//#define P13_Quasi_Mode				P1M1&=~SET_BIT3;P1M2&=~SET_BIT3
//#define P14_Quasi_Mode				P1M1&=~SET_BIT4;P1M2&=~SET_BIT4
//#define P15_Quasi_Mode				P1M1&=~SET_BIT5;P1M2&=~SET_BIT5
//#define P16_Quasi_Mode				P1M1&=~SET_BIT6;P1M2&=~SET_BIT6
//#define P17_Quasi_Mode				P1M1&=~SET_BIT7;P1M2&=~SET_BIT7
//#define P20_Quasi_Mode				P2M1&=~SET_BIT0;P2M2&=~SET_BIT0
//#define P30_Quasi_Mode				P3M1&=~SET_BIT0;P3M2&=~SET_BIT0
////------------------- Define Port as Push Pull mode -------------------
//#define P00_PushPull_Mode			P0M1|=SET_BIT0;P0M2&=~SET_BIT0
//#define P01_PushPull_Mode			P0M1|=SET_BIT1;P0M2&=~SET_BIT1
//#define P02_PushPull_Mode			P0M1&=~SET_BIT2;P0M2|=SET_BIT2
//#define P03_PushPull_Mode			P0M1&=~SET_BIT3;P0M2|=SET_BIT3
//#define P04_PushPull_Mode			P0M1&=~SET_BIT4;P0M2|=SET_BIT4
//#define P05_PushPull_Mode			P0M1&=~SET_BIT5;P0M2|=SET_BIT5
//#define P06_PushPull_Mode			P0M1&=~SET_BIT6;P0M2|=SET_BIT6
//#define P07_PushPull_Mode			P0M1&=~SET_BIT7;P0M2|=SET_BIT7
//#define P10_PushPull_Mode			P1M1&=~SET_BIT0;P1M2|=SET_BIT0
//#define P11_PushPull_Mode			P1M1&=~SET_BIT1;P1M2|=SET_BIT1
//#define P12_PushPull_Mode			P1M1&=~SET_BIT2;P1M2|=SET_BIT2
//#define P13_PushPull_Mode			P1M1&=~SET_BIT3;P1M2|=SET_BIT3
//#define P14_PushPull_Mode			P1M1&=~SET_BIT4;P1M2|=SET_BIT4
//#define P15_PushPull_Mode			P1M1&=~SET_BIT5;P1M2|=SET_BIT5
//#define P16_PushPull_Mode			P1M1&=~SET_BIT6;P1M2|=SET_BIT6
//#define P17_PushPull_Mode			P1M1&=~SET_BIT7;P1M2|=SET_BIT7
//#define P20_PushPull_Mode			P2M1&=~SET_BIT0;P2M2|=SET_BIT0
//#define P30_PushPull_Mode			P3M1&=~SET_BIT0;P3M2|=SET_BIT0
////------------------- Define Port as Input Only mode -------------------
//#define P00_Input_Mode				P0M1|=SET_BIT0;P0M2&=~SET_BIT0
//#define P01_Input_Mode				P0M1|=SET_BIT1;P0M2&=~SET_BIT1
//#define P02_Input_Mode				P0M1|=SET_BIT2;P0M2&=~SET_BIT2
//#define P03_Input_Mode				P0M1|=SET_BIT3;P0M2&=~SET_BIT3
//#define P04_Input_Mode				P0M1|=SET_BIT4;P0M2&=~SET_BIT4
//#define P05_Input_Mode				P0M1|=SET_BIT5;P0M2&=~SET_BIT5
//#define P06_Input_Mode				P0M1|=SET_BIT6;P0M2&=~SET_BIT6
//#define P07_Input_Mode				P0M1|=SET_BIT7;P0M2&=~SET_BIT7
//#define P10_Input_Mode				P1M1|=SET_BIT0;P1M2&=~SET_BIT0
//#define P11_Input_Mode				P1M1|=SET_BIT1;P1M2&=~SET_BIT1
//#define P12_Input_Mode				P1M1|=SET_BIT2;P1M2&=~SET_BIT2
//#define P13_Input_Mode				P1M1|=SET_BIT3;P1M2&=~SET_BIT3
//#define P14_Input_Mode				P1M1|=SET_BIT4;P1M2&=~SET_BIT4
//#define P15_Input_Mode				P1M1|=SET_BIT5;P1M2&=~SET_BIT5
//#define P16_Input_Mode				P1M1|=SET_BIT6;P1M2&=~SET_BIT6
//#define P17_Input_Mode				P1M1|=SET_BIT7;P1M2&=~SET_BIT7
//#define P20_Input_Mode				P2M1|=SET_BIT0;P2M2&=~SET_BIT0
//#define P30_Input_Mode				P3M1|=SET_BIT0;P3M2&=~SET_BIT0
////-------------------Define Port as Open Drain mode -------------------
//#define P00_OpenDrain_Mode		P0M1|=SET_BIT0;P0M2|=SET_BIT0
//#define P01_OpenDrain_Mode		P0M1|=SET_BIT1;P0M2|=SET_BIT1
//#define P02_OpenDrain_Mode		P0M1|=SET_BIT2;P0M2|=SET_BIT2
//#define P03_OpenDrain_Mode		P0M1|=SET_BIT3;P0M2|=SET_BIT3
//#define P04_OpenDrain_Mode		P0M1|=SET_BIT4;P0M2|=SET_BIT4
//#define P05_OpenDrain_Mode		P0M1|=SET_BIT5;P0M2|=SET_BIT5
//#define P06_OpenDrain_Mode		P0M1|=SET_BIT6;P0M2|=SET_BIT6
//#define P07_OpenDrain_Mode		P0M1|=SET_BIT7;P0M2|=SET_BIT7
//#define P10_OpenDrain_Mode		P1M1|=SET_BIT0;P1M2|=SET_BIT0
//#define P11_OpenDrain_Mode		P1M1|=SET_BIT1;P1M2|=SET_BIT1
//#define P12_OpenDrain_Mode		P1M1|=SET_BIT2;P1M2|=SET_BIT2
//#define P13_OpenDrain_Mode		P1M1|=SET_BIT3;P1M2|=SET_BIT3
//#define P14_OpenDrain_Mode		P1M1|=SET_BIT4;P1M2|=SET_BIT4
//#define P15_OpenDrain_Mode		P1M1|=SET_BIT5;P1M2|=SET_BIT5
//#define P16_OpenDrain_Mode		P1M1|=SET_BIT6;P1M2|=SET_BIT6
//#define P17_OpenDrain_Mode		P1M1|=SET_BIT7;P1M2|=SET_BIT7
//#define P20_OpenDrain_Mode		P2M1|=SET_BIT0;P2M2|=SET_BIT0
//#define P30_OpenDrain_Mode		P3M1|=SET_BIT0;P3M2|=SET_BIT0
////--------- Define all port as quasi mode ---------
//#define Set_All_GPIO_Quasi_Mode			P0M1=0;P0M1=0;P1M1=0;P1M2=0;P2M1=0;P2M2=0;P3M1=0;P3M2=0
#endif

/*------------------------------------------------
The main C function.  Program execution starts
here after stack initialization.
------------------------------------------------*/
void NUM_0()
{
	D_A = 0;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 0;		//E
	D_F = 0;		//F
	D_G = 1;		//G
	
} 
void NUM_1()
{
	D_A = 1;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 1;		//D
	D_E = 1;		//E
	D_F = 1;		//F
	D_G = 1;		//G
	
}

void NUM_2()
{
	D_A = 0;		//A
	D_B = 0;		//B
	D_C = 1;		//C
	D_D = 0;		//D
	D_E = 0;		//E
	D_F = 1;		//F
	D_G = 0;		//G
	
}
void NUM_3()
{
	D_A = 0;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 1;		//E
	D_F = 1;		//F
	D_G = 0;		//G
}
void NUM_4()
{
	D_A = 1;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 1;		//D
	D_E = 1;		//E
	D_F = 0;		//F
	D_G = 0;		//G
	
}
void NUM_5()
{
	D_A = 0;		//A
	D_B = 1;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 1;		//E
	D_F = 0;		//F
	D_G = 0;		//G
	
}
void NUM_6()
{
	D_A = 0;		//A
	D_B = 1;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 0;		//E
	D_F = 0;		//F
	D_G = 0;		//G
	
}
void NUM_7()
{
	D_A = 0;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 1;		//D
	D_E = 1;		//E
	D_F = 1;		//F
	D_G = 1;		//G
	
}
void NUM_8()
{
  D_A = 0;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 0;		//E
	D_F = 0;		//F
	D_G = 0;		//G	
}

void NUM_9()
{
  D_A = 0;		//A
	D_B = 0;		//B
	D_C = 0;		//C
	D_D = 0;		//D
	D_E = 1;		//E
	D_F = 0;		//F
	D_G = 0;		//G	
}
void clear_NUMS()
{
	D_A = 1;		//A
	D_B = 1;		//B
	D_C = 1;		//C
	D_D = 1;		//D
	D_E = 1;		//E
	D_F = 1;		//F
	D_G = 1;    //G
}


void Display_Digit(unsigned char Digit)
{
		switch(Digit)
	{
		case 0:
				NUM_0();	
		break;
		
		case 1:
				NUM_1();
		break;
		case 2:
			 NUM_2();
		break;
		
		case 3:
			NUM_3();
		break;
		
		case 4:
			NUM_4();
		break;
		
		case 5:
			NUM_5();
		break;
		case 6:
			NUM_6();	
		break;
		
		case 7:
			NUM_7();
		break;
		
		case 8:
			NUM_8(); 		
		break;
		
		case 9:
			NUM_9();
		break;
	}
}
void Display_Data(unsigned char val)
{
	  clear_NUMS();
		SEG2 = 0; 		
		SEG1 = 1;
		Display_Digit(val % 10); 	
    Timer0_Delay1ms(4);
		SEG1 = 0;		
		SEG2 = 1;
		Display_Digit(val / 10);
    Timer0_Delay1ms(5);
}

int Get_Temp()
{
	int val=0;
	int temp_val=0;
	double temp_double=0;
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
	val=ADCRH;
	val=(ADCRH*0x10)|ADCRL;
	temp_val = val;            
	temp_double = log(10000.0*((4096.0/temp_val-1)));
	temp_double = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * temp_double * temp_double ))* temp_double);
	temp_double = temp_double - 273.15;
	temp_val = temp_double;
	while(ADCF == 0);
	return temp_val;
}

unsigned int counter = 100,Value = 88;
void main (void) 
{
	int temp_val=0;
	Set_All_GPIO_Quasi_Mode;
	Enable_ADC_AIN2;			
	while(counter >= 1)
	{
		Display_Data(Value);
		counter--;			
	}	
	counter = 250;
	temp_val = Get_Temp();
  while(1)
  {
		counter++;
		if(counter > 200)
		{
			counter = 0;
      temp_val = Get_Temp();
			if(temp_val > 99)
				temp_val = 99;	
		}
	
		
		Display_Data(temp_val);
//		Timer0_Delay1ms(5);

	}
	

}
