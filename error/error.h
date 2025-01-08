/*
 * error.h
 *
 *  Created on: Sep 4, 2023
 *      Author: jcaf
 */

#ifndef ERROR_ERROR_H_
#define ERROR_ERROR_H_

	#define ERROR_SIZE 3

	enum ERROR_IDX{
		ERROR_IDX_THERMOCOUPLE = 0,
		ERROR_IDX_THERMOPILE,
		ERROR_IDX_HIGHLIMIT
	};

	struct _sensor
	{
		int8_t sm0;
		int8_t code;
	};
	struct _error
	{
		int8_t idx;
		uint16_t timer;
		struct _error_flag
		{
			unsigned indicator:1;
			unsigned __a:7;
		}f;

		struct _sensor sensor[ERROR_SIZE];

	};

extern struct _error e;
extern struct _error e_reset;

void error_job(void);

#endif /* ERROR_ERROR_H_ */
