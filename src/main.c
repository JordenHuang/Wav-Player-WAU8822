#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "MCU_init.h"
#include "NUC100Series.h"
#include "SYS_init.h"

#include "ffconf.h"
#include "diskio.h"
#include "ff.h"

#define MLH_LED
#define MLH_7SEG_INT
#define MLH_KEYPAD_INT
#define MLH_LCD
#define MLH_LCD_DYNAMIC_UPDATE
#include "myLib.h"

// #include "utils/wave_sample.h"
#include "utils/wav_lib.h"
#include "utils/wau8822.h"


uint32_t g_u32TxValue;
uint32_t cnt_5ms = 0;

BYTE ff_buff[512] ; 
FATFS FatFs[FF_VOLUMES];               /* File system object for logical drive */
wav_header_t wav_header;
unsigned char *ptr;

FIL wav_file_object;
FRESULT res;
const char wav_file_path[] = "Do_5sec.wav";
// const char wav_file_path[] = "test.wav";
// const char wav_file_path[] = "gb10.wav";
// const char wav_file_path[] = "im60.wav";
#define WAV_DATA_BUF_SIZE 128
unsigned char wav_data[WAV_DATA_BUF_SIZE];
uint32_t u32_wav_data_buf_size;
// #define PCM_BUFF_SIZE WAV_DATA_BUF_SIZE
#define PCM_BUFF_SIZE 512
uint16_t pcm_buffer[PCM_BUFF_SIZE];
uint32_t pcm_buffer_idx = 0;
bool pcm_buffer_full = true;


// Timer 0 interrupt
void TMR0_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER0); // Clear Timer0 time-out interrupt flag
    cnt_5ms += 1;
}

void Init_Timer0(void)
{
    TIMER_Open(TIMER0, TMR0_OPERATING_MODE, TMR0_OPERATING_FREQ);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}


void I2S_IRQHandler(void)
{
    uint32_t u32Reg;
    uint32_t u32data;
    uint16_t u16data;
    uint8_t i;
    static uint32_t c = 0;
    // u32Reg = I2S_GET_INT_FLAG(I2S, I2S_STATUS_TXTHF_Msk | I2S_STATUS_RXTHF_Msk);
    u32Reg = I2S->STATUS;

    // printf("I2S IRQ\n");
    if (u32Reg & I2S_STATUS_TXTHF_Msk) {
        // printf("I2S TX IRQ, %d\n", pcm_buffer_idx);
        // if (pcm_buffer_full) {
        //     printf("full return\n");
        //     return;
        // }
#if 0
        for (i = 0; i < 4; ++i) {
            u32data = (*(ptr + 1) << 24) | (*(ptr) << 16) | (*(ptr + 1) << 8) | (*(ptr));
            // TODO: what if use below
            // u32data = (*(ptr) << 24) | (*(ptr+1) << 16) | (*(ptr) << 8) | (*(ptr+1));
            I2S_WRITE_TX_FIFO(I2S, u32data);
            printf("d: %08x\n", u32data);
            ptr += 2;
            c += 2;
            if (c >= wav_header.data_chunk_size / 2) {
                I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
            }
        }
#else
        for (i = 0; i < 4; ++i) {
            if (wav_header.num_of_channels == 1) {
                u32data = (pcm_buffer[pcm_buffer_idx] << 16) | pcm_buffer[pcm_buffer_idx];
                // u16data = ((pcm_buffer[pcm_buffer_idx] & 0x00FF) << 8) | ((pcm_buffer[pcm_buffer_idx] >> 8) & 0x00FF);
                // printf("%04x\n", u16data);
                // u32data = (u16data << 16) | 0;
                // u32data = (u16data << 16) | u16data;
                // printf("u32: %04x\n", u32data);
                pcm_buffer_idx += 1;
                wav_header.data_chunk_size -= 2;
            }
            //  else {
            //     u32data = pcm_buffer[pcm_buffer_idx] << 16 | pcm_buffer[pcm_buffer_idx + 1];
            //     pcm_buffer_idx += 2;
            // wav_header.data_chunk_size -= 4;
            // }
            c += 1;

            I2S_WRITE_TX_FIFO(I2S, u32data);
            if (wav_header.data_chunk_size == 0) {
                I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
                printf("Song end\n");
                return;
            }
        }
        // if (u32data == 0) {
        //     printf("%08x\n", u32data);
        //     printf("%d, %d\n", pcm_buffer_idx, c);
        // }
        if (pcm_buffer_idx >= PCM_BUFF_SIZE) {
            // printf("pcm full\n");
            pcm_buffer_full = true;
            I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
        }
#endif
    }

    // if (u32Reg & I2S_STATUS_RXTHF_Msk) { // Check if RX threshold interrupt
    //     printf("I2S RX IRQ\n");
    //     // Read data from the RX FIFO
    //     printf("Received data: 0x%X\n", I2S_READ_RX_FIFO(I2S));
    //     // No explicit clear needed as reading the FIFO clears the interrupt
    // }
}

/*---------------------------------------------------------*/
/* User Provided RTC Function for FatFs module             */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support an RTC.                     */
/* This function is not required in read-only cfg.         */

unsigned long get_fattime(void)
{
    unsigned long tmr;

    tmr = 0x00000;

    return tmr;
}

void put_rc(FRESULT rc)
{
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

    uint32_t i;

    for (i = 0; (i != (UINT)rc) && *p; i++)
    {
        while (*p++) ;
    }

    printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}

int main(void)
{
    int16_t i;

    // uint32_t offset;

    SYS_Init();

    mlh_init_GPIO_for_led();
    mlh_init_7seg();
    mlh_init_keypad_INT();
    mlh_init_lcd();

    // Init_Timer0();

    printf("rc=%d\n", (WORD)disk_initialize(0));
    disk_read(0, ff_buff, 2, 1);
    f_mount(&FatFs[0], "0:", 1);
    res = f_open(&wav_file_object, (TCHAR*)wav_file_path, FA_READ);
    if (res)
    {
        put_rc(res);
        printf("[ERROR] Mount file system is failed!\n");

        while (1);
    }
    if (res == FR_OK) {
        f_lseek(&wav_file_object, 0);
        u32_wav_data_buf_size = WAV_DATA_BUF_SIZE;
        f_read(&wav_file_object, wav_data, u32_wav_data_buf_size, &u32_wav_data_buf_size);
        while (u32_wav_data_buf_size != WAV_DATA_BUF_SIZE);
        printf("file is opened!!\n");

        // i = parse_wav_arr(wav_sample, &wav_header, (void*)(&ptr));
        i = parse_wav_arr(wav_data, &wav_header, (void*)(&ptr));
        printf("Status of parse_wav: %s\n\n", i == 0 ? "Success" : "Fail");

        printf("riff: %x\n", wav_header.riff);
        printf("file size: %d\n", wav_header.file_size);
        printf("format: %d\n", wav_header.format);
        printf("format_chunk_id: %x\n", wav_header.format_chunk_id);
        printf("format_chunk_size: %d\n", wav_header.format_chunk_size);
        printf("audio_format: %d\n", wav_header.audio_format);
        printf("num_of_channels: %d\n", wav_header.num_of_channels);
        printf("sample_rate: %d\n", wav_header.sample_rate);
        printf("byte_per_sec: %d\n", wav_header.byte_per_sec);
        printf("block_align: %d\n", wav_header.block_align);
        printf("bits_per_sample: %d\n", wav_header.bits_per_sample);
        printf("data_chunk_id: %x\n", wav_header.data_chunk_id);
        printf("Data Chunk Found (data size): Size %u bytes\n", wav_header.data_chunk_size);
    }
    printf("Read file finish\n");

    // I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
    // while(1);

    GPIO_SetMode(PC, (BIT0 | BIT1), GPIO_MODE_QUASI);
    PC0 = 1;
    PC1 = 1;

    /* Init I2C0 to access WAU8822 */
    Init_I2C();

    /* Initialize WAU8822 codec */
    WAU8822_Setup();

    WAU8822_ConfigSampleRate(wav_header.sample_rate);

    // select source from HXT(12MHz)
    CLK_SetModuleClock(I2S_MODULE, CLK_CLKSEL2_I2S_S_HXT, 0);
    CLK_EnableModuleClock(I2S_MODULE);

    printf("real: %d\n", I2S_Open(I2S, I2S_MODE_SLAVE, 8000, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S));
    // printf("real: %d\n", I2S_Open(I2S, I2S_MODE_SLAVE, 22050, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S));

    /* Set MCLK and enable MCLK */
    I2S_EnableMCLK(I2S, 12000000);

    g_u32TxValue = 0x55005501;

    /* Start I2S play iteration */
    // I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
    NVIC_EnableIRQ(I2S_IRQn);
    // I2S_ENABLE_TX(I2S);

    printf("I2S STATUS: 0x%X\n", I2S->STATUS);
    printf("I2S IE: 0x%X\n", I2S->IE);


    f_lseek(&wav_file_object, 44);

    // I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
    printf("Song start\n");

i = 0;
    while (1) {
        if (pcm_buffer_full) {
            f_read(&wav_file_object, pcm_buffer, PCM_BUFF_SIZE * sizeof(pcm_buffer[0]), &pcm_buffer_idx);
            // printf("read: %d\n", pcm_buffer_idx);
            // if (i == 0) {
            //     printf("%04x %04x\n", pcm_buffer[0], pcm_buffer[1]);
            //     i += 1;
            // }
            pcm_buffer_idx = 0;
            pcm_buffer_full = false;
            I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
        }
        // printf("size: %d\n", wav_header.data_chunk_size);
        if (wav_header.data_chunk_size == 0) {
            break;
        }
    }

    printf("song end\n");
    while (1);


    while (1) {
        switch (mlh_get_key_state()) {
        case K_DOWN:
            mlh_number_to_seg_buf(KEY_FLAG, false);
            if (KEY_FLAG == 1) {
            I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
                printf("tx level: %d\n", I2S_GET_TX_FIFO_LEVEL(I2S));
                printf("rx level: %d\n", I2S_GET_RX_FIFO_LEVEL(I2S));
            }
            if (KEY_FLAG == 4) {
                printf("KEY 4 down\n");
                printf("  g_u32TxValue: 0x%x\n", g_u32TxValue);
                printf("  I2S STATUS: 0x%X\n", I2S->STATUS);
                printf("  I2S IE: 0x%X\n", I2S->IE);
            }
            break;
        case K_UP:
            if (KEY_FLAG_LAST == 4) {
                printf("KEY 4 UP\n");
            }
            break;
        case K_PRESSING:
            if (KEY_FLAG == 4) {
                // I2S_WRITE_TX_FIFO(I2S, g_u32TxValue);
            }
            break;
        case K_NO_PRESSING:
            break;
        }

        // I2S_WRITE_TX_FIFO(I2S, g_u32TxValue);
        // printf("g_u32TxValue: 0x%x\n", g_u32TxValue);
        // printf("I2S STATUS: 0x%X\n", I2S->STATUS);

        // CLK_SysTickDelay(1000000);
    }
}
