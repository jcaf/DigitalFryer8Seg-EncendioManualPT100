/*
 Atmega32 version corregida del ADC
 ultima prog. a tarjeta 13 agosto 2025
 ----------------------------------------
 1)    http://www.engbedded.com/fusecalc/
 lock bits:
 http://eleccelerator.com/fusecalc/fusecalc.php?chip=atmega328p
 2) verificar que responda el atmega (ONLY A RESET)
 [jcaf@jcafpc ~]$ avrdude -c usbasp -v -B5 -p m32

 3) programar fuse (sin preservar EEPROM)
 -U lfuse:w:0xbf:m -U hfuse:w:0xcf:m
 [jcaf@jcafpc ~]$ avrdude -c usbasp -B5 -p m32 -U lfuse:w:0xbf:m -U hfuse:w:0xcf:m

 4) GRABAR EL CODIGO FUENTE CON EL COMANDO ACOSTUMBRADO
 [root@JCAFPC Release]# avrdude -c usbasp -B5 -p m32 -U flash:w:DigitalFryerDisplay8Seg-EncendidoManual.hex
 [root@JCAFPC Release]# avrdude -c usbasp -B1 -p m32 -V -U flash:w:DigitalFryerDisplay8Seg-EncendidoManual.hex (SIN VERIFICAR)
 [jcaf@JCAFPC Release]$ avrdude -c usbasp -B5 -p m32 (ONLY A RESET)

 NUEVO
 [root@JCAFPC Release]# avrdude -c usbasp -B0.3 -p m32 -V -U flash:w:digitalFryer.hex (MAS RAPIDO!)
 Tambien puede ser sin -BX.. cuando ya esta bien configurado los fuses:
 [root@JCAFPC Release]# avrdude -c usbasp -p m32 -U flash:w:digitalFryer.hex

 5)
 8:50 a. m.
 GRABAR LA EEPROM
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m32 -V -U eeprom:w:DigitalFryerDisplay8Seg-EncendidoManual.eep

 6) programar fuse (PRESERVANDO EEPROM)

 avrdude -c usbasp -B5 -p m32 -U lfuse:w:0xbf:m -U hfuse:w:0xc7:m

 7) Verificar los fuses
 [jcaf@jcafpc Release]$ avrdude -c usbasp -B4 -p m32 -U lfuse:r:-:i -v

 +++++++++++++++++++++++
 Acomodar para Atmega32A
 proteger flash (modo 3): lectura y escritura

 avrdude -c usbasp -B10 -p m32 -U lock:w:0xFC:m

 (ignorar el error de 0x3C... pues los 2 bits de mayor peso no estan implentados)
*/

#include "main.h"
#include "pinGetLevel/pinGetLevel.h"
#include "psmode_program.h"
#include "psmode_operative.h"
#include "psmode_viewTemp.h"
#include "indicator/indicator.h"
#include "pid/pid.h"
#include "Display7S/display7s.h"
#include "display7s_setup.h"
#include "disp7s_applevel.h"
#include "adc/adc.h"
#include "error/error.h"
#include "termopile/termopile.h"

#include "usart/usart.h"
#include "serial/serial.h"

volatile struct _isr_flag isr_flag;
struct _mainflag mainflag;

struct _main_schedule main_schedule;
const struct _main_schedule main_schedule_reset;

//struct _error error;
//const struct _error error_reset;

const struct _process ps_reset;
struct _fryer fryer;
const struct _fryer fryer_reset;

struct _job job_captureTemperature;
const struct _job job_reset;

struct _job job_captureTermopile;

//k-load from EEPROM
struct _t EEMEM COOKTIME[BASKET_MAXSIZE]= { {7, 10}, {7,10} };    //mm:ss

/* Declare PID objects*/
struct PID mypid0;

/* Setear constantes para el primer objeto PID*/
/* cada PWM_ACCESS_TIME_MS se lleva el timing del periodo del PWM */
#define PWM_ACCESS_TIME_MS 100 //ms

void mypid0_set(void)
{
	mypid0.pwm.timing.kmax_ticks_ms = PWM_ACCESS_TIME_MS/SYSTICK_MS;//100 ms

	/* tengo que convertir a unidades de tiempo */
	mypid0.algo.scaler_time_ms = 1000.0f/PWM_ACCESS_TIME_MS;	// 1seg/PWM_ACCESS_TIME_MS


	/* aqui es donde realmente se fija el valor */
	/* todo depende practicamente del valor asignado asignado a KP*/
	mypid0.algo.pid_out_max_ms = 10 * mypid0.algo.scaler_time_ms; //10s
	mypid0.algo.pid_out_min_ms = 0 * mypid0.algo.scaler_time_ms;	//0s

	/* El sistema presenta esos delays */
	mypid0.pwm.timing.k_systemdelay_ton_ms = 500.0f / PWM_ACCESS_TIME_MS;
	mypid0.pwm.timing.k_systemdelay_toff_ms = 5000.0f/ PWM_ACCESS_TIME_MS;

	/* PID ktes x algorithm */

	/* 10 es la salida deseada del controlador PID que representa
	 * el valor maximo (mayor o igual a este siempre será acotado a 10)
	 *
	 * +50 es el error SP-PV, a partir de esa diferencia queremos el controlador
	 * empieza a regular entre 10..0 de manera proporcional
	 *  */
	//mypid0.algo.kp = 10.0f / 25;// 5/6/2024 la kte se dobla
	mypid0.algo.kp = 10.0f / 12;// 5/6/2024 la kte se dobla

	//pid.algo.kp = 1.0f/5;

	mypid0.algo.ki = 1.0f/10;
	mypid0.algo.kd = 0;
	mypid0.algo.kei_windup_min = 0;
	mypid0.algo.kei_windup_max = 10;

	mypid0.pwm.io.port = &PORTWxSOL_GAS_QUEMADOR;
	mypid0.pwm.io.pin = PINxSOL_GAS_QUEMADOR;
	//
	//mypid0.algo.sp = 300;
}

/* cada objeto PID es particular y necesita ser afinado antes de pasar al control */
int16_t mypid0_adjust_kei_windup(void)
{
	/* 1. error es target-specific */
	//int16_t pv = TCtemperature;
	//int16_t pv = 110;
	//int16_t error =  mypid0.algo.sp - pv;
	int16_t error =  tmprture_coccion.TC - TCtemperature;

	/* adjust windup for integral error */
	if (error > 5)
	{
		mypid0.algo.kei_windup_max = 10;
	}
	else
	{
		mypid0.algo.kei_windup_max = 1;
	}
	return error;
}

void kbmode_default(struct _kb_basket *kb)
{
	struct _key_prop key_prop = { 0 };
	//
	key_prop = propEmpty;
	//
	key_prop.uFlag.f.onKeyPressed = 1;
	ikb_setKeyProp(kb->sleep,key_prop);
	ikb_setKeyProp(kb->startStop ,key_prop);
	//
	key_prop.uFlag.f.onKeyPressed = 0;
	key_prop.uFlag.f.atTimeExpired2 = 1;
	ikb_setKeyProp(kb->program ,key_prop);//programacion
	//
	key_prop.uFlag.f.atTimeExpired2 = 0;
	//
	key_prop.uFlag.f.onKeyPressed = 1;
	key_prop.uFlag.f.reptt = 1;
	key_prop.numGroup = 0;
	key_prop.repttTh.breakTime = (uint16_t) 200.0 / KB_PERIODIC_ACCESS;
	key_prop.repttTh.period = (uint16_t) 50.0 / KB_PERIODIC_ACCESS;
	ikb_setKeyProp(kb->down ,key_prop);
	ikb_setKeyProp(kb->up ,key_prop);
}
void kbmode_2basket_set_default(void)
{
	for (int i=0; i<BASKET_MAXSIZE; i++)
	{
		kbmode_default(&fryer.basket[i].kb);
	}
}
void fryer_init(void)
{
	fryer = fryer_reset;
	//++--
	/* se usara la etiqueta KB_LYOUT_PROGRAM y no el [i]kb.program, xq es comun para ambos */
	fryer.basket[BASKET_LEFT].kb.startStop = KB_LYOUT_LEFT_STARTSTOP;
	//fryer.basket[BASKET_LEFT].kb.sleep = KB_LYOUT_LEFT_SLEEP;
	fryer.basket[BASKET_LEFT].kb.down = KB_LYOUT_LEFT_DOWN;
	fryer.basket[BASKET_LEFT].kb.up = KB_LYOUT_LEFT_UP;
	fryer.basket[BASKET_LEFT].kb.program = KB_LYOUT_PROGRAM;//comun a ambos

	fryer.basket[BASKET_LEFT].display.cursor.x = BASKETLEFT_DISP_CURSOR_START_X;//0x00;
	fryer.basket[BASKET_LEFT].display.cursor.y = 0x00;

	//
	fryer.basket[BASKET_RIGHT].kb.startStop = KB_LYOUT_RIGHT_STARTSTOP;
	//fryer.basket[BASKET_RIGHT].kb.sleep = KB_LYOUT_RIGHT_SLEEP;
	fryer.basket[BASKET_RIGHT].kb.down = KB_LYOUT_RIGHT_DOWN;
	fryer.basket[BASKET_RIGHT].kb.up = KB_LYOUT_RIGHT_UP;
	fryer.basket[BASKET_RIGHT].kb.program = KB_LYOUT_PROGRAM;//comun a ambos

	fryer.basket[BASKET_RIGHT].display.cursor.x = BASKETRIGHT_DISP_CURSOR_START_X;//0x0B;
	fryer.basket[BASKET_RIGHT].display.cursor.y = 0x00;
	//--++
}
void ADC_config2temperature(void)
{
	//
	ADC_setAutoTrigger_disabled();
	ADC_disable();
	//
	ADC_set_channel(ADC_CH_2);
	ADC_enable();
	ADC_set_prescaler(ADC_PRESCALER_128);
	ADC_set_reference(ADC_REF_AVCC);
	ADC_setAutoTrigger_enabled();
	ADC_setAutoTrigger_source(ADC_AUTOTRIGGER_SOURCE_FREE_RUNNING);
	ADC_setBit_startConversion_On();

}
void ADC_config2termopile(void)
{
	//
	ADC_setAutoTrigger_disabled();
	ADC_disable();
	//
	ADC_set_channel(ADC_CH_0);
	ADC_enable();
	ADC_set_prescaler(ADC_PRESCALER_128);
	ADC_set_reference(ADC_REF_INTERNAL_2_56V);
	ADC_setAutoTrigger_enabled();
	ADC_setAutoTrigger_source(ADC_AUTOTRIGGER_SOURCE_FREE_RUNNING);
	ADC_setBit_startConversion_On();
}

int main(void)
{
	int counter0 = 0;
	int counter1 = 0;
	unsigned char str[10];//cambiar a char_arr
	unsigned char data_array_buffer[DISP7S_TOTAL_NUMMAX];
	int8_t systick_counter0=0;
	int16_t counter_displayACIER=0;

	uint16_t ADCcoordinadorTiempos_timer=0;
	int8_t ADCcoordinadorTiempos_sm0=0;



	disp7s_init();//new

	eeprom_read_block((struct _Tcoccion *)&tmprture_coccion , (struct _Tcoccion *)&TMPRTURE_COCCION, sizeof(struct _Tcoccion) );

	//+-
	pgrmode.bf.unitTemperature = CELSIUS;
	//added 13/09/2025: dejando casi todo listo cuando se va a cambiar entre unidades de Farenheit o Centigrados
	if (pgrmode.bf.unitTemperature == CELSIUS)
	{
		if (tmprture_coccion.max >	TMPRTURE_COCCION_CELCIUS_MAX )
		{
			tmprture_coccion.max = TMPRTURE_COCCION_CELCIUS_MAX;
		}

		if (tmprture_coccion.min <	TMPRTURE_COCCION_CELCIUS_MIN )
		{
			tmprture_coccion.min = TMPRTURE_COCCION_CELCIUS_MIN;
		}

		if (tmprture_coccion.TC > TMPRTURE_COCCION_CELCIUS_MAX )
		{
			tmprture_coccion.TC = TMPRTURE_COCCION_CELCIUS_STD;
		}
		//update
		eeprom_update_block((struct _Tcoccion *)&tmprture_coccion , (struct _Tcoccion *)&TMPRTURE_COCCION, sizeof(struct _Tcoccion) );
	}

	//-+

	fryer_init();

	//Tiempo Necesario para estabilizar la tarjeta
	//__delay_ms(2000);//estabilizar tarjeta de deteccion

	//Active pull-up
	PinTo1(PORTWxKB_KEY0, PINxKB_KEY0);
	PinTo1(PORTWxKB_KEY1, PINxKB_KEY1);
	PinTo1(PORTWxKB_KEY2, PINxKB_KEY2);
	PinTo1(PORTWxKB_KEY3, PINxKB_KEY3);
	PinTo1(PORTWxKB_KEY4, PINxKB_KEY4);
	PinTo1(PORTWxKB_KEY5, PINxKB_KEY5);
	PinTo1(PORTWxKB_KEY6, PINxKB_KEY6);
	__delay_ms(1);
	ikb_init();

	pinGetLevel_init(); //with Changed=flag activated at initialization


	PinTo0(PORTWxSOL_GAS_QUEMADOR, PINxSOL_GAS_QUEMADOR);
	ConfigOutputPin(CONFIGIOxSOL_GAS_QUEMADOR, PINxSOL_GAS_QUEMADOR);

//	ADC_init(ADC_AUTOTRIGGER_SOURCE_FREE_RUNNING, ADC_REF_AVCC, ADC_PRESCALER_128);
//	ADC_set_channel(ADC_CH_2);
//	BitTo1(ADCSRA, ADSC);

	PinTo1(PORTWxHIGHLIMIT,PINxHIGHLIMIT);
	ConfigInputPin(CONFIGIOxHIGHLIMIT, PINxHIGHLIMIT);

	//
	ConfigOutputPin(CONFIGIOxBUZZER, PINxBUZZER);
	indicator_setPortPin(&PORTWxBUZZER, PINxBUZZER);
	indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);

	//With prescaler 64, gets 1 ms exact (OCR0=249)
	TCNT0 = 0x00;
	TCCR0 = (1 << WGM01) | (0 << CS02) | (1 << CS01) | (1 << CS00); //CTC, PRES=64
	OCR0 = CTC_SET_OCR_BYTIME(1e-3, 64); //TMR8-BIT @16MHz @PRES=1024-> BYTIME maximum = 16ms
	TIMSK |= (1 << OCIE0);
	sei();

	mypid0_set();	//1 vez
	disp7s_clear_all();

	strncpy(str,DIPS7S_MSG_ACIE,BASKET_DISP_MAX_CHARS_PERBASKET);
	disp7s_update_data_array(str, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);

	strncpy(str,DIPS7S_MSG_rInd,BASKET_DISP_MAX_CHARS_PERBASKET);
	disp7s_update_data_array(str, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);

	//USART_Init(416);
//	USART_Init(3); //depuracion a 250KBPS

//	while (1)
//	{
//		usart_println_string("abc");
//	}

	while (1)
	{
		if (isr_flag.sysTickMs)
		{
			isr_flag.sysTickMs = 0;
			mainflag.sysTickMs = 1;
		}

		if (mainflag.sysTickMs)
		{
			if (++systick_counter0 >= (1/SYSTICK_MS) )//ms
			{
				systick_counter0 = 0x00;
				disp7s_job();
			}
		}

		if (main_schedule.bf.display_ACIERInd == 0)
		{
			if (mainflag.sysTickMs)
			{
				if (++counter_displayACIER >= (3000/SYSTICK_MS))    //20ms
				{
					counter_displayACIER = 0;
					main_schedule.bf.display_ACIERInd = 1;
				}
			}
		}
		else
		{

			//------------------------------------------
			//usart_println_string("ALT");

//			if (temperature_job())
//			{
//				//usart_println_string("tj");
//
//				main_schedule.bf.startup_finish_stable_temperature = STARTUP_FINISHED;
//				e.sensor[ERROR_IDX_THERMOCOUPLE].code = 0;
//				//main_schedule.bf.status_thermocuple = STATUS_THERMOCOUPLE_OK;
//			}
			//////////////////////////////////////
			if (ADCcoordinadorTiempos_sm0 == 0)
			{
				if (mainflag.sysTickMs)
				{
					if (++ADCcoordinadorTiempos_timer >= (150/SYSTICK_MS))    //20ms
					{
						ADCcoordinadorTiempos_timer = 0;

						mainflag.ADCcoordinadorToggle = !mainflag.ADCcoordinadorToggle;

						if (mainflag.ADCcoordinadorToggle == 1)
						{
							ADC_config2temperature();
						}
						else
						{
							ADC_config2termopile();
						}

						ADCcoordinadorTiempos_sm0++;
					}
				}
			}
			else if (ADCcoordinadorTiempos_sm0 == 1)
			{
				if (mainflag.ADCcoordinadorToggle == 1)
				{
					if (temperature_job())
					{
						main_schedule.bf.startup_finish_stable_temperature = STARTUP_FINISHED;
						e.sensor[ERROR_IDX_THERMOCOUPLE].code = 0;
						//main_schedule.bf.status_thermocuple = STATUS_THERMOCOUPLE_OK;

						if (TCtemperature != temperature_filtered_smoothed)
						{
							TCtemperature = temperature_filtered_smoothed;//Actualiza TC
						}

						ADCcoordinadorTiempos_sm0 = 0;
					}
				}
				else
				{
					if (termopile_job())
					{
						ADCcoordinadorTiempos_sm0 = 0;

					}
				}
			}
			//

			//El highlimit que normalmente este a pullup con el contacto normalmente CERRADO (CORREGIDO FEB 09 2024), y se activara cuando abra el contacto,
			if (PinRead(PORTRxHIGHLIMIT, PINxHIGHLIMIT) == 1 )
			{
				//main_schedule.bf.status_highlimit = STATUS_HIGHLIMIT_BAD;
				e.sensor[ERROR_IDX_HIGHLIMIT].code = 1;//ERROR
			}
			else
			{
				//main_schedule.bf.status_highlimit = STATUS_HIGHLIMIT_OK;
				e.sensor[ERROR_IDX_HIGHLIMIT].code = 0;//NO ERROR
			}
			//------------------------------------------
			if (mainflag.sysTickMs)
			{
				if (++counter0 == (20/SYSTICK_MS))    //20ms
				{
					counter0 = 0;

					//
					pinGetLevel_job();
					//---------------------------

					if (pinGetLevel_hasChanged(PGLEVEL_LYOUT_SWONOFF))
					{
						pinGetLevel_clearChange(PGLEVEL_LYOUT_SWONOFF);
						//
						main_schedule.bf.startup_finish_read_switch_onoff = STARTUP_FINISHED;

						//
						indicatorTimed_setKSysTickTime_ms(75/SYSTICK_MS);
						indicatorTimed_run();
						//
						disp7s_clear_all();

						if (pinGetLevel_level(PGLEVEL_LYOUT_SWONOFF)== 0)	//active in low
						{
							/* ON*/
							/* forzar en sacar el nivel del PWM para el primer periodo, ya que T = 10s*/

							int16_t error = mypid0_adjust_kei_windup(); // dejar preparado para job()
							pid_find_ktop_ms(&mypid0, error);
							pid_pwm_stablish_levelpin(&mypid0);//set PWM por primera vez//tener de inmediato el valor de ktop_ms
							//
							main_schedule.bf.switch_status_onoff = 1;
							//mainflag.ADCrecurso = ADC_LIBRE;

						}
						else
						{
							main_schedule.bf.switch_status_onoff = 0;

							disp7s_update_data_array(DIPS7S_MSG_OFF, BASKETRIGHT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
							//
							pid_pwm_set_pin(&mypid0, 0);
							fryer_init();
							main_schedule = main_schedule_reset;
							e = e_reset;
							indicatorTimed_stop();
							//
							//added:
							termopila = termopilaReset;
							//mainflag.ADCrecurso = ADC_LIBRE;
							//como el ISR sigue corriendo, si existiera una interrupcion
							//por ADC corriendo... igual el flag se limpia por hardware
							//no hay necesidad de limpiarlo en este punto
							//el flag de habilitacion para leer la temperatura se restituye por la parte
							//del prorama que compara si el Termopile_counter < 50

							//
	//						PinTo0(PORTWxCONTROLLER_ONOFF, PINxCONTROLLER_ONOFF);
						}
					}
				}
			}



			if (main_schedule.bf.startup_finish_read_switch_onoff == STARTUP_FINISHED)// solo 1 vez se revisa al encender el equipo
			{
				if (main_schedule.bf.switch_status_onoff == 1)
				{
					if (error_job() == 0)


//					if ((main_schedule.bf.startup_finish_stable_temperature == STARTUP_FINISHED) &&
//							(main_schedule.bf.status_thermocuple == STATUS_THERMOCOUPLE_OK) &&
//							(main_schedule.bf.status_thermopile == STATUS_THERMOPILE_OK) &&
//							(main_schedule.bf.status_highlimit == STATUS_HIGHLIMIT_OK)
//							)
//					if (1)
					{
						//the machine can operate properly
						if (main_schedule.sm0 == 0)
						{
							kbmode_2basket_set_default();

							disp7s_update_data_array(DIPS7S_MSG_PRECALENTAMIENTO, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
							//
							fryer.bf.preheating = 1;
							fryer.viewmode = FRYER_VIEWMODE_PREHEATING;
							//
							//igDeteccFlama_resetJob();
							main_schedule.sm0++;
						}
						else if (main_schedule.sm0 == 1)
						{
							if (1)//(igDeteccFlama_doJob())
							{
								main_schedule.sm0++;    	//OK...Ignicion+deteccion de flama OK
															//PID_Control -> setpoint = Tprecalentamiento
							}
						}
						else if (main_schedule.sm0 == 2)
						{
							//Precalentamiento
							if (fryer.viewmode == FRYER_VIEWMODE_PREHEATING)
							{
								//if (TCtemperature >= tmprture_coccion.TC)
								if (TCtemperature >= 0)
								{
									//
									indicatorTimed_setKSysTickTime_ms(1000/SYSTICK_MS);
									indicatorTimed_run();
									//
									fryer.bf.preheating = 0;

									fryer.viewmode = FRYER_VIEWMODE_COOK;

									for (int i=0; i<BASKET_MAXSIZE; i++)//added
									{
										kbmode_default(&fryer.basket[i].kb);
										fryer.basket[i].kbmode = KBMODE_DEFAULT;
									}
									fryer.bf.operative_mode = 1;
									//

									main_schedule.sm0++;
								}
							}
						}
						else if (main_schedule.sm0 == 3)
						{
						}

						if ((fryer.viewmode == FRYER_VIEWMODE_PREHEATING) || (fryer.viewmode == FRYER_VIEWMODE_COOK))
						{
							if (ikb_key_is_ready2read(KB_LYOUT_PROGRAM))
							{
								ikb_key_was_read(KB_LYOUT_PROGRAM);

								if ( ikb_get_AtTimeExpired_BeforeOrAfter(KB_LYOUT_PROGRAM) == KB_AFTER_THR)
								{
									fryer.viewmode = FRYER_VIEWMODE_PROGRAM;
									fryer.ps_program = ps_reset;

									indicatorTimed_setKSysTickTime_ms(1000/SYSTICK_MS);
									indicatorTimed_run();

									//Salir actualizando eeprom
									for (int i=0; i<BASKET_MAXSIZE; i++)
									{
										eeprom_update_block( (struct _t *)(&basket_temp[i].cookCycle.time), (struct _t *)(&COOKTIME[i]), sizeof(struct _t));
									}

									disp7s_save_data_array(data_array_buffer, DISP7S_TOTAL_NUMMAX);
								}
								//added 7 abr 2022
								else//KB_BEFORE_THR
								{
									/// Visualizar la temperatura
									fryer.viewmode = FRYER_VIEWMODE_VIEWCOOKTEMP;
									fryer.ps_viewTemp = ps_reset;

									indicatorTimed_setKSysTickTime_ms(1000/SYSTICK_MS);
									indicatorTimed_run();

									//added

									disp7s_save_data_array(data_array_buffer, DISP7S_TOTAL_NUMMAX);
								}
							}
						}
						//
						if (fryer.viewmode == FRYER_VIEWMODE_PROGRAM)
						{
							if (psmode_program() == 1)
							{
								if (fryer.bf.preheating == 1)//sigue en precalentamiento ?
								{
									fryer.viewmode = FRYER_VIEWMODE_PREHEATING;

									//set kb
									for (int i=0; i<BASKET_MAXSIZE; i++)
									{
										kbmode_default(&fryer.basket[i].kb);
									}
									//lcdan_print_PSTRstring(PSTR("MELT"));
									disp7s_clear_all();
									disp7s_update_data_array(DIPS7S_MSG_PRECALENTAMIENTO, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
								}
								else
								{
									fryer.viewmode = FRYER_VIEWMODE_COOK;
									for (int i=0; i<BASKET_MAXSIZE; i++)//added
									{
										kbmode_default(&fryer.basket[i].kb);
										fryer.basket[i].kbmode = KBMODE_DEFAULT;
									}
									//added
									//lcdanBuff_dump2device(lcdanBuff);
									disp7s_update_data_array(data_array_buffer, 0, DISP7S_TOTAL_NUMMAX);
								}

							}
						}

						//added 7 ab 2022
						if (fryer.viewmode == FRYER_VIEWMODE_VIEWCOOKTEMP)
						{
							if (psmode_viewTemp() == 1)
							{
								if (fryer.bf.preheating == 1)	//sigue en precalentamiento
								{
									fryer.viewmode = FRYER_VIEWMODE_PREHEATING;

									//set kb
									for (int i=0; i<BASKET_MAXSIZE; i++)
									{
										kbmode_default(&fryer.basket[i].kb);
									}
									//lcdan_print_PSTRstring(PSTR("MELT"));
									disp7s_clear_all();
									disp7s_update_data_array(DIPS7S_MSG_PRECALENTAMIENTO, BASKETLEFT_DISP_CURSOR_START_X, BASKET_DISP_MAX_CHARS_PERBASKET);
								}
								else
								{
									fryer.viewmode = FRYER_VIEWMODE_COOK;

									for (int i=0; i<BASKET_MAXSIZE; i++)//added
									{
										kbmode_default(&fryer.basket[i].kb);
										fryer.basket[i].kbmode = KBMODE_DEFAULT;
									}
									//added
									//lcdanBuff_dump2device(lcdanBuff);
									disp7s_update_data_array(data_array_buffer, 0, DISP7S_TOTAL_NUMMAX);

								}
							}
						}

						if (fryer.bf.operative_mode == 1)
						{
							psmode_operative();
						}

						// PID control
						int16_t error = mypid0_adjust_kei_windup();
						pid_job(&mypid0, error);

						if (mainflag.sysTickMs)
						{
							if (++counter1 == (20/SYSTICK_MS))
							{
								counter1 = 0;
								//
								ikb_job();
							}
						}
					}
				}
			}
		}

		indicatorTimed_job();
		mainflag.sysTickMs = 0;

	}//while end

	return 0;
}

ISR(TIMER0_COMP_vect)
{
	isr_flag.sysTickMs = 1;
}
//ISR(ADC_vect)
//{
//	isr_flag.adcReady = 1;
//}
