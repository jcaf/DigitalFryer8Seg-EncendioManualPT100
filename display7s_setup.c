#include "system.h"
#include "types.h"
#include "display7s_setup.h"
#include "Display7S/display7s.h"


static inline void disp7s_mux0_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q0, PINxDISPLAY7S_Q0);
}
static inline void disp7s_mux0_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q0, PINxDISPLAY7S_Q0);
}
//
static inline void disp7s_mux1_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q1, PINxDISPLAY7S_Q1);
}
static inline void disp7s_mux1_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q1, PINxDISPLAY7S_Q1);
}
//
static inline void disp7s_mux2_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q2, PINxDISPLAY7S_Q2);
}
static inline void disp7s_mux2_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q2, PINxDISPLAY7S_Q2);
}
//
static inline void disp7s_mux3_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q3, PINxDISPLAY7S_Q3);
}
static inline void disp7s_mux3_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q3, PINxDISPLAY7S_Q3);
}
//
static inline void disp7s_mux4_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q4, PINxDISPLAY7S_Q4);
}
static inline void disp7s_mux4_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q4, PINxDISPLAY7S_Q4);
}
//
static inline void disp7s_mux5_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q5, PINxDISPLAY7S_Q5);
}
static inline void disp7s_mux5_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q5, PINxDISPLAY7S_Q5);
}
//
static inline void disp7s_mux6_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q6, PINxDISPLAY7S_Q6);
}
static inline void disp7s_mux6_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q6, PINxDISPLAY7S_Q6);
}
//
static inline void disp7s_mux7_off(void)
{
    PinTo0(PORTWxDISPLAY7S_Q7, PINxDISPLAY7S_Q7);
}
static inline void disp7s_mux7_on(void)
{
    PinTo1(PORTWxDISPLAY7S_Q7, PINxDISPLAY7S_Q7);
}

struct _disp7s_mux disp7s_mux[DISP7S_TOTAL_NUMMAX] =
{
    {{disp7s_mux0_off, disp7s_mux0_on}},
    {{disp7s_mux1_off, disp7s_mux1_on}},
    {{disp7s_mux2_off, disp7s_mux2_on}},
    {{disp7s_mux3_off, disp7s_mux3_on}},
    {{disp7s_mux4_off, disp7s_mux4_on}},
	{{disp7s_mux5_off, disp7s_mux5_on}},
	{{disp7s_mux6_off, disp7s_mux6_on}},
	{{disp7s_mux7_off, disp7s_mux7_on}}
};

unsigned char disp7s_data_array[DISP7S_TOTAL_NUMMAX];

void disp7s_init(void)
{
	int8_t i;

	//set all pointers and constansts
	disp7s_setup(DISP7S_TOTAL_NUMMAX, &PORTWxDISPLAY7S_DATA,  disp7s_data_array, disp7s_mux);

	//config transistors
	for (i=0; i< DISP7S_TOTAL_NUMMAX; i++)
    {
        disp7s_mux[i].Qonoff[0]();//off
    }

    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q0, PINxDISPLAY7S_Q0);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q1, PINxDISPLAY7S_Q1);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q2, PINxDISPLAY7S_Q2);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q3, PINxDISPLAY7S_Q3);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q4, PINxDISPLAY7S_Q4);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q5, PINxDISPLAY7S_Q5);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q6, PINxDISPLAY7S_Q6);
    ConfigOutputPin(CONFIGIOxDISPLAY7S_Q7, PINxDISPLAY7S_Q7);

    //config output port
    ConfigOutputPort(CONFIGIOxDISPLAY7S_DATA,OUTPUT_PORT8BIT);
}
