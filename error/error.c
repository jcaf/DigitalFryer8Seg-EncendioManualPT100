/*
 * error.c
 *
 *  Created on: Sep 4, 2023
 *      Author: jcaf
 */

#include "../main.h"
#include "error.h"
#include "../disp7s_applevel.h"
#include "../indicator/indicator.h"
#include "../pid/pid.h"
#include "../Display7S/display7s.h"
#include "../display7s_setup.h"
#include "../disp7s_applevel.h"


struct _error e;
struct _error e_reset;

unsigned char disp7s_data_array_saved[DISP7S_TOTAL_NUMMAX];

void error_job(void)
{
	int8_t counter;
	unsigned char str[10];//cambiar a char_arr

	//+++++++++++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	if (e.f.indicator == 0)
	{
		for (int i=0; i<ERROR_SIZE; i++)
		{
			if (e.sensor[i].code > 0)//existe algun error?
			{
				e.f.indicator = 1;
				//

				indicatorTimed_setKSysTickTime_ms(150/SYSTICK_MS);
				indicatorTimed_cycle_start();
				//
				pid_pwm_set_pin(&mypid0, 0);
				//

				for (int x=0; x<DISP7S_TOTAL_NUMMAX; x++)
				{
					disp7s_data_array_saved[x] = disp7s_data_array[x];
				}

				break;
			}
		}
	}
	else
	{
		counter = 0;
		for (int i=0; i<ERROR_SIZE; i++)
		{
			if (e.sensor[i].code == 0)
			{
				counter++;
			}
		}

		if (counter == ERROR_SIZE)
		{
			indicatorTimed_stop();

			e.f.indicator = 0;

			for (int x=0; x<DISP7S_TOTAL_NUMMAX; x++)
			{
				disp7s_data_array[x] = disp7s_data_array_saved[x];
			}

		}

	}

	//+++++++++++++++++++++++++++++++++++++++++
	//+++++++++++++++++++++++++++++++++++++++++

	if (e.sensor[e.idx].sm0 == 0)
	{
		if (e.sensor[e.idx].code > 0)//hay error
		{
			//[Err  text]
			disp7s_clear_all();
			strncpy(str,DIPS7S_MSG_ERROR,BASKET_DISP_MAX_CHARS_PERBASKET);
			disp7s_update_data_array(str, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
			if (e.idx == ERROR_IDX_THERMOCOUPLE)
			{
				strncpy(str,DIPS7S_MSG_THERMOCOUPLE_NC,BASKET_DISP_MAX_CHARS_PERBASKET);
			}
			else if (e.idx == ERROR_IDX_THERMOPILE)
			{
				strncpy(str,DIPS7S_MSG_THERMOPILE,BASKET_DISP_MAX_CHARS_PERBASKET);
			}
			else if (e.idx == ERROR_IDX_HIGHLIMIT)
			{
				strncpy(str,DIPS7S_MSG_HIGHLIMIT,BASKET_DISP_MAX_CHARS_PERBASKET);
			}
			disp7s_update_data_array(str, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);


			e.sensor[e.idx].sm0++;
			e.timer = 0x00000;
			//
//			indicatorTimed_setKSysTickTime_ms(300/SYSTICK_MS);
//			indicatorTimed_cycle_start();
//			//
//			pid_pwm_set_pin(&mypid0, 0);
		}
		else
		{
			if (++e.idx >= ERROR_SIZE)//avanza a ver el siguiente error
				{e.idx = 0x00;}
		}
	}
	else if (e.sensor[e.idx].sm0 == 1)
	{
		if (e.sensor[e.idx].code == 0)//ya no hay error
		{
			e.sensor[e.idx].sm0 = 0x00;
			//
			//indicatorTimed_stop();
		}
		else
		{
			//timing
			if (mainflag.sysTickMs)
			{
				if (++e.timer >= (1000/SYSTICK_MS))
				{
					e.timer = 0x0;

					e.sensor[e.idx].sm0 = 0;	//dejar habilitado para la siguiente pasada el mismo

					if (++e.idx >= ERROR_SIZE)//avanza a ver el siguiente error
						{e.idx = 0x00;}
				}
			}
			//
		}

	}


}
