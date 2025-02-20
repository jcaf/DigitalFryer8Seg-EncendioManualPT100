/*
 * temperature.c
 *
 *  Created on: Dec 3, 2020
 *      Author: jcaf
 */

#include "../main.h"
#include "../MAX6675/MAX6675.h"
#include "../smoothAlg/smoothAlg.h"
#include "temperature.h"
#include "../psmode_program.h"
#include "../disp7s_applevel.h"
#include "../utils/utils.h"
#include "../disp7s_applevel.h"
#include "../adc/adc.h"

int TCtemperature;// = 30;

/*
 * LM334-3 + 68 OHM PRECISION comprados en HIFI
 */
#define INA326_G 49.9f
#define REF200_I 0.001f
#define GxI 0.05f//(INA326_G*REF200_I)
#define INA326_R_OPPOSITE 100.0f//OHMS
/*
G*(Rpt100*Iref - INA326_R_OPPOSITE*Iref) = Voltaje_ADC_uC
despejando Rpt100...

R = Voltaje_ADC_uC/(G*Iref) + INA326_R_OPPOSITE

Por otro lado tenemos que Voltaje_ADC_uC es :
[ADCH:ADCL] * (5V/1023)

Entonces la ecuacion es la siguiente

Rpt100 = [ADCH:ADCL]*(5/(1023*G*Iref)) + INA326_R_OPPOSITE

Resolviendo tenemos
octave:1> 5/(1023*49.9*0.001)
ans = 0.097948

Rpt100 = [ADCH:ADCL]*(0.097948) + INA326_R_OPPOSITE
*/

#include <stdio.h>
#include <stdlib.h>


//**************************************************************
// File         : RTDpwl.c
// Author       : Automatically generated using 'coefRTD.exe'
// Compiler     : intended for Keil C51
// Description  : Subroutines for linearization of RTD signals
//                using piecewise linear approximation method.
// More Info    : Details in application note AN-709, available
//                at....  http://www.analog.com/MicroConverter
//**************************************************************

// definitons....
#define TMIN (0)  // = minimum temperature in degC
#define TMAX (232.0)  // = maximum temperature in degC
#define RMIN (100.0)  // = input resistance in ohms at 0 degC
#define RMAX (187.564f)  // = input resistance in ohms at 232 degC
#define NSEG 255  // = number of sections in table
#define RSEG 0.343389f  // = (RMAX-RMIN)/NSEG = resistance RSEG in ohms of each segment

// lookup table....
//const float C_rtd[] PROGMEM = {-1.42214e-05,0.878715,1.75767,2.63686,3.51627,4.39592,5.27579,6.15589,7.03622,7.91678,8.79757,9.67859,10.5598,11.4413,12.323,13.205,14.0871,14.9696,15.8522,16.7351,17.6182,18.5015,19.385,20.2688,21.1529,22.0371,22.9216,23.8063,24.6913,25.5765,26.4619,27.3476,28.2335,29.1196,30.0059,30.8925,31.7794,32.6664,33.5537,34.4412,35.329,36.217,37.1053,37.9937,38.8824,39.7714,40.6606,41.55,42.4396,43.3295,44.2197,45.11,46.0006,46.8915,47.7826,48.6739,49.5654,50.4572,51.3493,52.2415,53.1341,54.0268,54.9198,55.813,56.7065,57.6002,58.4942,59.3883,60.2828,61.1774,62.0724,62.9675,63.8629,64.7586,65.6544,66.5506,67.4469,68.3435,69.2404,70.1375,71.0348,71.9324,72.8302,73.7282,74.6266,75.5251,76.4239,77.3229,78.2222,79.1217,80.0215,80.9215,81.8218,82.7223,83.6231,84.5241,85.4253,86.3268,87.2286,88.1305,89.0328,89.9353,90.838,91.741,92.6442,93.5477,94.4514,95.3553,96.2596,97.164,98.0687,98.9737,99.8789,100.784,101.69,102.596,103.502,104.409,105.315,106.222,107.13,108.037,108.945,109.853,110.761,111.669,112.578,113.487,114.396,115.306,116.216,117.126,118.036,118.946,119.857,120.768,121.679,122.591,123.503,124.415,125.327,126.24,127.152,128.065,128.979,129.892,130.806,131.72,132.634,133.549,134.464,135.379,136.294,137.21,138.126,139.042,139.958,140.875,141.792,142.709,143.627,144.544,145.462,146.38,147.299,148.218,149.137,150.056,150.975,151.895,152.815,153.735,154.656,155.577,156.498,157.419,158.341,159.263,160.185,161.107,162.03,162.953,163.876,164.8,165.723,166.647,167.572,168.496,169.421,170.346,171.271,172.197,173.123,174.049,174.975,175.902,176.829,177.756,178.683,179.611,180.539,181.467,182.396,183.325,184.254,185.183,186.113,187.043,187.973,188.903,189.834,190.765,191.696,192.628,193.56,194.492,195.424,196.357,197.289,198.223,199.156,200.09,201.024,201.958,202.892,203.827,204.762,205.698,206.633,207.569,208.505,209.442,210.379,211.316,212.253,213.19,214.128,215.066,216.005,216.943,217.882,218.822,219.761,220.701,221.641,222.581,223.522,224.463,225.404,226.345,227.287,228.229,229.171,230.114,231.057,232};
const float C_rtd[]= {-1.42214e-05,0.878715,1.75767,2.63686,3.51627,4.39592,5.27579,6.15589,7.03622,7.91678,8.79757,9.67859,10.5598,11.4413,12.323,13.205,14.0871,14.9696,15.8522,16.7351,17.6182,18.5015,19.385,20.2688,21.1529,22.0371,22.9216,23.8063,24.6913,25.5765,26.4619,27.3476,28.2335,29.1196,30.0059,30.8925,31.7794,32.6664,33.5537,34.4412,35.329,36.217,37.1053,37.9937,38.8824,39.7714,40.6606,41.55,42.4396,43.3295,44.2197,45.11,46.0006,46.8915,47.7826,48.6739,49.5654,50.4572,51.3493,52.2415,53.1341,54.0268,54.9198,55.813,56.7065,57.6002,58.4942,59.3883,60.2828,61.1774,62.0724,62.9675,63.8629,64.7586,65.6544,66.5506,67.4469,68.3435,69.2404,70.1375,71.0348,71.9324,72.8302,73.7282,74.6266,75.5251,76.4239,77.3229,78.2222,79.1217,80.0215,80.9215,81.8218,82.7223,83.6231,84.5241,85.4253,86.3268,87.2286,88.1305,89.0328,89.9353,90.838,91.741,92.6442,93.5477,94.4514,95.3553,96.2596,97.164,98.0687,98.9737,99.8789,100.784,101.69,102.596,103.502,104.409,105.315,106.222,107.13,108.037,108.945,109.853,110.761,111.669,112.578,113.487,114.396,115.306,116.216,117.126,118.036,118.946,119.857,120.768,121.679,122.591,123.503,124.415,125.327,126.24,127.152,128.065,128.979,129.892,130.806,131.72,132.634,133.549,134.464,135.379,136.294,137.21,138.126,139.042,139.958,140.875,141.792,142.709,143.627,144.544,145.462,146.38,147.299,148.218,149.137,150.056,150.975,151.895,152.815,153.735,154.656,155.577,156.498,157.419,158.341,159.263,160.185,161.107,162.03,162.953,163.876,164.8,165.723,166.647,167.572,168.496,169.421,170.346,171.271,172.197,173.123,174.049,174.975,175.902,176.829,177.756,178.683,179.611,180.539,181.467,182.396,183.325,184.254,185.183,186.113,187.043,187.973,188.903,189.834,190.765,191.696,192.628,193.56,194.492,195.424,196.357,197.289,198.223,199.156,200.09,201.024,201.958,202.892,203.827,204.762,205.698,206.633,207.569,208.505,209.442,210.379,211.316,212.253,213.19,214.128,215.066,216.005,216.943,217.882,218.822,219.761,220.701,221.641,222.581,223.522,224.463,225.404,226.345,227.287,228.229,229.171,230.114,231.057,232};

// lookup table size:
//   = 255 linear sections
//   = 256 coefficients
//   = 1024 bytes (4 bytes per floating point coefficient)

// linearization routine error band:
//   = -2.08933e-05degC .. 2.56428e-05degC
// specified over measurement range 0degC .. 232degC

// _____________________________________________________________
// Temperature of RTD Function                             T_rtd
// input: r = resistance of RTD
// output: T_rtd() = corresponding temperature of RTD
// Calculates temperature of RTD as a function of resistance via
// a piecewise linear approximation method.

float T_rtd (float r)
{
  float t;
  int i;
  i=(r-RMIN)/RSEG;       // determine which coefficients to use
  if (i<0)               // if input is under-range..
    i=0;                 // ..then use lowest coefficients
  else if (i>NSEG-1)     // if input is over-range..
    i=NSEG-1;            // ..then use highest coefficients

  //t = pgm_read_float(C_rtd[i]) + (r-(RMIN+RSEG*i))*(pgm_read_float(C_rtd[i+1])-pgm_read_float(C_rtd[i]))/RSEG;
  t = C_rtd[i]+(r-(RMIN+RSEG*i))*(C_rtd[i+1]-C_rtd[i])/RSEG;

  return (t);
}
/*
// _____________________________________________________________
// Resistance of RTD Function                              R_rtd
// input: t = temperature of RTD
// output: R_rtd() = corresponding resistance of RTD
// Calculates resistance of RTD as a function of temperature via
// a piecewise linear approximation method.

float R_rtd (float t)
{
  float r;
  int i, adder;

  // set up initial values
  i = NSEG/2;           // starting value for 'i' index
  adder = (i+1)/2;      // adder value used in do loop

  // determine if input t is within range
  if (t<C_rtd[0])           // if input is under-range..
    i=0;                    // ..then use lowest coefficients
  else if (t>C_rtd[NSEG])   // if input is over-range..
    i=NSEG-1;               // ..then use highest coefficients

  // if input within range, determine which coefficients to use
  else do
  {
    if (C_rtd[i]>t)   i-=adder; // either decrease i by adder..
    if (C_rtd[i+1]<t) i+=adder; // ..or increase i by adder
    if (i<0)       i=0;         // make sure i is >=0..
    if (i>NSEG-1)  i=NSEG-1;    // ..and <=NSEG-1
    adder = (adder+1)/2;        // divide adder by two (rounded)
  } while ((C_rtd[i]>t)||(C_rtd[i+1]<t));   // repeat 'til done

  // compute final result
  r = RMIN+RSEG*i + (t-C_rtd[i])*RSEG/(C_rtd[i+1]-C_rtd[i]);

  return (r);
}

// _____________________________________________________________
// Minimum Temperature Function                         Tmin_rtd
// Returns minimum temperature specified by lookup table.
float Tmin_rtd ()
{
  return (TMIN);
}

// _____________________________________________________________
// Maximum Temperature Function                         Tmax_rtd
// Returns maximum temperature specified by lookup table.
float Tmax_rtd ()
{
  return (TMAX);
}

// _____________________________________________________________
// Minimum Resistance Function                          Rmin_rtd
// Returns minimum RTD resistance specified by lookup table.
float Rmin_rtd ()
{
  return (RMIN);
}

// _____________________________________________________________
// Maximum Resistance Function                          Rmax_rtd
// Returns maximum RTD resistance specified by lookup table.
float Rmax_rtd ()
{
  return (RMAX);
}
*/


#ifdef MAX6675_UTILS_LCD_PRINT3DIG_C
/*****************************************************
Format with 3 digits 999C
*****************************************************/
void MAX6675_formatText3dig_C(int16_t temper, char *str_out)
{
    char buff[10];

	if (temper == MAX6675_THERMOCOUPLED_OPEN)
	{
		strcpy(str_out,"N.C ");//4posit
        return;
    }
    else
    {
        itoa(temper, buff, 10);//convierte

        //4 positions to display
        strcpy(str_out,"   C");

        if (temper< 10)
        {
            strncpy(&str_out[2], buff, 1);
        }
        else if (temper<100)
        {
            strncpy(&str_out[1], buff, 2);
        }
        else if (temper<1000)
        {
            strncpy(&str_out[0], buff, 3);
        }
        else
        {
            strncpy(&str_out[0], buff, 4);
        }
    }
}
#endif
#ifdef MAX6675_UTILS_LCD_PRINT3DIG
/*****************************************************
Format with 4 digits 999 sin grados ni C
*****************************************************/


void MAX6675_formatText3dig(int16_t temperatura, unsigned char *str_out)
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

		//unsigned char bcd[10];
		//int k = integer_to_arraybcd_msb_lsb(temperatura, bcd);
		//int idx= ((BASKET_DISP_MAX_CHARS_PERBASKET))-1 -k;
		//for (int i = 0; i< k; i++ )
		//{
		//	str_out[idx++] = DISP7_NUMERIC_ARR[bcd[i]];
		//}
		str_out[BASKET_DISP_MAX_CHARS_PERBASKET-1] = D7S_DATA_GRADE_CENTIGRADE;
	}
	//fix right basket: upsidedown displays
	disp7s_fix_upsidedown_display(&str_out[2]);
}
#endif
/*****************************************************

*****************************************************/
#ifdef MAX6675_UTILS_LCD_PRINTCOMPLETE_C
void MAX6675_convertIntTmptr2str_wformatPrintComplete(int16_t temper, char *str_out)
{
    char buff[10];

    if (temper < 0)
    {
        if (temper == -1)
            {strcpy(str_out,"N.C  ");}
        return;
    }
    else
    {
        itoa(temper, buff, 10);

        //5 positions to display: 1023 + C
        strcpy(str_out,"    C");

        if (temper< 10)
        {
            strncpy(&str_out[3], buff, 1);
        }
        else if (temper<100)
        {
            strncpy(&str_out[2], buff, 2);
        }
        else if (temper<1000)
        {
            strncpy(&str_out[1], buff, 3);
        }
        else
        {
            strncpy(&str_out[0], buff, 4);
        }
    }
}
#endif


#define TEMPERATURE_SMOOTHALG_MAXSIZE 30// 8
static uint16_t smoothVector[TEMPERATURE_SMOOTHALG_MAXSIZE];

struct _smoothAlg smoothAlg_temp;
const struct _smoothAlg smoothAlg_reset;

int8_t AdqAccSamples(void)
{
	static int sm0;

	if (sm0 == 0)
	{
		if (mainflag.ADCrecurso == ADC_LIBRE)
		{
			mainflag.ADCrecurso = ADC_OCUPADO;

//			ADC_set_reference(ADC_REF_AVCC);
//			ADC_start_conv(ADC_CH_2);
			//
			sm0++;
//usart_println_string("deja convr");
//__delay_ms(1);

		}
	}
	else
	{
		if (1)//if (isr_flag.adcReady == 1)
		{
			//isr_flag.adcReady = 0;

			mainflag.ADCrecurso = ADC_LIBRE;
			sm0 = 0;
			//
			uint8_t adclow = ADCL;
			uint16_t adc16 = (((uint16_t)ADCH)<<8) + adclow;

			smoothVector[job_captureTemperature.counter0] = adc16;

//usart_println_string("LEE ADCH ACHL");


			if (++job_captureTemperature.counter0 >= TEMPERATURE_SMOOTHALG_MAXSIZE)
			{
				job_captureTemperature.counter0 = 0x00;
//usart_println_string("job_captureT");

				return 1;
			}

		}
	}
	return 0;
}
/*****************************************************
*****************************************************/
int8_t MAX6675_smoothAlg_nonblock_job(int16_t *TCtemperature)
{
	float smoothAnswer;

	if (smoothAlg_nonblock(&smoothAlg_temp, smoothVector, TEMPERATURE_SMOOTHALG_MAXSIZE, &smoothAnswer))
	{
//usart_println_string("smoothAlg_nonblock");

		if (smoothAnswer > 0.0f)
		{
			float Rtd = (smoothAnswer*0.097948f)+ INA326_R_OPPOSITE;
			//Rtd *= 1.011f;//factor de correccion
			Rtd *= 1.005f;//factor de correccion

			*TCtemperature = (int)T_rtd(Rtd);
		}
		else
		{
			*TCtemperature = 0;
		}
		return 1;
	}
	return 0;
}

/*****************************************************
tendria que cambiar la temperature_job para saber cuando tiene correctamente la temperatura
para poder leer al inicio del programa, ojo xq se necesita el flag de systick
*****************************************************/

int8_t temperature_job(void)
{
	char bufferTC[20];

	int8_t codret = 0;
	static int8_t sm0;

//	usart_println_string("+++");

	if (sm0 == 0)
	{
		if (AdqAccSamples() )
		{

			sm0++;
		}
	}
	else
	{
		if (MAX6675_smoothAlg_nonblock_job( &TCtemperature ))
		{
			if (pgrmode.bf.unitTemperature == FAHRENHEIT)
			{
//				TCtemperature = (TCtemperature*1.8f) + 32;//TCtemperature = (TCtemperature*(9.0f/5)) + 32;

//				itoa(TCtemperature,bufferTC,10);
//usart_print_string("T:");
//usart_println_string(bufferTC);

			}
			sm0 = 0x00;

			codret = 1;	//fin del proceso
		}
	}
	return codret;
}
