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

// #include "wave_sample.h"
#include "wav_lib.h"
#include "wau8822.h"
#include "DEBUG_PRINTF.h"


/* -------------------- */
// Macros
/* -------------------- */
#define WAV_HEADER_BUF_SIZE 64
#define PCM_BUFF_SIZE 512
#define SCROLL_BAR_WIDTH 5
#define PLAYBACK_SAMPLE_RATE 8192

/* -------------------- */
// Game state enumeration define and global variable
/* -------------------- */
typedef enum Program_State {
    P_START,
    P_MODE_SELECT,
    P_MODE_AUDIO_PLAY,
    P_MODE_PLAYBACK,
    P_MODE_AUDIO_RECORDER,
} Program_State;

struct Pgm_Mode {
    unsigned char name[17];
    Program_State mode;
};

#define MODE_NUM 3
Program_State pgm_state = P_START;
struct Pgm_Mode pgm_mode_name_map[MODE_NUM] = {
    {"Player",   P_MODE_AUDIO_PLAY},
    {"Playback", P_MODE_PLAYBACK},
    {"Recorder", P_MODE_AUDIO_RECORDER},
};

/* -------------------- */
// SD card related global variable
/* -------------------- */
FIL fp;
BYTE ff_buff[128];
// File system object for logical drive
FATFS FatFs[FF_VOLUMES];
// Path to mount
unsigned char mount_path[] = "0:";

/* -------------------- */
// Wav file related global variable
/* -------------------- */
wav_header_t wav_header;
unsigned char wav_header_data[WAV_HEADER_BUF_SIZE];
uint16_t pcm_buffer[PCM_BUFF_SIZE];
uint32_t pcm_buffer_idx = 0;
bool pcm_buffer_needs_refill = true;
// file name <= 8 characters, and extension <= 3 characters
const char wav_file_path[][13] = {
    "stereo.wav",
    "Do_8192.wav",
    "test.wav",
    "Do_5sec.wav",
    "gb10.wav",
    "im60.wav",
    "happy.wav",
    // "Thunder.wav",
    // "hero.wav",
    // "highhope.wav",
};
uint32_t wav_file_count = sizeof(wav_file_path) / sizeof(wav_file_path[0]);

/* -------------------- */
// EINT1 related global variable
/* -------------------- */
bool STOP_PLAYING = false;

/* -------------------- */
// Timer related global variable
/* -------------------- */
uint32_t cnt_5ms = 0;
uint32_t seg_no = 3;
bool seg_effect = true;
bool start_count = false;

/* -------------------- */
// Function prototypes
/* -------------------- */
void init_audio_stuff(uint32_t sample_rate);
void init_sdcard_stuff(void);
void open_wav_file(FIL *fp, const char *file_path, wav_header_t *header);
void start_play(FIL *fp);
void close_wav_file(FIL *fp);

void put_rc(FRESULT rc);
unsigned long get_fattime(void);

void show_song_menu(uint16_t idx, uint16_t line);
void show_mode_menu(uint16_t idx);
void pgm_start(void);
void pgm_mode_selection(void);
void pgm_audio_play(void);
void pgm_audio_playback(void);


/* -------------------- */
// Extenal INT 1 functions
/* -------------------- */

/**
 * @brief IRQ handler for external interrupt
 * @details For stop and quit the song when the song is playing
 */
void EINT1_IRQHandler(void)
{
    GPIO_CLR_INT_FLAG(PB, BIT15); // Clear GPIO interrupt flag
    STOP_PLAYING = true;
    DEBUG_PRINTF("int1\n");
}

void Init_EXTINT(void)
{
    // Configure EINT1 pin and enable interrupt by rising and falling edge trigger
    GPIO_SetMode(PB, BIT15, GPIO_MODE_INPUT);
    GPIO_EnableEINT1(PB, 15, GPIO_INT_FALLING); // RISING, FALLING, BOTH_EDGE, HIGH, LOW
    NVIC_EnableIRQ(EINT1_IRQn);

    // Enable interrupt de-bounce function and select de-bounce sampling cycle time
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_LIRC, GPIO_DBCLKSEL_64);
    GPIO_ENABLE_DEBOUNCE(PB, BIT15);
}

/* -------------------- */
// Timer 0 functions
/* -------------------- */

/**
 * @brief IRQ handler for timer 0
 * @details Counting time, like how long the audio played, or the 7seg effect
 */
void TMR0_IRQHandler(void)
{
    TIMER_ClearIntFlag(TIMER0); // Clear Timer0 time-out interrupt flag
    if (start_count) {
        cnt_5ms += 1;

        if (cnt_5ms % 200 == 0) {
            mlh_number_to_seg_buf(cnt_5ms / 200, false);
        }
    }
    else if (seg_effect) {
        cnt_5ms += 1;
        if (cnt_5ms % 125 == 0) {
            mlh_set_7seg_buf(3 - seg_no, 16);
            seg_no = (seg_no + 1) % 4;
            mlh_set_7seg_buf(3 - seg_no, 17);
        }
    }
}

void Init_Timer0(void)
{
    TIMER_Open(TIMER0, TMR0_OPERATING_MODE, TMR0_OPERATING_FREQ);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}


/* -------------------- */
// I2S IRQ handler, to send data to WAU8822
/* -------------------- */
/**
 * @brief IRQ handler for I2S
 * @details send and recieve data to or from I2S, in modes like audio player, playback .etc
 */
void I2S_IRQHandler(void)
{
    uint32_t u32status;
    uint32_t u32data;
    // uint16_t u16data;
    uint32_t i;
    u32status = I2S_GET_INT_FLAG(I2S, I2S_STATUS_TXTHF_Msk | I2S_STATUS_RXTHF_Msk);

    // I2S TX threshold interrupt
    if (u32status & I2S_STATUS_TXTHF_Msk) {
        // Mode audio play
        if (pgm_state == P_MODE_AUDIO_PLAY) {
            for (i = 0; i < 4; ++i) {
                // Prepare data from 16 bit to 32 bit
                if (wav_header.num_of_channels == 1) {
                    u32data = (pcm_buffer[pcm_buffer_idx] << 16) | pcm_buffer[pcm_buffer_idx];
                    pcm_buffer_idx += 1;
                    wav_header.data_chunk_size -= 2;
                } else if (wav_header.num_of_channels == 2) {
                    // FIXME: I'm not sure if it's wrong
                    if (wav_header.block_align == 4) {
                        u32data = pcm_buffer[pcm_buffer_idx] << 16 | pcm_buffer[pcm_buffer_idx + 1];
                        pcm_buffer_idx += 2;
                        wav_header.data_chunk_size -= 4;
                    }
                }

                // Send sound data
                I2S_WRITE_TX_FIFO(I2S, u32data);

                // Check end of the song
                if (wav_header.data_chunk_size == 0) {
                    I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
                    return;
                }
            }

            // Check if buffer needs refill sound data
            if (pcm_buffer_idx >= PCM_BUFF_SIZE) {
                pcm_buffer_needs_refill = true;
                I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
            }
        }

        // Mode playback
        // From https://github.com/OpenNuvoton/NUC472_442BSP/blob/master/SampleCode/StdDriver/I2S_NAU8822/nuc472_442_isr.c
        else if (pgm_state == P_MODE_PLAYBACK) {
            uint32_t u32Len;
            uint16_t *pBuffTx = &pcm_buffer[0];
            // Read Tx FIFO free size
            u32Len = 8 - I2S_GET_TX_FIFO_LEVEL(I2S);

            if (pcm_buffer_idx >= 8) {
                for (i = 0; i < u32Len; ++i) {
                    u32data = (pBuffTx[i] << 16) | pBuffTx[i];
                    I2S_WRITE_TX_FIFO(I2S, u32data);
                    DEBUG_PRINTF("dt:%d\n", u32data);
                }

                for (i = 0; i < PCM_BUFF_SIZE - u32Len; ++i) {
                    pBuffTx[i] = pBuffTx[i + u32Len];
                }

                pcm_buffer_idx -= u32Len;
            DEBUG_PRINTF("\nT%8x\n", pcm_buffer_idx);
            } else {
                for (i = 0; i < u32Len; i++) {
                    I2S_WRITE_TX_FIFO(I2S, 0x00000000);
                }
            // I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk);
            }
        }
    }

    // I2S RX threshold interrupt
    if (u32status & I2S_STATUS_RXTHF_Msk) {
        if (pgm_state == P_MODE_PLAYBACK) {
            uint32_t u32Len;
            uint16_t *pBuffRx = &pcm_buffer[pcm_buffer_idx];

            if (pcm_buffer_idx < (PCM_BUFF_SIZE - 8)) {
                /* Read Rx FIFO Level */
                u32Len = I2S_GET_RX_FIFO_LEVEL(I2S);

                for (i = 0; i < u32Len; i++) {
                    pBuffRx[i] = I2S_READ_RX_FIFO(I2S) & 0x0000FFFF;
                    DEBUG_PRINTF("R:%8x\n", pBuffRx[i]);
                }

                pcm_buffer_idx += u32Len;
            DEBUG_PRINTF("\nR%d\n", pcm_buffer_idx);

                if (pcm_buffer_idx >= PCM_BUFF_SIZE) {
                    pcm_buffer_idx = 0;
                    DEBUG_PRINTF("x\n");
                }
            } else {
            // I2S_DisableInt(I2S, I2S_IE_RXTHIE_Msk);
            }
        }
    }

    // Quit song
    if (STOP_PLAYING) {
        I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
        return;
    }
}


/* -------------------- */
// Main function
/* -------------------- */
int main(void)
{
    int16_t i = 0;

    SYS_Init();

    // Init perhepherial hardwares
    mlh_init_GPIO_for_led();
    mlh_init_7seg();
    mlh_init_keypad_INT();
    mlh_init_lcd();

    Init_EXTINT();
    Init_Timer0();

    DEBUG_PRINTF("\nInit sd card\n");
    // First init sd card, TODO: If no sd card inserted, only enables playback mode
    init_sdcard_stuff();

    // 7seg effect
    mlh_regist_new_pattern_to_SEG_BUF(17, mlh_pattern_to_7seg_pattern(0xFD));
    seg_effect = true;

    // State machine
    while (1) {
        switch (pgm_state) {
            case P_START:
#if 0
                // Skip welcome page to save time, for development
                pgm_state = P_MODE_SELECT;
#else
                pgm_start();
#endif
                break;

            case P_MODE_SELECT:
                pgm_mode_selection();
                // Close 7seg effect
                if (pgm_state == P_MODE_AUDIO_PLAY || pgm_state == P_MODE_PLAYBACK) {
                    seg_effect = false;
                    cnt_5ms = 0;
                    mlh_reset_7seg_buf(false);
                }
                break;

            case P_MODE_AUDIO_PLAY:
                pgm_audio_play();
                break;

            case P_MODE_PLAYBACK:
                pgm_audio_playback();
                break;

            case P_MODE_AUDIO_RECORDER:
                mlh_clear_lcd_buf();
                mlh_print_line_lcd_buf(0, 0 * 16, 8, "Recorder mode");
                mlh_print_line_lcd_buf(0, 2 * 16, 8, "COMING SOON");
                mlh_show_lcd();
                while (1);
                break;
        }
    }

    while (1);
}


/* -------------------- */
// Functions related to the hardware
/* -------------------- */

/**
 * @brief Initialize I2C, WAU8822, I2S
 * @param sample_rate Sample rate of the audio, to config WAU8822
 */
void init_audio_stuff(uint32_t sample_rate)
{
    uint32_t real_sample_rate;
    // Prevent compiler warning
    (void)real_sample_rate;

    // I2S FS(LRCLK) and BCLK
    GPIO_SetMode(PC, (BIT0 | BIT1), GPIO_MODE_QUASI);
    PC0 = 1;
    PC1 = 1;

    // Init I2C0 to access WAU8822
    Init_I2C();

    // Initialize WAU8822 codec
    WAU8822_Setup();

    // Config sample rate to match the wav file
    WAU8822_ConfigSampleRate(sample_rate);

    // select source from HXT(12MHz)
    CLK_SetModuleClock(I2S_MODULE, CLK_CLKSEL2_I2S_S_HXT, 0);
    CLK_EnableModuleClock(I2S_MODULE);

    // Sample rate config in I2S doesn't matter, because I2S is in slave mode
    real_sample_rate = I2S_Open(I2S, I2S_MODE_SLAVE, 8000, I2S_DATABIT_16, I2S_STEREO, I2S_FORMAT_I2S);
    DEBUG_PRINTF("Real I2S sample rate: %d\n", real_sample_rate);

    // Set MCLK and enable MCLK
    I2S_EnableMCLK(I2S, 12000000);


    NVIC_EnableIRQ(I2S_IRQn);
    NVIC_SetPriority(I2S_IRQn, 1);

    DEBUG_PRINTF("I2S STATUS: 0x%X\n", I2S->STATUS);
    DEBUG_PRINTF("I2S IE: 0x%X\n", I2S->IE);
}

/**
 * @brief Initialize SPI and sd card
 */
void init_sdcard_stuff(void)
{
    WORD rc;

    rc = (WORD)disk_initialize(0);

    // Prevent compiler warning
    (void)rc;

    DEBUG_PRINTF("rc=%d\n", rc);
    disk_read(0, ff_buff, 2, 1);
    f_mount(&FatFs[0], (TCHAR*)mount_path, 1);
}

/**
 * @brief Result type when opening file
 * @param rc The result
 */
void put_rc(FRESULT rc)
{
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0");

    uint32_t i;

    for (i = 0; (i != (UINT)rc) && *p; i++) {
        while (*p++) ;
    }

    DEBUG_PRINTF(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}

/**
 * @brief To open wav file
 * @param fp[in] file pointer
 * @param file_path[in] file path
 * @param header[out] Pointer to the destination to store the wav file header infomations
 */
void open_wav_file(FIL *fp, const char *file_path, wav_header_t *header)
{
    uint32_t status;
    uint32_t wav_header_buf_size;
    FRESULT res;
    unsigned char *tmp_ptr;

    DEBUG_PRINTF("[INFO] Opening %s\n", file_path);

    res = f_open(fp, (TCHAR*)file_path, FA_READ);
    if (res) {
        put_rc(res);
        DEBUG_PRINTF("[ERROR] Mount file system is failed!\n");
        while (1);
    }

    if (res == FR_OK) {
        f_lseek(fp, 0);
        wav_header_buf_size = WAV_HEADER_BUF_SIZE;
        f_read(fp, wav_header_data, wav_header_buf_size, &wav_header_buf_size);
        if (wav_header_buf_size != WAV_HEADER_BUF_SIZE) {
            DEBUG_PRINTF("[ERROR] Reading wav file error\n");
            while (1);
        }
        DEBUG_PRINTF("file opened!!\n");

        status = parse_wav_arr(wav_header_data, &wav_header, (void*)(&tmp_ptr));
        DEBUG_PRINTF("Status of parse_wav: %d\n", status);
        if (status != 0) {
            DEBUG_PRINTF("[ERROR] Fail to parse wav file header\n");
            while (1);
        }

        DEBUG_PRINTF("riff: %x\n", wav_header.riff);
        DEBUG_PRINTF("file size: %d\n", wav_header.file_size);
        DEBUG_PRINTF("format: %d\n", wav_header.format);
        DEBUG_PRINTF("format_chunk_id: %x\n", wav_header.format_chunk_id);
        DEBUG_PRINTF("format_chunk_size: %d\n", wav_header.format_chunk_size);
        DEBUG_PRINTF("audio_format: %d\n", wav_header.audio_format);
        DEBUG_PRINTF("num_of_channels: %d\n", wav_header.num_of_channels);
        DEBUG_PRINTF("sample_rate: %d\n", wav_header.sample_rate);
        DEBUG_PRINTF("byte_per_sec: %d\n", wav_header.byte_per_sec);
        DEBUG_PRINTF("block_align: %d\n", wav_header.block_align);
        DEBUG_PRINTF("bits_per_sample: %d\n", wav_header.bits_per_sample);
        DEBUG_PRINTF("data_chunk_id: %x\n", wav_header.data_chunk_id);
        DEBUG_PRINTF("Data Chunk Found (data size): Size %u bytes\n", wav_header.data_chunk_size);
    }
    DEBUG_PRINTF("[INFO] Wav header successfully parsed\n");
}

/**
 * @brief Close the file
 * @param fp File pointer
 */
void close_wav_file(FIL *fp)
{
    f_close(fp);
}

/**
 * @brief Start to play the song after opening the file and config the WAU8822
 * @param fp File pointer, to the wav file
 */
void start_play(FIL *fp)
{
    // Move to start of the sound data
    f_lseek(fp, 44);
    pcm_buffer_needs_refill = true;
    // I2S_ENABLE_TX(I2S);

    // Start I2S play iteration
    while (1) {
        // Check if needs refill sound data
        if (pcm_buffer_needs_refill) {
            f_read(fp, pcm_buffer, PCM_BUFF_SIZE * sizeof(pcm_buffer[0]), &pcm_buffer_idx);
            pcm_buffer_idx = 0;
            pcm_buffer_needs_refill = false;
            I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
        }

        // Break loop when song ends
        if (wav_header.data_chunk_size == 0) {
            I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
            // I2S_DISABLE_TX(I2S);
            break;
        }

        if (STOP_PLAYING) {
            I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk | I2S_IE_RXTHIE_Msk);
            // I2S_DISABLE_TX(I2S);
            break;
        }
    }
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


/* -------------------- */
// Functions related to the program utility
/* -------------------- */

/**
 * @brief Shows the songs to the LCD for selection
 * @param idx index in the songs array
 * @param line line on the lcd
 */
void show_song_menu(uint16_t idx, uint16_t line)
{ // TODO: top line shows "Song selection", only when 0 <= idx <= 2 ,(only shows 3 lines of option)
    uint16_t i;
    mlh_clear_lcd_buf();
    // Add scroll bar visualliztion effect
    mlh_draw_rectangle_lcd_buf(LCD_Xmax-SCROLL_BAR_WIDTH, idx * LCD_Ymax/wav_file_count, LCD_Xmax-1, (idx+1) * LCD_Ymax/wav_file_count, FG_COLOR, true);
    idx -= line;
    for (i = 0; i < 4; ++i) {
        if (idx + i < wav_file_count) {
            if (i == line) mlh_print_line_lcd_buf(0, i * 16, 8, "> ");
            mlh_print_line_lcd_buf(2 * 8, i * 16, 8, "%s", wav_file_path[idx + i]);
        }
    }
    mlh_show_lcd();
}

/**
 * @brief Shows modes for selection on LCD
 * @param idx index of the mode array
 */
void show_mode_menu(uint16_t idx)
{
    uint16_t i, offset = 8;
    mlh_clear_lcd_buf();
    // Add scroll bar visualliztion effect
    mlh_draw_rectangle_lcd_buf(LCD_Xmax-SCROLL_BAR_WIDTH, idx * LCD_Ymax/MODE_NUM, LCD_Xmax-1, (idx+1) * LCD_Ymax/MODE_NUM, FG_COLOR, true);
    mlh_print_line_lcd_buf(0, 0, 5, "Mode selection");
    for (i = 0; i < 3; ++i) {
        if (i == idx) mlh_print_line_lcd_buf(0, i * 16 + offset, 8, "> ");
        mlh_print_line_lcd_buf(2 * 8, i * 16 + offset, 8, "%s", pgm_mode_name_map[i].name);
    }
    mlh_show_lcd();
}

/**
 * @brief Messages to show when the program starts
 * @details Shows the program name, author, and the usage
 */
void pgm_start(void)
{// Show info
    // Show program name
    mlh_print_line_lcd_buf(0, 0 * 16, 8, "   WAV PLAYER   ");
    mlh_print_line_lcd_buf(0, 1 * 16, 8, "Author          ");
    mlh_print_line_lcd_buf(0, 2 * 16, 8, "    Jorden Huang");
    mlh_print_line_lcd_buf(0, 3*16+8, 5, "Press any key to continue...");
    mlh_show_lcd();
    while (1) {
        bool start = false;
        switch (mlh_get_key_state()) {
        case K_DOWN:
            start = true;
            break;
        }
        if (start) break;
    }
    mlh_clear_lcd_buf();

    // Show usage
    mlh_print_line_lcd_buf(0, 0 * 16, 8, "     2 to UP    ");
    mlh_print_line_lcd_buf(0, 0 * 16, 5, "Usage");
    mlh_print_line_lcd_buf(0, 1 * 16, 8, "     5 to select");
    mlh_print_line_lcd_buf(0, 2 * 16, 8, "     8 to DOWN  ");
    mlh_print_line_lcd_buf(0, 3*16,   5, "Use INT1 to quit song ");
    mlh_print_line_lcd_buf(0, 3*16+8, 5, "Press any key to start");
    mlh_show_lcd();
    while (1) {
        bool start = false;
        switch (mlh_get_key_state()) {
        case K_DOWN:
            start = true;
            break;
        }
        if (start) break;
    }

    // tranform to next state
    pgm_state = P_MODE_SELECT;
}

/**
 * @brief Hanlder the mode selection
 */
void pgm_mode_selection(void)
{
    int16_t idx = 0;
    bool selected = false;
    while (1) {
        // Show mode menu
        show_mode_menu(idx);

        switch (mlh_get_key_state()) {
        case K_DOWN:
            if (KEY_FLAG == 1 || KEY_FLAG == 2) {
                // Up
                if (idx > 0) {
                    idx -= 1;
                }
            } else if (KEY_FLAG == 7 || KEY_FLAG == 8) {
                // Down
                if (idx < MODE_NUM-1) {
                    idx += 1;
                }
            } else if (KEY_FLAG == 5) {
                pgm_state = pgm_mode_name_map[idx].mode;
                selected = true;
            }
            break;
        case K_UP:
        case K_PRESSING:
        case K_NO_PRESSING:
            break;
        }

        if (selected) break;
    }
}

/**
 * @brief State handler for audio player mode
 * @details Let the user select the song, and play it
 */
void pgm_audio_play(void)
{
    int16_t idx = 0, line = 0;
    bool user_selected = false;
    mlh_set_7seg_buf(0, 0);

    while (1) {
        show_song_menu(idx, line);

        switch (mlh_get_key_state()) {
        case K_DOWN:
            if (KEY_FLAG == 1 || KEY_FLAG == 2) {
                // Up
                if (idx > 0) {
                    idx -= 1;
                    if (line != 0) line -= 1;
                }
            } else if (KEY_FLAG == 7 || KEY_FLAG == 8) {
                // Down
                if (idx < (wav_file_count - 1)) {
                    idx += 1;
                    if (line != 3) line += 1;
                }
            } else if (KEY_FLAG == 5) {
                user_selected = true;
                // Invert the region
                mlh_invert_region_lcd_buf(2 * 8, line * 16 + 1, 8 * strlen(wav_file_path[idx]), 15);
                mlh_show_lcd();
            }
            break;
        case K_UP:
            if (KEY_FLAG_LAST == 4) {
                DEBUG_PRINTF("KEY 4 UP\n");
            }
            break;
        case K_PRESSING:
            if (KEY_FLAG == 4) { }
            break;
        case K_NO_PRESSING:
            break;
        }

        if (user_selected) {
            DEBUG_PRINTF("\nOpen wav file\n");
            // Then read the file
            open_wav_file(&fp, wav_file_path[idx], &wav_header);

            DEBUG_PRINTF("\nInit audio stuff\n");
            // After reading the file, init audio stuff
            // (Must be after reading the file, because it needs to be configured using the sample rate)
            init_audio_stuff(wav_header.sample_rate);

            start_count = true;
            {
                DEBUG_PRINTF("\nStart play\n");
                start_play(&fp);
                if (STOP_PLAYING) STOP_PLAYING = false;
            }
            start_count = false;

            DEBUG_PRINTF("\nClose wav file\n");
            close_wav_file(&fp);

            cnt_5ms = 0;
            user_selected = false;

            I2C_Close(I2C0);
            I2S_Close(I2S);
            CLK_DisableModuleClock(I2S_MODULE);
            I2S_DisableMCLK(I2S);
            NVIC_DisableIRQ(I2S_IRQn);

            mlh_reset_7seg_buf(false);
            mlh_set_7seg_buf(0, 0);
        }
    }
}

/**
 * @brief State handler for audio playback mode
 * @note NOT yet finished
 */
void pgm_audio_playback(void)
{
    bool start_flag = true;
    pcm_buffer_idx = 0;

    mlh_clear_lcd_buf();
    mlh_print_line_lcd_buf(0, 0 * 16, 8, "Playback mode");
    mlh_print_line_lcd_buf(0, 2 * 16, 8, "COMING SOON");
    mlh_show_lcd();

    DEBUG_PRINTF("\nInit audio stuff\n");
    init_audio_stuff(PLAYBACK_SAMPLE_RATE);

    // Enable RX
    I2S_EnableInt(I2S, I2S_IE_RXTHIE_Msk);
    // I2S_ENABLE_RX(I2S);
    // I2S_SET_MONO_RX_CHANNEL(I2S, I2S_MONO_LEFT);

    start_count = true;
    while (1) {
        // if (pcm_buffer_idx < 8) {
        //     I2S_DisableInt(I2S, I2S_IE_TXTHIE_Msk);
        //     I2S_DISABLE_TX(I2S);
        //         I2S_EnableInt(I2S, I2S_IE_RXTHIE_Msk);
        //         I2S_ENABLE_RX(I2S);

        // }
        // else if (pcm_buffer_idx >= PCM_BUFF_SIZE-8) {
        //     I2S_DisableInt(I2S, I2S_IE_RXTHIE_Msk);
        //     I2S_DISABLE_RX(I2S);
        //         I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk);
        //         I2S_ENABLE_TX(I2S);
        // }
        if (start_flag) {
            /* Enable I2S Tx function to send data when data in the buffer is more than half of buffer size */
            if (pcm_buffer_idx >= PCM_BUFF_SIZE / 2) {
                I2S_EnableInt(I2S, I2S_IE_TXTHIE_Msk);
                I2S_ENABLE_TX(I2S);
                start_flag = false;
                DEBUG_PRINTF("e\n");
            }
        }

        if (STOP_PLAYING) {
            break;
        }
    }
    start_count = false;

    cnt_5ms = 0;
    mlh_reset_7seg_buf(false);
    mlh_set_7seg_buf(0, 0);
}
