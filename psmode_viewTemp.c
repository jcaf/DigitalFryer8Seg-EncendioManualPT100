/*
 * psmode_viewTemp.c
 *
 *  Created on: Apr 7, 2022
 *      Author: jcaf
 */

#include "main.h"
#include "psmode_program.h"
#include "Temperature/temperature.h"
#include "indicator/indicator.h"
#include "disp7s_applevel.h"

int8_t psmode_viewTemp(void)
{
	char codret = 0;
	unsigned char str[10];

	//fryer.ps_program.sm0 avanza por la tecla e internamente
	if (fryer.ps_viewTemp.sm0 == 0)
	{
		struct _key_prop key_prop = { 0 };
		key_prop = propEmpty;
		key_prop.uFlag.f.onKeyPressed = 1;
		for (int i=0; i<BASKET_MAXSIZE; i++)
		{
			ikb_setKeyProp(fryer.basket[i].kb.program ,key_prop);//
		}
		//lcdan_clear();
		fryer.ps_viewTemp.sm0++;
	}
	else if (fryer.ps_viewTemp.sm0 == 1)
	{
		disp7s_update_data_array(DIPS7S_MSG_OIL, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
		fryer.ps_viewTemp.sm0++;
	}
	else if (fryer.ps_viewTemp.sm0 == 2)
	{
		MAX6675_formatText3dig(TCtemperature, str);
		disp7s_update_data_array(str, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
	}
	else if (fryer.ps_viewTemp.sm0 == 3)
	{
		if (fryer.ps_viewTemp.sm0 == 3)//EXIT
		{
			codret = 1;
		}
	}

	//al ultimo q evalue KB PROGRAM
	if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
	{
		ikb_key_was_read(KB_LYOUT_PROGRAM);

		fryer.ps_viewTemp.sm0++;
		//
		indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
		indicatorTimed_run();
	}

	return codret;
}
