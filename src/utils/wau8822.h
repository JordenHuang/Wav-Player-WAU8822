#ifndef _WAU8822_H_
#define _WAU8822_H_

#define WAU8822_ADDR    0x1A                /* WAU8822 Device ID */

void I2C_WriteWAU8822(uint8_t u8addr, uint16_t u16data);
void RoughDelay(uint32_t t);
void WAU8822_ConfigSampleRate(uint32_t u32SampleRate);
void WAU8822_Setup(void);
void Init_I2C(void);

#endif // _WAU8822_H_
