/*
 * psmode_program.h
 *
 *  Created on: Aug 25, 2021
 *      Author: jcaf
 */

#ifndef PSMODE_PROGRAM_H_
#define PSMODE_PROGRAM_H_

#define TMPRTURE_COCCION_FARENHEIT_MAX 450
#define TMPRTURE_COCCION_FARENHEIT_MIN 50
#define TMPRTURE_COCCION_FARENHEIT_STD 350

#define TMPRTURE_COCCION_CELCIUS_MAX 232
#define TMPRTURE_COCCION_CELCIUS_MIN 10
#define TMPRTURE_COCCION_CELCIUS_STD 177

struct _Tcoccion
{
	int16_t TC;
	int16_t min;
	int16_t max;
};
extern struct _Tcoccion tmprture_coccion;
extern struct _Tcoccion EEMEM TMPRTURE_COCCION;


int8_t psmode_program(void);
extern struct _pgrmmode pgrmode;

#endif /* PSMODE_PROGRAM_H_ */
