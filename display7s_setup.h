/*
 * display7s_setup.h
 *
 *  Created on: Dec 30, 2022
 *      Author: jcaf
 */

#ifndef DISPLAY7S_SETUP_H_
#define DISPLAY7S_SETUP_H_

	#define DISP7S_TOTAL_NUMMAX 8

	//DISLAYS DE CATODO COMUN - Todo el puerto de 8bits
    #define PORTWxDISPLAY7S_DATA 		PORTC
    #define PORTRxDISPLAY7S_DATA 		PINC
    #define CONFIGIOxDISPLAY7S_DATA 	DDRC

	///////////////////////////////////////////////////
	//Define position of hardware pin into the Data Port
	#define D7S_A 6
	#define D7S_B 7
	#define D7S_C 1
	#define D7S_D 3
	#define D7S_E 0
	#define D7S_F 5
	#define D7S_G 4
	#define D7S_DP 2


	#define D7S_Ginv D7S_DP
	#define D7S_DPinv D7S_G


	//TRANSISTORES PARA EL BARRIDO/MULTIPLEXING
    #define PORTWxDISPLAY7S_Q0 		PORTD
    #define PORTRxDISPLAY7S_Q0 		PIND
    #define CONFIGIOxDISPLAY7S_Q0 	DDRD
    #define PINxDISPLAY7S_Q0 		5

	#define PORTWxDISPLAY7S_Q1 		PORTD
    #define PORTRxDISPLAY7S_Q1 		PIND
    #define CONFIGIOxDISPLAY7S_Q1 	DDRD
    #define PINxDISPLAY7S_Q1		2

	#define PORTWxDISPLAY7S_Q2 		PORTD
    #define PORTRxDISPLAY7S_Q2 		PIND
    #define CONFIGIOxDISPLAY7S_Q2 	DDRD
    #define PINxDISPLAY7S_Q2		3

	#define PORTWxDISPLAY7S_Q3 		PORTD
    #define PORTRxDISPLAY7S_Q3 		PIND
    #define CONFIGIOxDISPLAY7S_Q3 	DDRD
    #define PINxDISPLAY7S_Q3		4

	#define PORTWxDISPLAY7S_Q4 		PORTA
    #define PORTRxDISPLAY7S_Q4 		PINA
    #define CONFIGIOxDISPLAY7S_Q4 	DDRA
    #define PINxDISPLAY7S_Q4		7

	#define PORTWxDISPLAY7S_Q5 		PORTA
    #define PORTRxDISPLAY7S_Q5 		PINA
    #define CONFIGIOxDISPLAY7S_Q5 	DDRA
    #define PINxDISPLAY7S_Q5		6

	#define PORTWxDISPLAY7S_Q6 		PORTA
    #define PORTRxDISPLAY7S_Q6 		PINA
    #define CONFIGIOxDISPLAY7S_Q6 	DDRA
    #define PINxDISPLAY7S_Q6		5

	#define PORTWxDISPLAY7S_Q7 		PORTA
    #define PORTRxDISPLAY7S_Q7 		PINA
    #define CONFIGIOxDISPLAY7S_Q7 	DDRA
    #define PINxDISPLAY7S_Q7		4

	void disp7s_init(void);
	extern unsigned char disp7s_data_array[DISP7S_TOTAL_NUMMAX];

#endif /* DISPLAY7S_SETUP_H_ */
