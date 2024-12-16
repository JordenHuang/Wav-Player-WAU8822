#include <stdio.h>
#include "MCU_init.h"
#include "NUC100Series.h"
#include "SYS_init.h"
#include "NVT_I2C.h"

#include "wau8822.h"
#include "DEBUG_PRINTF.h"

/*---------------------------------------------------------------------------------------------------------*/
/*  Write 9-bit data to 7-bit address register of WAU8822 with I2C0                                        */
/*---------------------------------------------------------------------------------------------------------*/
void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data)
{
    /* Send START */
    I2C_START(I2C0);
    I2C_WAIT_READY(I2C0);

    /* Send device address */
    I2C_SET_DATA(I2C0, WAU8822_ADDR << 1);
    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    I2C_WAIT_READY(I2C0);

    /* Send register number and MSB of data */
    I2C_SET_DATA(I2C0, (uint8_t)((u8addr << 1) | (u16data >> 8)));
    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    I2C_WAIT_READY(I2C0);

    /* Send data */
    I2C_SET_DATA(I2C0, (uint8_t)(u16data & 0x00FF));
    I2C_SET_CONTROL_REG(I2C0, I2C_I2CON_SI);
    I2C_WAIT_READY(I2C0);

    /* Send STOP */
    I2C_STOP(I2C0);
}

static void RoughDelay(uint32_t t)
{
    volatile int32_t delay;

    delay = t;

    while(delay-- >= 0);
}

void WAU8822_ConfigSampleRate(uint32_t u32SampleRate)
{
    DEBUG_PRINTF("[NAU8822] Configure Sampling Rate to %d\n", u32SampleRate);

    if((u32SampleRate % 8) == 0) {
        I2C_WriteWAU8822(36, 0x008);    //12.288Mhz
        I2C_WriteWAU8822(37, 0x00C);
        I2C_WriteWAU8822(38, 0x093);
        I2C_WriteWAU8822(39, 0x0E9);
    } else if(u32SampleRate == 44100) {
        // I2C_WriteWAU8822(36, 0x007);    //11.2896Mhz, for 44.1k
        // I2C_WriteWAU8822(37, 0x021);
        // I2C_WriteWAU8822(38, 0x161);
        // I2C_WriteWAU8822(39, 0x026);
        I2C_WriteWAU8822(36, 0x008);    //12.288Mhz
        I2C_WriteWAU8822(37, 0x00C);
        I2C_WriteWAU8822(38, 0x093);
        I2C_WriteWAU8822(39, 0x0E9);
    } else {
        I2C_WriteWAU8822(36, 0x00B);    //16.934Mhz
        I2C_WriteWAU8822(37, 0x011);
        I2C_WriteWAU8822(38, 0x153);
        I2C_WriteWAU8822(39, 0x1F0);
    }

    switch (u32SampleRate) {
    case 16000:
        I2C_WriteWAU8822(6, 0x1AD);   /* Divide by 6, 16K */
        I2C_WriteWAU8822(7, 0x006);   /* 16K for internal filter cofficients */
        break;

    case 32000:
        I2C_WriteWAU8822(6, 0x16D);    /* Divide by 3, 32K */
        I2C_WriteWAU8822(7, 0x002);    /* 32K for internal filter cofficients */
        break;

    case 44100:
        // duriation super long
        // I2C_WriteWAU8822(6, 0x1C7); // PLL divide by 4 for 44.1 kHz
        // I2C_WriteWAU8822(7, 0x000); // Internal filter coefficients

        // weird sound, and a little longer than 5 seconds (Do_5sec.wav)
        I2C_WriteWAU8822(6, 0x14D);    /* Divide by 1, 48K */
        I2C_WriteWAU8822(7, 0x000);    /* 48K for internal filter cofficients */

        // slow
        // I2C_WriteWAU8822(6, 0x187); // PLL divider for 44.1 kHz
        // I2C_WriteWAU8822(7, 0x000); // Filter coefficients for 44.1 kHz

        // slow
        // I2C_WriteWAU8822(6, 0x16D);    /* Divide by 3, 32K */
        // I2C_WriteWAU8822(7, 0x002);    /* 32K for internal filter cofficients */
        break;

    case 48000:
        I2C_WriteWAU8822(6, 0x14D);    /* Divide by 1, 48K */
        I2C_WriteWAU8822(7, 0x000);    /* 48K for internal filter cofficients */
        break;

    case 11025:
        I2C_WriteWAU8822(6, 0x1ED);    /* Divide by 12 */
        break;

    case 22050:
        I2C_WriteWAU8822(6, 0x1AD);    /* Divide by 6 */
        // I2C_WriteWAU8822(7, 0x006);    /* 16K for internal filter cofficients */

        // slow
        // I2C_WriteWAU8822(6, 0x1ED);    /* Divide by 12 */
        // I2C_WriteWAU8822(7, 0x008);    /* 48K for internal filter cofficients */

        // fast
        // I2C_WriteWAU8822(6, 0x14D);    /* Divide by 1, 48K */
        // I2C_WriteWAU8822(7, 0x000);    /* 48K for internal filter cofficients */

        // still fast
        // I2C_WriteWAU8822(6, 0x16D);    /* Divide by 3, 32K */
        // I2C_WriteWAU8822(7, 0x002);    /* 32K for internal filter cofficients */

        // Ok
        // I2C_WriteWAU8822(6, 0x185); // PLL divider for 44.1 kHz, divide by 4
        // I2C_WriteWAU8822(6, 0x181); // PLL divider for 44.1 kHz, divide by 4
        // I2C_WriteWAU8822(7, 0x00A); // Filter coefficients for 44.1 kHz
        break;

    case 192000:
//      I2C_WriteWAU8822(6, 0x109);     //16bit/192k
        I2C_WriteWAU8822(6, 0x105);     //32bit/192k
        I2C_WriteWAU8822(72, 0x017);
        break;
    default:
        I2C_WriteWAU8822(6, 0x1ED);   /* Divide by 12, 8K */
        I2C_WriteWAU8822(7, 0x00A);   /* 8K for internal filter coefficients */
        break;
    }
}

void WAU8822_Setup(void)
{
    //uint32_t i;
    DEBUG_PRINTF("\nConfigure WAU8822 ...");

    I2C_WriteWAU8822(0,  0x000);   /* Reset all registers */
    RoughDelay(0x200);

	I2C_WriteWAU8822(1,  0x02F);        
	I2C_WriteWAU8822(2,  0x1B3);	// Enable L/R Headphone, ADC Mix/Boost, ADC
    I2C_WriteWAU8822(3,  0x07F);   /* Enable L/R main mixer, DAC */
    I2C_WriteWAU8822(4,  0x010);   /* 16-bit word length, I2S format, Stereo */
    I2C_WriteWAU8822(5,  0x000);   /* Companding control and loop back mode (all disable) */

#if(PLAY_RATE == 48000)
    I2C_WriteWAU8822(6,  0x14D);   /* Divide by 2, 48K */
    I2C_WriteWAU8822(7,  0x000);   /* 48K for internal filter coefficients */
#elif(PLAY_RATE == 32000)
    I2C_WriteWAU8822(6,  0x16D);   /* Divide by 3, 32K */
    I2C_WriteWAU8822(7,  0x002);   /* 32K for internal filter coefficients */
#elif(PLAY_RATE == 16000)
    I2C_WriteWAU8822(6,  0x1AD);   /* Divide by 6, 16K */
    I2C_WriteWAU8822(7,  0x006);   /* 16K for internal filter coefficients */
#else
    I2C_WriteWAU8822(6,  0x1ED);   /* Divide by 12, 8K */
    I2C_WriteWAU8822(7,  0x00A);   /* 8K for internal filter coefficients */
#endif

	I2C_WriteWAU8822(10, 0x008);	// DAC softmute is disabled, DAC oversampling rate is 128x
	I2C_WriteWAU8822(14, 0x108);	// ADC HP filter is disabled, ADC oversampling rate is 128x
	I2C_WriteWAU8822(15, 0x1EF);	// ADC left digital volume control
	I2C_WriteWAU8822(16, 0x1EF);	// ADC right digital volume control
	I2C_WriteWAU8822(43, 0x010);   
	I2C_WriteWAU8822(44, 0x000);	// LLIN/RLIN is not connected to PGA
	I2C_WriteWAU8822(45, 0x150);	// LLIN connected, and its Gain value
	I2C_WriteWAU8822(46, 0x150);	// RLIN connected, and its Gain value
	I2C_WriteWAU8822(47, 0x007);	// LLIN connected, and its Gain value
	I2C_WriteWAU8822(48, 0x007);	// RLIN connected, and its Gain value
	I2C_WriteWAU8822(49, 0x047);
	I2C_WriteWAU8822(50, 0x001);	// Left DAC connected to LMIX
	I2C_WriteWAU8822(51, 0x000);	// Right DAC connected to RMIX
 	I2C_WriteWAU8822(54, 0x139);	// LSPKOUT Volume
	I2C_WriteWAU8822(55, 0x139);	// RSPKOUT Volume

    GPIO_SetMode(PE, BIT14, GPIO_MODE_OUTPUT);
    PE14 = 0;

    DEBUG_PRINTF("[OK]\n");
}

void Init_I2C(void)
{
    I2C_Open(I2C0, I2C0_CLOCK_FREQUENCY);
    DEBUG_PRINTF("I2C0 clock %d Hz\n", I2C_GetBusClockFreq(I2C0)); // get I2C0 clock

    /* Set I2C3 4 Slave Addresses */
    // I2C_SetSlaveAddr(I2C0, 0, 0x15, I2C_GCMODE_DISABLE);   /* Slave Address : 0x15 */
    // I2C_SetSlaveAddr(I2C0, 1, 0x35, I2C_GCMODE_DISABLE);   /* Slave Address : 0x35 */
    // I2C_SetSlaveAddr(I2C0, 2, 0x55, I2C_GCMODE_DISABLE);   /* Slave Address : 0x55 */
    // I2C_SetSlaveAddr(I2C0, 3, 0x75, I2C_GCMODE_DISABLE);   /* Slave Address : 0x75 */

    I2C_EnableInt(I2C0);       // Enable I2C0 interrupt generation
    // // NVIC_EnableIRQ(I2C0_IRQn); // Enable NVIC I2C0 interrupt input
}
