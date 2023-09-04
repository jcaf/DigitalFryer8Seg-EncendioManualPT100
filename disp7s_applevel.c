/*
 * disp7s_applevel.c
 *
 *  Created on: Jan 3, 2023
 *      Author: jcaf
 */
#include "main.h"
#include "system.h"
#include "disp7s_applevel.h"
#include "display7s_setup.h"

const unsigned char DISP7_NUMERIC_ARR[10] =
{
	D7S_DATA_0,
	D7S_DATA_1,
	D7S_DATA_2,
	D7S_DATA_3,
	D7S_DATA_4,
	D7S_DATA_5,
	D7S_DATA_6,
	D7S_DATA_7,
	D7S_DATA_8,
	D7S_DATA_9,
};

const unsigned char DIPS7S_MSG_OFF[4] =
{
		D7S_DATA_O,
		D7S_DATA_F,
		DISP7Sinvfix(D7S_DATA_F),
		D7S_DATA_BLANK
};

const unsigned char DIPS7S_MSG_donE[4] =
{
		D7S_DATA_d,
		D7S_DATA_o,
		DISP7Sinvfix(D7S_DATA_n),
		D7S_DATA_E
};

const unsigned char DIPS7S_MSG_OIL[4] =
{
		D7S_DATA_O,
		D7S_DATA_I,
		DISP7Sinvfix(D7S_DATA_L),
		D7S_DATA_BLANK
};

const unsigned char DIPS7S_MSG_SET[4] =
{
		D7S_DATA_S,
		D7S_DATA_E,
		DISP7Sinvfix(D7S_DATA_t),
		D7S_DATA_BLANK
};
const unsigned char DIPS7S_MSG_PRECALENTAMIENTO[4] =
{
		D7S_DATA_P,
		D7S_DATA_r,
		DISP7Sinvfix(D7S_DATA_E),
		D7S_DATA_BLANK
};

const unsigned char DIPS7S_MSG_ERROR[4] =
{
		D7S_DATA_E,
		D7S_DATA_r,
		DISP7Sinvfix(D7S_DATA_r),
		D7S_DATA_BLANK
};

const unsigned char DIPS7S_MSG_THERMOCOUPLE_NC[4] =
{
		D7S_DATA_t ,
		D7S_DATA_h | (1<< D7S_DP),
		DISP7Sinvfix(D7S_DATA_n),
		D7S_DATA_c
};

const unsigned char DIPS7S_MSG_THERMOPILE[4] =
{
		D7S_DATA_t | (1<< D7S_DP),
		D7S_DATA_P,
		DISP7Sinvfix(D7S_DATA_I),
		D7S_DATA_L
};

const unsigned char DIPS7S_MSG_HIGHLIMIT[4] =
{
		D7S_DATA_H | (1<< D7S_DP),
		D7S_DATA_L,
		DISP7Sinvfix(D7S_DATA_BLANK),
		D7S_DATA_BLANK
};
/*
void disp7s_datarr_ErrorTh(void)
{
	//PARECE QUE EL ASUNTO ESTA EN EL SEGMENTO D Y E

	disp7s_data_array[0]= D7S_DATA_E;//mientras no toque otro segmento, est[a ok
	disp7s_data_array[1]= D7S_DATA_8;
	disp7s_data_array[2]= DISP7Sinvfix(D7S_DATA_BLANK);
	disp7s_data_array[3]= D7S_DATA_8;
	disp7s_data_array[4]= D7S_DATA_BLANK;
	disp7s_data_array[5]= D7S_DATA_8,
	//disp7s_data_array[6]= DISP7Sinvfix(D7S_DATA_t);
	disp7s_data_array[6]= D7S_DATA_BLANK;

	disp7s_data_array[7]= D7S_DATA_8;
	//disp7s_data_array[7]= D7S_DATA_h;
}
*/
//
//void disp7s_on2DecPoint_basket0(void)
//{
//	OR_BITWISE(disp7s_data_array[1],(1<< D7S_DPinv));
//	OR_BITWISE(disp7s_data_array[2],(1<< D7S_DP));
//}
//void disp7s_on2DecPoint_basket1(void)
//{
//	OR_BITWISE(disp7s_data_array[5],(1<< D7S_DPinv));
//	OR_BITWISE(disp7s_data_array[6],(1<< D7S_DP));
//}
//
//void disp7s_off2DecPoint_basket0(void)
//{
//	AND_BITWISE(disp7s_data_array[1],~(1<< D7S_DPinv));
//	AND_BITWISE(disp7s_data_array[2],~(1<< D7S_DP));
//}
//void disp7s_off2DecPoint_basket1(void)
//{
//	AND_BITWISE(disp7s_data_array[5],~(1<< D7S_DPinv));
//	AND_BITWISE(disp7s_data_array[6],~(1<< D7S_DP));
//}

//parsear a toda el array es lo mejor
void disp7s_fix_upsidedown_display(unsigned char *data)
{
	*data = (((*data & 0x10)>>2) | ((*data & 0x04)<<2) | (*data & 0xEB));
}


//Display order
//[0][1][2][3]    [4][5][6][7]
//[2] -> upsidedown
//[6] -> upsidedown
void disp7s_fix_all_upsidedown_display(void)
{
	disp7s_fix_upsidedown_display(&disp7s_data_array[2]);
	disp7s_fix_upsidedown_display(&disp7s_data_array[6]);
}
void disp7s_blank_displays(unsigned char *data, int8_t initial_position, int8_t num_digits)
{
	//int8_t idx = initial_position;
	for (int i=0; i<num_digits; i++)
	{
		data[initial_position++] = D7S_DATA_BLANK;
	}
}


void disp7s_update_data_array(const unsigned char *data, int8_t initial_position, int8_t num_digits)
{
	for (int i = 0; i< num_digits; i++ )
	{
		disp7s_data_array[initial_position++] = data[i];
	}
}

void disp7s_save_data_array(unsigned char *data_array_buffer, int8_t num_digits)
{
	for (int i = 0; i<num_digits ; i++ )
	{
		data_array_buffer[i] = disp7s_data_array[i];
	}
}


void disp7s_clear_all(void)
{
	disp7s_blank_displays(disp7s_data_array,0,DISP7S_TOTAL_NUMMAX);
}

void integer_to_arraybcd_msb_lsb_paddingleft_blank(int value, unsigned char *arraybcd, int8_t num_digits)
{
	unsigned char bcd[10];

	//blank all buffer
	disp7s_blank_displays(arraybcd,0,num_digits);

	int length = integer_to_arraybcd_msb_lsb(value, bcd);
	int idx= num_digits - length;
	for (int i = 0; i<length; i++ )
	{
		arraybcd[idx++] = DISP7_NUMERIC_ARR[bcd[i]];
	}
}
void integer_to_arraybcd_msb_lsb_paddingleft_zeroes(int value, unsigned char *arraybcd, int8_t num_digits)
{
	unsigned char bcd[10];

	//blank all buffer
	//disp7s_blank_displays(arraybcd,0,num_digits);
	for (int8_t i=0; i<num_digits; i++)
	{
		arraybcd[i] = D7S_DATA_0;
	}


	int length = integer_to_arraybcd_msb_lsb(value, bcd);
	int idx= num_digits - length;
	for (int i = 0; i<length; i++ )
	{
		arraybcd[idx++] = DISP7_NUMERIC_ARR[bcd[i]];
	}
}

void disp7s_decimalpoint_on(unsigned char *dig)
{

	OR_BITWISE(*dig, (1<< D7S_DP) );
}
void disp7s_decimalpoint_off(unsigned char *dig)
{

	AND_BITWISE(*dig, ~(1<< D7S_DP) );
}
