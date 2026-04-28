/*
 * termopile.c
 *
 *  Created on: 20 feb. 2025
 *      Author: NEFER
 */
#include "../main.h"
#include "termopile.h"
#include "../error/error.h"
#include "../smoothAlg/smoothAlg.h"

#define TERMOPILE_SMOOTHALG_MAXSIZE 30//60// 8
static uint16_t smoothVector[TERMOPILE_SMOOTHALG_MAXSIZE];

struct _smoothAlg smoothAlg_termopile;

////////////////////////////////////////////
#define TERMOPILA_VOLTAGE 0.140f//0.6V
#define ADC_VOLTAGE_REFERENCE 2.56f
#define TERMOPILA_ADC_VALUE	((1023.0f*TERMOPILA_VOLTAGE)/ADC_VOLTAGE_REFERENCE)

struct _termopila termopila;
const struct _termopila termopilaReset;


static int8_t AdqAccSamples(void)
{
	uint8_t adclow = ADCL;
	uint16_t adc16 = (((uint16_t)ADCH)<<8) + adclow;

	smoothVector[job_captureTermopile.counter0] = adc16;

	if (++job_captureTermopile.counter0 >= TERMOPILE_SMOOTHALG_MAXSIZE)
	{
		job_captureTermopile.counter0 = 0x00;
		return 1;
	}
	return 0;
}


int8_t termopile_job(void)
{
	int8_t codret = 0;

	float smoothAnswer = 0;

	if (termopila.sm0 == 0)
	{
		if (AdqAccSamples() )
		{
			termopila.sm0++;
		}
	}
	else
	{
		if (smoothAlg_nonblock(&smoothAlg_termopile, smoothVector, TERMOPILE_SMOOTHALG_MAXSIZE, &smoothAnswer))
		{
			if ( (uint16_t)smoothAnswer < (uint16_t)TERMOPILA_ADC_VALUE )
			{
				termopila.error_counter++;
			}

			//
			if (++termopila.counter0 >= 2)
			{
				termopila.counter0 = 0;

				if (termopila.error_counter >=2)
				{
					//main_schedule.bf.status_thermopile = STATUS_THERMOPILE_BAD;
					e.sensor[ERROR_IDX_THERMOPILE].code = 1;//ERROR

				}
				else
				{
					//main_schedule.bf.status_thermopile = STATUS_THERMOPILE_OK;
					e.sensor[ERROR_IDX_THERMOPILE].code = 0;//NO ERROR

				}

				termopila.error_counter = 0;
			}
			//
			termopila.sm0 = 0x00;
			codret = 1;	//fin del proceso
		}
	}
	return codret;
}

/*
#define ALPHA_Q15   12879   // α ≈ 0.393  → T95 ≈ 300 ms con Ts = 50 ms

static uint16_t filter_EMA(uint16_t x)
{
    // convertir estado filtrado anterior de Q15 a entero ADC
    int32_t y_prev = termopila.emaQ15 >> 15;

    // diferencia entre nueva muestra y filtrado anterior
    int32_t diff = (int32_t)x - y_prev;

    // actualización en Q15 (el núcleo del filtro)
    termopila.emaQ15 += diff * (int32_t)ALPHA_Q15;

    // devolver filtrado en entero ADC
    return (uint16_t)(termopila.emaQ15 >> 15);
}
*/
/*
#define ALPHA_Q15  25441  // α ≈ 0.7764

static uint16_t filter_EMA(uint16_t x)
{
    // valor filtrado anterior en entero (Q15 → entero)
    int32_t y_prev = termopila.emaQ15 >> 15;

    // diferencia entre nueva muestra y valor filtrado previo
    int32_t diff = (int32_t)x - y_prev;

    // yQ15 = yQ15 + alpha * diff
    termopila.emaQ15 += diff * (int32_t)ALPHA_Q15;

    // devolver valor filtrado como entero ADC
    return (uint16_t)(termopila.emaQ15 >> 15);
}
int8_t termopile_job(void)
{
    int8_t codret = 0;


    // 1) Leer ADC crudo
    uint8_t adclow = ADCL;
    uint16_t adc16 = (((uint16_t)ADCH) << 8) + adclow;

    // 2) Suavizado profesional (EMA)
    uint16_t smoothValue = filter_EMA(adc16);

    // 3) Comparación con umbral
    if (smoothValue < (uint16_t)TERMOPILA_ADC_VALUE)
    {
        termopila.error_counter++;
    }

    // 4) Tomar decisión cada 2 muestras (100 ms)
    if (++termopila.counter0 >= 1)//2)
    {
        termopila.counter0 = 0;

        if (termopila.error_counter >= 1)//2)
        {
            // ERROR
            e.sensor[ERROR_IDX_THERMOPILE].code = 1;
        }
        else
        {
            // OK
            e.sensor[ERROR_IDX_THERMOPILE].code = 0;
        }

        termopila.error_counter = 0;
    }

    codret = 1;
    return codret;
}


*/
