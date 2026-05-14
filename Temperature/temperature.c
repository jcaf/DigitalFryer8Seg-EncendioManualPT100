/*
 * temperature.c  (OPTIMIZED Q8.8 VERSION)
 *
 * Author: jcaf
 * Optimized: fixed-point Q8.8, no loss of precision
 */

#include "../main.h"
#include "../MAX6675/MAX6675.h"
#include "temperature.h"
#include "../psmode_program.h"
#include "../disp7s_applevel.h"
#include "../utils/utils.h"
#include "../disp7s_applevel.h"
#include "../adc/adc.h"
#include <stdint.h>

/* ============================================================
   GLOBALS (legacy compatibility)
   ============================================================ */
int TCtemperature;

/////////////////////////////////////////////////////////////////////////
/// /////////////////////////////////////////////////////////////////////////

const uint16_t C_rtd_q8_8[] = {0,225,450,675,900,1125,1351,1576,1801,2027,2252,2478,2703,2929,3155,3380,3606,3832,4058,4284,4510,4736,4963,5189,5415,5641,5868,6094,6321,6548,6774,7001,7228,7455,7682,7908,8136,8363,8590,8817,9044,9272,9499,9726,9954,10181,10409,10637,10865,11092,11320,11548,11776,12004,12232,12461,12689,12917,13145,13374,13602,13831,14059,14288,14517,14746,14975,15203,15432,15661,15891,16120,16349,16578,16808,17037,17266,17496,17726,17955,18185,18415,18645,18874,19104,19334,19565,19795,20025,20255,20486,20716,20946,21177,21408,21638,21869,22100,22331,22561,22792,23023,23255,23486,23717,23948,24180,24411,24642,24874,25106,25337,25569,25801,26033,26265,26497,26729,26961,27193,27425,27657,27890,28122,28355,28587,28820,29053,29285,29518,29751,29984,30217,30450,30683,30917,31150,31383,31617,31850,32084,32317,32551,32785,33019,33252,33486,33720,33954,34189,34423,34657,34891,35126,35360,35595,35829,36064,36299,36534,36769,37003,37238,37473,37709,37944,38179,38414,38650,38885,39121,39356,39592,39828,40063,40299,40535,40771,41007,41243,41480,41716,41952,42189,42425,42662,42898,43135,43372,43609,43845,44082,44319,44557,44794,45031,45268,45506,45743,45980,46218,46456,46693,46931,47169,47407,47645,47883,48121,48359,48598,48836,49074,49313,49551,49790,50029,50267,50506,50745,50984,51223,51462,51701,51940,52180,52419,52659,52898,53138,53377,53617,53857,54097,54337,54577,54817,55057,55297,55537,55778,56018,56259,56499,56740,56981,57222,57463,57703,57944,58185,58427,58668,58909,59151,59392};

/*
 * K = 5 / (1023 * G * Iref) ≈ 0.097948
 * RSEG = (187.564 - 100) / 255 = 0.343389
 *
 * C = (K * 256) / RSEG ≈ 73.03
 * C en Q8.8 → 73.03 * 256 ≈ 18700
 */
#define ADC_TO_SEG_Q8_8   18700UL

static inline uint16_t adc_to_rseg_q8_8(uint16_t adc)
{
    uint32_t tmp;

    // tmp = adc * C_ADC_TO_SEG (Q8.16)
    tmp = (uint32_t)adc * ADC_TO_SEG_Q8_8;

    // Redondeo y paso a Q8.8
    tmp = (tmp + 128) >> 8;

    if (tmp > (255UL << 8))
        tmp = (255UL << 8);

    return (uint16_t)tmp;
}


static inline uint16_t T_rtd_from_rseg_q8_8(uint16_t r_q8_8)
{
      // CASO EXACTO: final de tabla
    if (r_q8_8 >= (255U << 8))
        return C_rtd_q8_8[255];

    uint16_t i;
    uint8_t  frac;
    uint16_t t0, t1;

    i    = r_q8_8 >> 8;     // segmento
    frac = r_q8_8 & 0xFF;   // α (Q0.8)

    if (i > 254)
        i = 254;

    // aquí i SIEMPRE será 0..254
    t0 = C_rtd_q8_8[i];
    t1 = C_rtd_q8_8[i + 1];

    return t0 + (((uint32_t)(t1 - t0) * frac) >> 8);
}

uint16_t T_rtd_from_adc_q8_8(uint16_t adc)
{
    uint16_t r_q8_8;
    r_q8_8 = adc_to_rseg_q8_8(adc);
    return T_rtd_from_rseg_q8_8(r_q8_8);
}
////////////////////////////////////////////////////////////////////////
//Un detalle sobre el Filtro EMA
//Si sientes que el valor tarda en llegar al real al encenderse (por el acumulador del EMA), podrías "forzar" el valor inicial:
//Esto hará que la primera lectura válida sea instantánea en lugar de ver cómo la temperatura sube lentamente desde 0 hasta el valor real.
////////////////////////////////////////////////////////////////////////

//#define AVG_WINDOW 8
/*
 * el AVG_WINDOWS para 16Mhz tiene que ser de 8 para arriba, porque si no se muestra muy cambiante
 */
#define AVG_WINDOW_SHIFT_POT2 2
#define AVG_WINDOW (1<<AVG_WINDOW_SHIFT_POT2)
/////////////////////////////////////////////////////////////////
#define EMA_SHIFT_FAST  1   // α = 1/2  (muy rápido), no puede ser 0
#define EMA_SHIFT_MED   2   // α = 1/4
#define EMA_SHIFT_SLOW  3//4   // α = 1/16 (muy estable)
/////////////////////////////////////////////////////////////////
#define THRESH_FAST  2     // cambio grande
#define THRESH_MED   1      // cambio medio

 //con AVG_WINDOW
uint16_t adc_filter_1s(uint16_t adc_sample)
{
    static uint32_t acc = 0;
    static uint8_t count = 0;
    static int32_t ema = 0;
    static uint8_t initialized = 0;

    acc += adc_sample;
    count++;

    // Promedio parcial durante arranque
    if (count < AVG_WINDOW)
    {
        return (uint16_t)(acc / count);
        //return ema;
    }

    //uint16_t avg = acc >> 3;
    uint16_t avg = acc >> AVG_WINDOW_SHIFT_POT2;

    acc = 0;
    count = 0;

    if (!initialized)
    {
        ema = avg;
        initialized = 1;
        return (uint16_t)ema;
    }

    // velocidad de cambio
    int32_t diff = (int32_t)avg - ema;
    int32_t abs_diff = (diff >= 0) ? diff : -diff;

    uint8_t shift;

    if (abs_diff > THRESH_FAST)//5
    {
        shift = EMA_SHIFT_FAST;   // seguir rápido
    }
    else if (abs_diff > THRESH_MED)//5 4 3
    {
        shift = EMA_SHIFT_MED;    // intermedio
    }
    else
    {
        shift = EMA_SHIFT_SLOW;   // filtrar fuerte
    }
    int32_t round_val = (1 << (shift - 1)); // Redondeo dinámico: 0.5 para el shift actual

    if (diff > 0)
		ema += (diff + round_val) >> shift;
	else if (diff < 0)
		ema += (diff - round_val) >> shift;

    return (uint16_t)ema;
}
/*
//sin AVG_WINDOW
uint16_t adc_filter_1s(uint16_t sample)
{
    static int32_t ema = 0;
    static uint8_t initialized = 0;

    if (!initialized)
    {
        ema = sample;
        initialized = 1;
        return sample;
    }

    int32_t diff = (int32_t)sample - ema;
    int32_t abs_diff = (diff >= 0) ? diff : -diff;

    uint8_t shift;

	if (abs_diff > THRESH_FAST)//5
   {
	   shift = EMA_SHIFT_FAST;   // seguir rápido
   }
   else if (abs_diff > THRESH_MED)//5 4 3
   {
	   shift = EMA_SHIFT_MED;    // intermedio
   }
   else
   {
	   shift = EMA_SHIFT_SLOW;   // filtrar fuerte
   }
   int32_t round_val = (1 << (shift - 1)); // Redondeo dinámico: 0.5 para el shift actual

   if (diff > 0)
	   ema += (diff + round_val) >> shift;
   else if (diff < 0)
	   ema += (diff - round_val) >> shift;

   return (uint16_t)ema;
}
*/
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
/*
 * Esto es correcto y muy eficiente para el modo Free Running porque:

No detienes el flujo del programa principal esperando conversiones (el ADC trabaja "en segundo plano").

Al leer ADCL primero, el hardware del AVR bloquea los registros para que el valor de 10 bits sea consistente (evitas que la parte alta sea de una muestra y la baja de otra).
 */
static inline uint16_t adc_read16(void)
{
    uint8_t l = ADCL;
    return ((uint16_t)ADCH << 8) | l;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//NUEVO FIX PARA 16MHz
static inline uint16_t adc_read_blocking(void)//solo cuando es SINGLE CONVERSION, NO FREE RUNNING
{
	// iniciar conversión
	ADCSRA |= (1 << ADSC);

	// esperar a que termine
	//while (ADCSRA & (1 << ADSC));
	//wait for conversion to finish

	while(!(ADCSRA & (1 << ADIF)))
	{;}
	ADCSRA |= (1 << ADIF); //reset as required
	//

	// leer (orden correcto: primero ADCL)
	uint8_t l = ADCL;
	uint8_t h = ADCH;

	return ((uint16_t)h << 8) | l;
}

/* ============================================================
   TEMPERATURE JOB
   ============================================================ */
//#define GAIN_Q8_8   ((uint32_t)(1.07f*256))//267   //1ra tarjeta
//#define GAIN_Q8_8   ((uint32_t)(1.06f*256))//267   //2da tarjeta
//#define GAIN_Q8_8   ((uint32_t)(1.06f*256))//267   //3ra tarjeta
//#define GAIN_Q8_8   ((uint32_t)(1.05f*256))//267   //4ta tarjeta
#define GAIN_Q8_8   ((uint32_t)(1.03f*256))//267   //5ta tarjeta

static int8_t i_avg;
int8_t temperature_job(void)
{
    //
    uint16_t raw = adc_read16();
    //uint16_t raw = adc_read_blocking();
    uint16_t ADCHL_filtered = adc_filter_1s(raw);
    //
    uint16_t r_q8_8 = adc_to_rseg_q8_8(ADCHL_filtered);
    uint32_t T_q8_8 =  T_rtd_from_rseg_q8_8(r_q8_8);//fix bug 28/4/2026
    //
    // si quieres °C enteros:
    //uint16_t T_C = T_q8_8 >> 8;
    // si quieres decimal:
    //uint16_t T_dec = (T_q8_8 & 0xFF) * 100 / 256;

    //T_C = (float)T_C * 1.042f;
    // Corrección de ganancia (1.042 ≈ 267/256)

    // 1. Aplicar Ganancia: (Q8.8 * Q8.8) -> Q16.16, luego >> 8 para volver a Q16.8
	// Usamos uint32_t para evitar el overflow de la parte entera momentáneamente
	//  Aplicar Ganancia (Mantiene Q8.8)
	// (Q8.8 * Q8.8) >> 8 = Q16.8 (24 bits enteros, 8 decimales)

	// Remedio real para la precisión:

    //T_q8_8 = ((uint32_t)T_q8_8 * GAIN_Q8_8) >> 8;
    T_q8_8 = ( ((uint32_t)T_q8_8 * GAIN_Q8_8) +128) >> 8;// +128 es 0.5 en Q8.8 para redondear

//T_q8_8+= (100<<8);

    if (pgrmode.bf.unitTemperature == FAHRENHEIT)//T_C = ((float)T_C *1.8f) + 32;
    {
    	//T_q8_8 = ((uint32_t)T_q8_8 * 461)>>8; // 461/256
		//T_q8_8 += (32 << 8);

		//T_C = ((float)T_C *1.8f) + 32;
		// 3. Convertir a Fahrenheit (Mantiene Q8.8)
		// Usamos 461 como 1.8 * 256.
		// (T_c_q8.8 * 461) >> 8 sigue siendo Qx.8
		T_q8_8 = (((uint32_t)T_q8_8 * 461) +128)>>8; // 461/256
		T_q8_8 += (32 << 8);

    }

    // Extracción del entero final
	// T_q8_8 ahora puede valer hasta ~115200 (450 << 8)
	// 4. Solo al final, pasar a la variable entera de visualización
    //uint16_t T_C = T_q8_8 >> 8;	//aqui tenemos el valor a final a publicar
    uint16_t T_C = (T_q8_8+128) >> 8;	//aqui tenemos el valor a final a publicar


    if (++i_avg >= AVG_WINDOW)
    {
    	i_avg = 0;
    	TCtemperature = T_C;
    	return 1;
    }
    else
    {
    	return 0;
    }

}

/*
 * int8_t temperature_job(void)
{
    //
    uint16_t raw = adc_read16();
    uint16_t ADCHL_filtered = adc_filter_1s(raw);
    //
    uint16_t r_q8_8 = adc_to_rseg_q8_8(ADCHL_filtered);
    uint16_t T_q8_8 =  T_rtd_from_rseg_q8_8(r_q8_8);
    //
    // si quieres °C enteros:
    uint16_t T_C = T_q8_8 >> 8;
    // si quieres decimal:
    //uint16_t T_dec = (T_q8_8 & 0xFF) * 100 / 256;

    T_C = (float)T_C * 1.042f;


    if (pgrmode.bf.unitTemperature == FAHRENHEIT)
    {
    	T_C = ((float)T_C *1.8f) + 32;
    }

    if (++i_avg >= AVG_WINDOW)
    {
    	i_avg = 0;
    	TCtemperature = T_C;
    	return 1;
    }
    else
    {
    	return 0;
    }

}
 */

void temperature_format_temperature_3digits(int16_t temperatura, unsigned char *str_out)
{
    //1. clear the basket display
    disp7s_blank_displays(str_out, 0, BASKET_DISP_MAX_CHARS_PERBASKET);

    if (temperatura == MAX6675_THERMOCOUPLED_OPEN)
    {
        str_out[0] = D7S_DATA_n | (1<< D7S_DP);
        str_out[1] = D7S_DATA_c;
    }
    else if (temperatura>999)
    {
        str_out[0] = D7S_DATA_o;
        str_out[1] = D7S_DATA_u;
        str_out[2] = D7S_DATA_t;
    }
    else//numerical
    {
        integer_to_arraybcd_msb_lsb_paddingleft_blank(temperatura, str_out, BASKET_DISP_MAX_CHARS_PERBASKET-1 );
        str_out[BASKET_DISP_MAX_CHARS_PERBASKET-1] = D7S_DATA_GRADE_CENTIGRADE;
    }
    //fix right basket: upsidedown displays
    disp7s_fix_upsidedown_display(&str_out[2]);
}
