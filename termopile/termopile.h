/*
 * termopile.h
 *
 *  Created on: 20 feb. 2025
 *      Author: NEFER
 */

#ifndef TERMOPILE_TERMOPILE_H_
#define TERMOPILE_TERMOPILE_H_

struct _termopila
{
	int sm0;
	int sm1;
	int counter0;
	int counter1;
	int error_counter;
};

int8_t termopile_job(void);

extern struct _termopila termopila;
extern const struct _termopila termopilaReset;


#endif /* TERMOPILE_TERMOPILE_H_ */
