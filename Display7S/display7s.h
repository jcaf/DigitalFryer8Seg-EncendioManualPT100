/*
 * display7s.h
 *
 *  Created on: Dec 29, 2022
 *      Author: jcaf
 */

#ifndef DISPLAY7S_DISPLAY7S_H_
#define DISPLAY7S_DISPLAY7S_H_

struct _disp7s_mux
{
    PTRFX_retVOID Qonoff[2];//0=..off(), 1=..on()
};

//void disp7s_setup(uint8_t disp7s_nummax, volatile uint8_t * port,  char * _disp7s_data);
void disp7s_setup(uint8_t disp7s_nummax, volatile uint8_t * port,  unsigned char * _disp7s_data_array, struct _disp7s_mux * __disp7s_mux);
//void disp7s_init(void);
void disp7s_job(void);


#endif /* DISPLAY7S_DISPLAY7S_H_ */
