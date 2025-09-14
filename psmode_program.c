/*
 * psmode_program.c
 *
 *  Created on: Aug 24, 2021
 *      Author: jcaf
 */
#include "main.h"
#include "psmode_program.h"
#include "psmode_operative.h"
#include "Temperature/temperature.h"
#include "indicator/indicator.h"
#include "disp7s_applevel.h"

#define PSMODE_PROGRAM_BLINK_TIMER_KMAX (400/SYSTICK_MS)//Xms/10ms de acceso


struct _pgrmmode pgrmode;



/* Farenheiths*/
struct _Tcoccion EEMEM TMPRTURE_COCCION =
{
		.TC = TMPRTURE_COCCION_FARENHEIT_STD,
		.max = TMPRTURE_COCCION_FARENHEIT_MAX,
		.min = TMPRTURE_COCCION_FARENHEIT_MIN
};
struct _Tcoccion tmprture_coccion;

int blinkIsActive=0;



int8_t psmode_program(void)
{
	char codret = 0;
	unsigned char str[10];
	//
	static struct _blink blink;
	blink_set(&blink);
	//

	//fryer.ps_program.sm0 avanza por la tecla e internamente
	if (fryer.ps_program.sm0 == 0)
	{
		struct _key_prop key_prop = { 0 };
		key_prop = propEmpty;
		key_prop.uFlag.f.onKeyPressed = 1;
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			ikb_setKeyProp(fryer.basket[i].kb.program ,key_prop);//
		}
//		lcdan_clear();

		fryer.ps_program.sm0++;
		//
		blink.timerBlink_K = PSMODE_PROGRAM_BLINK_TIMER_KMAX;

	}
	else if (fryer.ps_program.sm0 == 1)
	{
//		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
//		lcdan_print_string("OIL  ");
		disp7s_update_data_array(DIPS7S_MSG_OIL, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
		//
//		lcdan_set_cursor(0x0E, 0);//PRINT SYMBOL DEGREE
//		lcdan_write_data(0b11011111);//

		if (pgrmode.bf.unitTemperature == FAHRENHEIT)
		{
//			lcdan_write_data('F');
		}
		else
		{
//			lcdan_write_data('C');
		}
		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 2)
	{
//		MAX6675_formatText3dig(TCtemperature, str);
//		lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
//		lcdan_print_string(str);

		MAX6675_formatText3dig(TCtemperature, str);
		disp7s_update_data_array(str, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);

	}
	else if (fryer.ps_program.sm0 == 3)
	{
//		lcdan_set_cursor(DISP_CURSOR_BASKETLEFT_START_X, 0);
//		lcdan_print_string("SET  ");
		disp7s_update_data_array(DIPS7S_MSG_SET, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);

		fryer.ps_program.sm0++;
	}
	else if (fryer.ps_program.sm0 == 4)
	{
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			if (ikb_key_is_ready2read(fryer.basket[i].kb.down))
			{
				ikb_key_was_read(fryer.basket[i].kb.down);
				//
				if (!ikb_inReptt(fryer.basket[i].kb.down))
				{
					indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
					indicatorTimed_run();
				}
				//

				blink_reset(BLINK_TOGGLE_SET_TEXT);

				if (--tmprture_coccion.TC <= tmprture_coccion.min)
				{
					tmprture_coccion.TC = tmprture_coccion.min;
				}
			}
			if (ikb_key_is_ready2read(fryer.basket[i].kb.up))
			{
				ikb_key_was_read(fryer.basket[i].kb.up);
				//
				if (!ikb_inReptt(fryer.basket[i].kb.up))
				{
					indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
					indicatorTimed_run();
				}
				//

				blink_reset(BLINK_TOGGLE_SET_TEXT);

				if (++tmprture_coccion.TC >= tmprture_coccion.max)
				{
					tmprture_coccion.TC = tmprture_coccion.max;
				}
			}
		}
	}

	//--------------------------------------------
	if (blinkIsActive)
	{
		if (blink.bf.update)
		{
			blink.bf.update = 0;
			//
			if (blink.bf.toggle == BLINK_TOGGLE_SET_BLANK)
			{
//				strcpy(str,"   ");
				disp7s_blank_displays(str,0,BASKET_DISP_MAX_CHARS_PERBASKET);
			}
			else
			{
//				MAX6675_formatText3dig(tmprture_coccion.TC, str);
				MAX6675_formatText3dig(tmprture_coccion.TC, str);
			}
//			lcdan_set_cursor(DISP_CURSOR_BASKETRIGHT_START_X, 0);
//			lcdan_print_string(str);

			disp7s_update_data_array(str, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);

		}
	}


	//al ultimo q evalue KB PROGRAM
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		fryer.ps_program.sm0++;
		//
		if (fryer.ps_program.sm0 == 3)
		{
			//blink.timerBlink_K = PSMODE_PROGRAM_BLINK_TIMER_KMAX;
			//blink_set(&blink);
			blinkIsActive = 1;
		}
		else if (fryer.ps_program.sm0 == 5)//EXIT
		{
			codret = 1;

			eeprom_update_block((struct _Tcoccion *)&tmprture_coccion , (struct _Tcoccion *)&TMPRTURE_COCCION, sizeof(struct _Tcoccion) );

			blinkIsActive = 0;
		}
		indicatorTimed_setKSysTickTime_ms(80/SYSTICK_MS);
		indicatorTimed_run();
	}
	//
	//+++++++++++++++++++++++++++++++++++++++++++++++++
	if (mainflag.sysTickMs)
	{
		blink_timing();
	}
	//+++++++++++++++++++++++++++++++++++++++++++++++++

	return codret;
}
