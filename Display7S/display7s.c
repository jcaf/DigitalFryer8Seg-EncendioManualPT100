/*
 * display7s.c
 * Driver
 *  Created on: Dec 29, 2022
 *      Author: jcaf

 from left to right
 q = 0 1  2 1 4
 *   [][]-[][][]

 */
#include "../system.h"
#include "../types.h"
#include "display7s.h"

static uint8_t disp7s_num_total;
static volatile uint8_t * PORTWxDISPLAY7S_DATA;
static unsigned char * disp7s_data;
static struct _disp7s_mux * disp7s_mux;

void disp7s_setup(uint8_t disp7s_nummax, volatile uint8_t * port,  unsigned char * _disp7s_data_array, struct _disp7s_mux * __disp7s_mux)
{
	disp7s_num_total = disp7s_nummax;
	PORTWxDISPLAY7S_DATA = port;
	disp7s_data = _disp7s_data_array;
	disp7s_mux = __disp7s_mux;
}

//inline static void disp7s_mux_Q(int8_t q)//q=transistor #
//{
//
//    /*
//    //option 1
//
//    for (i=0; i< disp7s_num_total; i++)
//    {
//        if ( (1<<i) & (1<<q) )
//            disp7s_mux[i].Qonoff[1]();//on
//        else
//            disp7s_mux[i].Qonoff[0]();//off
//    }
//    */
//
//    //option 2
//	//int8_t i;
//    /*
//    for (i=0; i< disp7s_num_total; i++)
//        {
//                disp7s_mux[i].Qonoff[0]();//off
//        }
//    disp7s_mux[q].Qonoff[1]();//on
//    */
//
//
//
//    //+-off current Q before to present new digit
///*	int8_t pq=0;
//
//	pq= q-1;
//
//	if (pq<0)
//	{
//		pq = disp7s_num_total-1;
//	}
//	disp7s_mux[pq].Qonoff[0]();//off
//*/
//	//__delay_us(100);
//	//-+
//	disp7s_mux[q].Qonoff[1]();//on
//
//}


void disp7s_job(void)
{
    static int8_t q;//q=transistor #

    //+-off current Q before to present new digit
    int8_t pq = q-1;
    if (pq<0)
    {
        pq = disp7s_num_total-1;
    }
    disp7s_mux[pq].Qonoff[0]();//off
    //-+

    //__delay_us(100);
    *PORTWxDISPLAY7S_DATA = disp7s_data[q];
    //disp7s_mux_Q(q);
    disp7s_mux[q].Qonoff[1]();//on

    //
    if (++q == disp7s_num_total)
        {q=0x0;}
}
