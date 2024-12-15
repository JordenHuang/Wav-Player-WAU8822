/**
 * @author: Jorden Huang
 * @version: 0.1.0
 * @desription:
 *   My library for Nu-LB-NUC140 board.
 *   This library is a 'single header library', just like the stb library
 *   The functions are begin with a prefix, "mlh", which is short for "my library's header_file".
 *   The naming in functions is: mlh + verb + noun, written in snake case, example: mlh_show_lcd(), or mlh_close_7seg()
 * @usage:
 *   Define the utility (macros), then include this library.
 *   The macros:
 *     MLH_LED
 *     MLH_KEYPAD
 *     MLH_KEYPAD_INT
 *     MLH_7SEG
 *     MLH_7SEG_INT
 *     MLH_LCD
 *     MLH_LCD_DYNAMIC_UPDATE (need MLH_LCD declared)
 *     MLH_PWM_BUZZER
 *     MLH_ADC_VR
 *     MLH_UART
 * 
 *   Example:
 *     #define MLH_7SEG // or #define MLH_7SEG_INT to use timer interrupt to update 7seg
 *     #define MLH_KEYPAD // or #define MLH_KEYPAD_INT to use gpio interrupt when scan key
 *     #define MLH_LCD
 *     #define MLH_LCD_DYNAMIC_UPDATE
 *     #include "myLib.h"
 *     int main(void) {
 *         mlh_init_7seg();
 *         mlh_init_keypad(); // or mlh_init_keypad_INT() if using gpio interrupt
 *         mlh_init_lcd();
 *         ... // other initialization functions
 *         mlh_clear_lcd_buf(); // manually clear lcd buf
 *         while (1) { ... }
 *     }
 * @note
 *   (1) If use 7seg interrupt, then below needs to add to MCU_init.h
 *     ```c
 *     // Timer 3
 *     #define MCU_INTERFACE_TMR3
 *     #define TMR3_CLOCK_SOURCE_HXT // HXT, LXT, HCLK, EXT, LIRC, HIRC
 *     #define TMR3_CLOCK_DIVIDER    1
 *     #define TMR3_OPERATING_MODE   TIMER_PERIODIC_MODE // ONESHOT, PERIODIC, TOGGLE, CONTINUOUS
 *     #define TMR3_OPERATING_FREQ   200  //Hz, Equals 5ms
 *     ```
 *   (2) 7 seg and keypad can use interrupt to work easily, no need to manually call in while loop
 * @example
 *   example of MCU_init.h
 *     ```c {
 *     //Define Clock source
 *     #define MCU_CLOCK_SOURCE
 *     #define MCU_CLOCK_SOURCE_HXT  // HXT, LXT, HIRC, LIRC
 *     #define MCU_CLOCK_FREQUENCY   50000000  // Hz
 * 
 *     //Define MCU Interfaces
 *     // LCD
 *     #define MCU_INTERFACE_SPI3
 *     #define SPI3_CLOCK_SOURCE_HCLK // HCLK, PLL
 *     #define PIN_SPI3_SS0_PD8
 *     #define PIN_SPI3_SCLK_PD9
 *     #define PIN_SPI3_MISO0_PD10
 *     #define PIN_SPI3_MOSI0_PD11
 * 
 *     // Timer 3, dedicate for 7seg
 *     #define MCU_INTERFACE_TMR3
 *     #define TMR3_CLOCK_SOURCE_HXT // HXT, LXT, HCLK, EXT, LIRC, HIRC
 *     #define TMR3_CLOCK_DIVIDER    1
 *     #define TMR3_OPERATING_MODE   TIMER_PERIODIC_MODE // ONESHOT, PERIODIC, TOGGLE, CONTINUOUS
 *     #define TMR3_OPERATING_FREQ   200  //Hz, Equals 5ms
 * 
 *     // Timer 0
 *     #define MCU_INTERFACE_TMR0
 *     #define TMR0_CLOCK_SOURCE_HXT // HXT, LXT, HCLK, EXT, LIRC, HIRC
 *     #define TMR0_CLOCK_DIVIDER    1
 *     #define TMR0_OPERATING_MODE   TIMER_PERIODIC_MODE // ONESHOT, PERIODIC, TOGGLE, CONTINUOUS
 *     #define TMR0_OPERATING_FREQ   200  // Hz, Equals 5 ms
 * 
 *     // PWM output to Buzzer
 *     #define MCU_INTERFACE_PWM1
 *     #define PWM1_CH01_CLOCK_SOURCE_HXT
 *     #define PIN_PWM1_CH0_PB11
 * 
 *     // ADC7, for variable resistor
 *     #define MCU_INTERFACE_ADC
 *     #define ADC_CLOCK_SOURCE_HXT // HXT, LXT, PLL, HIRC, HCLK
 *     #define ADC_CLOCK_DIVIDER    1
 *     #define PIN_ADC7_PA7
 *     #define ADC_CHANNEL_MASK      ADC_CH_7_MASK
 *     #define ADC_INPUT_MODE        ADC_INPUT_MODE_SINGLE_END // SINGLE_END, DIFFERENTIAL
 *     #define ADC_OPERATION_MODE    ADC_OPERATION_MODE_SINGLE // SINGLE, SINGLE_CYCLE, CONTINUOUS
 * 
 *     // UART 0
 *     #define MCU_INTERFACE_UART0
 *     #define UART_CLOCK_SOURCE_HXT // HXT, LXT, PLL, HIRC
 *     #define UART_CLOCK_DIVIDER     3
 *     #define PIN_UART0_RX_PB0
 *     #define PIN_UART0_TX_PB1
 *     } ```
 */
#ifndef _MY_LIB_H_
#define _MY_LIB_H_

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "NUC100Series.h"
#include "GPIO.h"
#include "SYS.h"
#include "SPI.h"

/***********************/
// Function prototypes {
/***********************/

int32_t mlh_calc_digit_length(int32_t num);

void mlh_init_GPIO_for_led(void);
void mlh_set_and_close_others_led(uint8_t no);
void mlh_set_single_led(uint8_t no, bool turn_on);

void mlh_init_keypad(void);
uint8_t mlh_scan_key(void);
void mlh_init_keypad_INT(void);
enum Key_state mlh_get_key_state(void);

void mlh_init_7seg(void);
void mlh_regist_new_pattern_to_SEG_BUF(uint8_t index, uint8_t pattern);
void mlh_set_7seg_buf(uint8_t no, uint8_t number);
void mlh_reset_7seg_buf(bool fill_zero);
void mlh_show_digit_7seg(uint8_t no, uint8_t number);
void mlh_show_buf_7seg(uint8_t no);
void mlh_close_7seg(void);
uint8_t mlh_pattern_to_7seg_pattern(uint8_t pattern);
void mlh_show_7seg_with_pattern(uint8_t no, uint8_t pattern);
void mlh_number_to_seg_buf(uint16_t number, bool fill_zero);
uint16_t mlh_seg_buf_to_number(void);

void mlh_init_SPI3(void);
void mlh_lcdWriteCommand(unsigned char temp);
void mlh_lcdWriteData(uint8_t temp);
void mlh_lcdSetAddr(uint8_t PageAddr, uint8_t ColumnAddr);
void mlh_init_lcd(void);
void mlh_clear_lcd_only(void);
void mlh_clear_lcd_buf(void);
void mlh_show_lcd(void);
void mlh_show_lcd_16_round(bool *done_flag);

void mlh_invert_region_lcd_buf(uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px);

void mlh_draw_pixel_lcd_buf(uint16_t x, uint16_t y, uint16_t color);
void mlh_draw_byte_lcd_buf(const uint8_t pattern, uint16_t x, uint16_t y, uint16_t color);
void mlh_draw_bitmap_lcd_buf(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px, uint16_t color, bool force_draw);
void mlh_draw_bitmap_scaled_lcd_buf(const uint8_t* bitmap, uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px, uint16_t color, uint8_t scale, bool force_draw);
void mlh_draw_line_lcd_buf(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t color);
void mlh_draw_rectangle_lcd_buf(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, bool fill);
void mlh_draw_circle_lcd_buf(int16_t xc, int16_t yc, int16_t r, uint16_t color, bool fill);
void mlh_draw_triangle_lcd_buf(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

void mlh_print_char_lcd_buf(uint16_t x, uint16_t y, uint8_t font_size, unsigned char ascii_code);
void mlh_print_line_lcd_buf(uint16_t x, uint16_t y, uint8_t size, const char *format, ...);

void mlh_print_line_8_round_lcd(uint8_t line, char *text, bool *done_flag);

void Init_Timer3(void);

void mlh_set_output_pwm_buzzer(bool enable);
void mlh_init_pwm_buzzer(uint32_t freq, uint32_t duty_cycle, bool enable);

void mlh_init_adc_vr(void);
uint32_t mlh_get_value_adc(void);

void mlh_init_uart(void);
bool mlh_is_rx_ready(void);
void mlh_clear_rx_flag(void);
void mlh_write_text_uart(char *text);
void mlh_read_text_uart(char *dest);
void mlh_write_int_uart(int32_t num);
int32_t mlh_read_int_uart(void);
void mlh_write_format_text_uart(const char *format, ...);

/***********************/
// End Function prototypes }
/***********************/

/***********************/
// FUNCTION IMPLEMENTATIONS
/***********************/

/***********************/
// Utility function {
/***********************/

/**
 * @brief Calculate length of a number as if it's a string
 * @param num The number to calculate
 * @return The length
 * @note The return type should not be unsigned
 */
int32_t mlh_calc_digit_length(int32_t num)
{
    uint32_t len = 0;
    if (num < 0) num = -num;
    while (num > 0) {
        num /= 10;
        len += 1;
    }
    return len;
}

/***********************/
// End Utility function }
/***********************/

/***********************/
// LED {
/***********************/
#ifdef MLH_LED

#define mlh_turn_on_all_led() \
    do {                      \
        PC12 = 0;             \
        PC13 = 0;             \
        PC14 = 0;             \
        PC15 = 0;             \
    } while (0)
#define mlh_turn_off_all_led() \
    do {                       \
        PC12 = 1;              \
        PC13 = 1;              \
        PC14 = 1;              \
        PC15 = 1;              \
    } while (0)

/**
 * @brief Initialize the leds
 */
void mlh_init_GPIO_for_led(void)
{
    GPIO_SetMode(PC, (BIT12 | BIT13 | BIT14 | BIT15), GPIO_MODE_OUTPUT);
    mlh_turn_off_all_led();
}

/**
 * @brief Set one of the led to on and close others
 * @param no which led, valid value: 0 to 3 (the leftmost led is 0)
 */
void mlh_set_and_close_others_led(uint8_t no)
{
    uint8_t i;
    for (i = 0; i < 4; i++) {
        // PC12 == GPIO_PIN_DATA(2, 12)
        GPIO_PIN_DATA(2, 12 + i) = (i == no) ? 0 : 1;
    }
}

/**
 * @brief set one led to on/off
 * @param no which led, valid value: 0 to 3
 * @param turn_on true: turn on the no. led. false: turn off the no. led
 */
void mlh_set_single_led(uint8_t no, bool turn_on)
{
    GPIO_PIN_DATA(2, 12 + no) = (turn_on == true) ? 0 : 1;
}

#endif // MLH_LED
/***********************/
// End LED }
/***********************/

/***********************/
// keypad {
/***********************/
#ifdef MLH_KEYPAD

/**
 * @brief Initialize the keypad
 */
void mlh_init_keypad(void)
{
    GPIO_SetMode(PA, (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5), GPIO_MODE_QUASI);
}

/**
 * @brief [Internal function] Scan key from keypad
 * @return the key that is pressed. Key number: top-left is 1, bottom-right is 9
 */
uint8_t __mlh_scan_key(void)
{
/*
    In quasi-bidirectional mode, if you want to read the state of a GPIO pin, you would first write a 1 to the pin. Writing a 1 activates the weak pull-up resistor, allowing the pin to act as an input. Now you can read the pin's state.
    If there’s no external device pulling it low, it will read as 1 (due to the pull-up). But if an external device (like a button or another component) pulls the pin low, it will read as 0.
*/
    PA0 = 1; PA1 = 1; PA2 = 0; PA3 = 1; PA4 = 1; PA5 = 1;
    if (PA3 == 0) return 1;
    if (PA4 == 0) return 4;
    if (PA5 == 0) return 7;
    PA0 = 1; PA1 = 0; PA2 = 1; PA3 = 1; PA4 = 1; PA5 = 1;
    if (PA3 == 0) return 2;
    if (PA4 == 0) return 5;
    if (PA5 == 0) return 8;
    PA0 = 0; PA1 = 1; PA2 = 1; PA3 = 1; PA4 = 1; PA5 = 1;
    if (PA3 == 0) return 3;
    if (PA4 == 0) return 6;
    if (PA5 == 0) return 9;
    return 0;
}

// TODO: add control to the mode, on press, on leave, etc.
/**
 * @brief Scan key. Only want the key when the button is released
 * @return The key that is pressed
 */
uint8_t mlh_scan_key(void)
{
    static uint8_t LAST_KEY = 0;
    uint8_t curKey, theKey;
    curKey = __mlh_scan_key();
    if (curKey == 0 && LAST_KEY != 0) {
        theKey = LAST_KEY;
    }
    else {
        theKey = 0;
    }
    LAST_KEY = curKey;
    return theKey;
}

#endif // MLH_KEYPAD
/***********************/
// End keypad }
/***********************/

/***********************/
// keypad with interrupt {
/***********************/
#ifdef MLH_KEYPAD_INT

enum Key_state {
    K_DOWN,
    K_PRESSING,
    K_UP,
    K_NO_PRESSING
};

bool KEY_CHANGED = false;
uint8_t KEY_FLAG = 0;
uint8_t KEY_FLAG_LAST = 0;

/**
 * @brief Interrupt service routine for keypad
 * @note Assume only one key will be pressed at a time
 * @note Keypad pin layout:
 *       PA2 PA1 PA0
 *   PA3  1   2   3
 *   PA4  4   5   6
 *   PA5  7   8   9
 */
void GPAB_IRQHandler(void)
{
    uint8_t which_pin;
    // The ISRC register holds interrupt status flags for each pin.
    // Writing a 1 to a bit in ISRC clears the interrupt flag for that pin.
    if (PA->ISRC & BIT0) { which_pin = 0; } // Check if the interrupt is from PA0 and clear PA0 interrupt
    if (PA->ISRC & BIT1) { which_pin = 1; }
    if (PA->ISRC & BIT2) { which_pin = 2; }

    // KEY_FLAG_LAST may be used if checking KEY_UP state
    KEY_FLAG_LAST = KEY_FLAG;
    // Get previous key state and update current key state
    switch (mlh_get_key_state()) {
    case K_DOWN:
    case K_PRESSING:
        KEY_FLAG = 0;
        break;
    case K_UP:
    case K_NO_PRESSING:
        GPIO_PIN_DATA(0, which_pin) = 0;
        PA3 = 1; PA4 = 1; PA5 = 1;
        if (PA3 == 0) { KEY_FLAG = 3 - which_pin; }
        if (PA4 == 0) { KEY_FLAG = 6 - which_pin; }
        if (PA5 == 0) { KEY_FLAG = 9 - which_pin; }
        break;
    }
    // Update KEY_CHANGED flag after the mlh_get_key_state call
    KEY_CHANGED = true;

    PA0 = 1; PA1 = 1; PA2 = 1; PA3 = 0; PA4 = 0; PA5 = 0;
    PA->ISRC = PA->ISRC; // clear all GPA pins
}

/**
 * @brief Initialize the keypad that uses interrupt
 * @details Interrupt occurs when falling edge, i.e. the moment the key is pressed
 */
void mlh_init_keypad_INT(void)
{
    GPIO_SetMode(PA, (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5), GPIO_MODE_QUASI);
    // Using PA[0, 1, 2] to detect interrupt (see which one becomes 0)
    // and use PA[3, 4, 5] to check which key is pressed (Not the same as mlh_init_keypad)
    GPIO_EnableInt(PA, 0, GPIO_INT_BOTH_EDGE);
    GPIO_EnableInt(PA, 1, GPIO_INT_BOTH_EDGE);
    GPIO_EnableInt(PA, 2, GPIO_INT_BOTH_EDGE);
    NVIC_EnableIRQ(GPAB_IRQn);
    NVIC_SetPriority(GPAB_IRQn, 3); // Check this
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_LIRC, GPIO_DBCLKSEL_128);
    GPIO_ENABLE_DEBOUNCE(PA, (BIT0 | BIT1 | BIT2));
    PA0 = 1; PA1 = 1; PA2 = 1; PA3 = 0; PA4 = 0; PA5 = 0;
}

enum Key_state mlh_get_key_state(void)
{
    if (KEY_CHANGED) {
        KEY_CHANGED = false;
        return (KEY_FLAG == 0) ? K_UP : K_DOWN;
    } else {
        return (KEY_FLAG == 0) ? K_NO_PRESSING : K_PRESSING;
    }
}

#endif // MLH_KEYPAD_INT
/***********************/
// End keypad with interrupt }
/***********************/

/***********************/
// 7 segment display {
/***********************/
#if defined(MLH_7SEG) || defined(MLH_7SEG_INT)

/*
0: 10000010
1: 11101110
2: 00000111
3: 01000110
4: 01101010
5: 01010010
6: 00010010
7: 11100110
8: 00000010
9: 01100010
A: 00100010
b: 00011010
C: 10010011
d: 00001110
E: 00010011
F: 00110011
   gedbafhc

 _       a
|_|  -> fgb
|_|.    edch

Segments in byte presentation:
gedbafhc

PE7 ~ PE0
*/
#define SEG_N0 0x82
#define SEG_N1 0xEE
#define SEG_N2 0x07
#define SEG_N3 0x46
#define SEG_N4 0x6A
#define SEG_N5 0x52
#define SEG_N6 0x12
#define SEG_N7 0xE6
#define SEG_N8 0x02
#define SEG_N9 0x62
#define SEG_N10 0x22
#define SEG_N11 0x1A
#define SEG_N12 0x93
#define SEG_N13 0x0E
#define SEG_N14 0x13
#define SEG_N15 0x33
#define SEG_EMPTY 0xFF

#define SEG_NEGATIVE 0x7F

uint8_t _mlh_SEG_BUF[20] = {
    SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_N10, SEG_N11, SEG_N12, SEG_N13, SEG_N14, SEG_N15,
    SEG_EMPTY, SEG_EMPTY, SEG_EMPTY, SEG_EMPTY
}; // ^ 17th   ^ 18th

/**
 * @brief Buffer for 7seg data
 * @details index 0 is the rightmost 7seg, index 3 is the leftmost 7seg on the board
 */
uint8_t mlh_seg_buf[4] = {16, 16, 16, 16};

/**
 * @brief Initialize seven segment display
 * @details
 *   If defined MLH_7SEG_INT, then there is no need to manually call mlh_show_7seg() to show pattern on 7seg.
 *   But the macros for TIMER 3 needs to be define in MCU_init.h
 */
void mlh_init_7seg(void)
{
    GPIO_SetMode(PC, (BIT4 | BIT5 | BIT6 | BIT7), GPIO_PMD_OUTPUT);
    PC4 = 0; PC5 = 0; PC6 = 0; PC7 = 0;
    GPIO_SetMode(PE, (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7), GPIO_PMD_QUASI);
    PE0 = 0; PE1 = 0; PE2 = 0; PE3 = 0; PE4 = 0; PE5 = 0; PE6 = 0; PE7 = 0;
#ifdef MLH_7SEG_INT
    Init_Timer3();
#endif
}

/**
 * @brief Regist a new pattern to _mlh_SEG_BUF
 * @param index Position in _mlh_SEG_BUF, starts from 17
 * @param pattern The pattern to regist
 */
void mlh_regist_new_pattern_to_SEG_BUF(uint8_t index, uint8_t pattern)
{
    if (index < 17 && index > (sizeof(_mlh_SEG_BUF)/sizeof(_mlh_SEG_BUF[0]))) return;
    _mlh_SEG_BUF[index] = pattern;
}

/**
 * @brief Set mlh_seg_buf[no] to number
 * @param no Index in mlh_seg_buf, valid values: 3 to 0, leftmost 7seg is 3
 * @param number The number, which is the index of _mlh_SEG_BUF
 */
void mlh_set_7seg_buf(uint8_t no, uint8_t number)
{
    mlh_seg_buf[no] = number;
}

/**
 * @brief Reset mlh_seg_buf, set all to 16 (or 0 if fill_zero is true)
 * @param fill_zero Set all digit to 0, rather than 16 (empty digit)
 */
void mlh_reset_7seg_buf(bool fill_zero)
{
    uint8_t i, d;
    d = fill_zero == true ? 0 : 16;
    for (i=0; i<4; ++i) mlh_seg_buf[i] = d;
}

/**
 * @brief Show number on the no. 7seg
 * @param no which 7seg, valid value: 3 to 0. Leftmost one is 3
 * @param number the number to show, valid value: 0~16, 16 is all empty
 * @note If you have register new pattern, then use the index as @number for that pattern
 * @note Useless if MLH_7SEG_INT is set
 */
void mlh_show_digit_7seg(uint8_t no, uint8_t number)
{
    PE->DOUT = _mlh_SEG_BUF[number];
    GPIO_PIN_DATA(2, 4 + no) = 1;
}

/**
 * @brief Show the digit at mlh_seg_buf[no]
 * @param no which 7seg, and the index in mlh_seg_buf
 * @note Useless if MLH_7SEG_INT is set
 */
void mlh_show_buf_7seg(uint8_t no)
{
    PE->DOUT = _mlh_SEG_BUF[mlh_seg_buf[no]];
    GPIO_PIN_DATA(2, 4 + no) = 1;
}

/**
 * @brief Close all four 7seg
 * @note Useless if MLH_7SEG_INT is set
 */
void mlh_close_7seg(void)
{
    PC4 = 0; PC5 = 0; PC6 = 0; PC7 = 0;
}

/**
 * @brief Trun pattern to match 7seg's pin order
 * @param pattern The pattern to tranform, the order: abcdefgh, 0 is on
 */
uint8_t mlh_pattern_to_7seg_pattern(uint8_t pattern)
{
    // gedbafhc
    uint8_t output = 0x00;
    // pattern example: 11110111
    // bit a
    // output |= (pattern & (1 << 7)) >> 4; // This is original
    output |= (pattern & 0x80) >> 4;
    // bit b
    output |= (pattern & 0x40) >> 2;
    // bit c
    output |= (pattern & 0x20) >> 5;
    // bit d
    output |= (pattern & 0x10) << 1;
    // bit e
    output |= (pattern & 0x08) << 3;
    // bit f
    output |= (pattern & 0x04);
    // bit g
    output |= (pattern & 0x02) << 6;
    // bit h
    output |= (pattern & 0x01) << 1;

    /* FIXME: How about
    if ((pattern&0x80) output |= 0x08;
    if ((pattern&0x40) output |= 0x10;
    ...
    */

    return output;
}

/**
 * @brief Show pattern on 7seg
 * @param no Which 7seg, valid value: 3 to 0. Leftmost one is 3
 * @param pattern The pattern to display, the order: abcdefgh, 0 is on
 */
void mlh_show_7seg_with_pattern(uint8_t no, uint8_t pattern)
{
    PE->DOUT = mlh_pattern_to_7seg_pattern(pattern);
    switch (no) {
    case 0: PC4 = 1; break;
    case 1: PC5 = 1; break;
    case 2: PC6 = 1; break;
    case 3: PC7 = 1; break;
    }
}


/**
 * @brief Turn number to digits, store in mlh_seg_buf
 * @param number The number to transform
 * @param fill_zero Fill 0 to empty digits
 */
void mlh_number_to_seg_buf(uint16_t number, bool fill_zero)
{
    uint8_t i;
    int32_t len = mlh_calc_digit_length(number);
    for (i=0; i<4; ++i) {
        if (i > (len - 1)) {
            if (number % 10 == 0) {
                mlh_seg_buf[i] = (fill_zero == true) ? 0 : 16;
            }
        } else {
            mlh_seg_buf[i] = number % 10;
        number /= 10;
        }
    }
}
/**
 * @brief Turn content in mlh_seg_buf to number
 * @return number
 */
uint16_t mlh_seg_buf_to_number(void)
{
    uint16_t num = 0;
    int8_t i;
    for (i=3; i>=0; --i) {
        num *= 10;
        if (mlh_seg_buf[i] != 16) num += mlh_seg_buf[i];
        else break;
    }
    return num;
}

#endif // defined(MLH_7SEG) || defined(MLH_7SEG_INT)
/***********************/
// End 7 segment display }
/***********************/

/***********************/
// LCD {
/***********************/
#ifdef MLH_LCD

#include "Font5x7.h"
#include "Font8x16.h"

#define LCD_Xmax 128
#define LCD_Ymax 64
// Do not change the color definition (the code will break), FG_COLOR and BG_COLOR should be complement
#define FG_COLOR 0xFFFF
#define BG_COLOR 0x0000

#if defined(MLH_LCD_DYNAMIC_UPDATE)
/**
 * @brief [Internal variable] Store the current data that display on the LCD
 */
uint8_t _mlh_current_data_in_lcd[128 * 8];
#endif // defined(MLH_LCD_DYNAMIC_UPDATE)

/**
 * @brief Store the data to be shown on the LCD
 */
uint8_t mlh_lcd_buffer[128 * 8];

/**
 * @brief Initialize the SPI portocol, LCD uses this protocol to communicate with.
 */
void mlh_init_SPI3(void)
{
    /* Configure as a master, clock idle low, 9-bit transaction, drive output on falling clock edge and latch input on rising edge. */
    /* Set IP clock divider. SPI clock rate = 1MHz */
    SPI_Open(SPI3, SPI_MASTER, SPI_MODE_0, 9, 1000000);
    SPI_DisableAutoSS(SPI3);
}

/**
 * @brief Write command to LCD
 * @param temp The command to write
 */
void mlh_lcdWriteCommand(unsigned char temp)
{
    SPI_SET_SS0_LOW(SPI3);
    SPI_WRITE_TX0(SPI3, temp); // Write Data
    SPI_TRIGGER(SPI3);         // Trigger SPI data transfer
    while (SPI_IS_BUSY(SPI3))
        ; // Check SPI3 busy status
    SPI_SET_SS0_HIGH(SPI3);
}

/**
 * @brief Write data to LCD
 * @param temp The data to write
 */
void mlh_lcdWriteData(uint8_t temp)
{
    SPI_SET_SS0_LOW(SPI3);
    SPI_WRITE_TX0(SPI3, 0x100 + temp); // Write Data
    SPI_TRIGGER(SPI3);                 // Trigger SPI data transfer
    while (SPI_IS_BUSY(SPI3))
        ; // Check SPI3 busy status
    SPI_SET_SS0_HIGH(SPI3);
}

/**
 * @brief Set the address of LCD
 * @param PageAddr Page address, 0 to 8
 * @param ColumnAddr Column address, 0 to 127
 */
void mlh_lcdSetAddr(uint8_t PageAddr, uint8_t ColumnAddr)
{
    // Set PA (Set page address, 1011____, _ means data bits)
    SPI_SET_SS0_LOW(SPI3);
    SPI_WRITE_TX0(SPI3, 0xB0 | PageAddr); // Write Data
    SPI_TRIGGER(SPI3);                    // Trigger SPI data transfer
    while (SPI_IS_BUSY(SPI3))
        ; // Check SPI3 busy status
    SPI_SET_SS0_HIGH(SPI3);
    // Set CA MSB (Set column address MSB, 0001____)
    SPI_SET_SS0_LOW(SPI3);
    SPI_WRITE_TX0(SPI3, 0x10 | (ColumnAddr >> 4) & 0xF); // Write Data
    SPI_TRIGGER(SPI3);                                   // Trigger SPI data transfer
    while (SPI_IS_BUSY(SPI3))
        ; // Check SPI3 busy status
    SPI_SET_SS0_HIGH(SPI3);
    // Set CA LSB // (Set column address LSB, 0000____)
    SPI_SET_SS0_LOW(SPI3);
    SPI_WRITE_TX0(SPI3, 0x00 | (ColumnAddr & 0xF)); // Write Data
    SPI_TRIGGER(SPI3);                              // Trigger SPI data transfer
    while (SPI_IS_BUSY(SPI3))
        ; // Check SPI3 busy status
    SPI_SET_SS0_HIGH(SPI3);
}

/**
 * @brief Initialize LCD
 */
// TODO: backlight setting
// TODO: boolean option for clearing lcd and lcd buf
void mlh_init_lcd(void)
{
    mlh_init_SPI3();
    mlh_lcdWriteCommand(0xEB); // Set LCD Bias Ratio
    mlh_lcdWriteCommand(0x81); // Set V_BIAS Potentiometer (double-byte command)
    mlh_lcdWriteCommand(0xA0); // Set Frame Rate
    mlh_lcdWriteCommand(0xC0); // Set LCD Mapping Control
    mlh_lcdWriteCommand(0xAF); // Set Display Enable

    mlh_clear_lcd_only();
    mlh_clear_lcd_buf();
}

/**
 * @brief Clear the LCD, but not the lcd buffer
 */
void mlh_clear_lcd_only(void)
{
    uint16_t i;
    // column address and page address will automatically increment
    mlh_lcdSetAddr(0x00, 0x00);
    for (i = 0; i < 132 * 8; i++) {
        mlh_lcdWriteData(0x00);
    }

#if defined(MLH_LCD_DYNAMIC_UPDATE)
    // Clear buffer for current lcd content
    memset(_mlh_current_data_in_lcd, 0, sizeof(_mlh_current_data_in_lcd));
#endif // defined(MLH_LCD_DYNAMIC_UPDATE)
}

/**
 * @brief Clear the LCD buffer
 */
void mlh_clear_lcd_buf(void)
{
    memset(mlh_lcd_buffer, 0, sizeof(mlh_lcd_buffer));
}

/**
 * @brief Show the data in the LCD buffer to LCD
 * @note
 *   leftmost CA is 129, rightmost CA is 2
 * 
 *   CPU跟記憶體交換資料 和
 *   CPU跟LCD交換資料 哪一個比較快?
 *   我自己覺得是前者會快很多，然後我覺得後者慢的原因是因為LCD用的是peripheral interface，走APB bus，所以還多一個bridge跟AHB bus連接才能跟CPU交換資料
 *   而不像led或七段燈是直接GPIO控制的，都在AHB bus
 */
void mlh_show_lcd(void)
{
    uint16_t x, y, idx_in_buf;
    bool flag;
    for (y = 0; y < (LCD_Ymax / 8); ++y) {
        mlh_lcdSetAddr(y, 0x02); // Write from the right of the lcd
        flag = false;
        for (x = 0; x < LCD_Xmax; ++x) {
            idx_in_buf = y * LCD_Xmax + ((LCD_Xmax - 1) - x);

#if defined(MLH_LCD_DYNAMIC_UPDATE)
            if (mlh_lcd_buffer[idx_in_buf] != _mlh_current_data_in_lcd[idx_in_buf]) {
                if (flag == true) {
                    mlh_lcdSetAddr(y, x + 2);
                    flag = false;
                }
                mlh_lcdWriteData(mlh_lcd_buffer[idx_in_buf]);
                _mlh_current_data_in_lcd[idx_in_buf] = mlh_lcd_buffer[idx_in_buf];
            } else {
                flag = true;
            }
#else
            mlh_lcdWriteData(mlh_lcd_buffer[idx_in_buf]);
#endif // defined(MLH_LCD_DYNAMIC_UPDATE)
        }
    }
}

/**
 * @brief Show LCD, in 16 rounds
 * @param done_flag Flag to check if it's done showing
 * @note Can be used if there is no timer interrupt and updating lcd would cause the 7seg blinking
 */
void mlh_show_lcd_16_round(bool *done_flag)
{
    static uint8_t round = 0;
    uint16_t x, x_end, y;
    for (y = 0; y < (LCD_Ymax / 8); ++y) {
        x = round * 8;
        x_end = x + 8;
        for (; x < x_end; ++x) {
#if defined(MLH_LCD_DYNAMIC_UPDATE)
            if (mlh_lcd_buffer[y * LCD_Xmax + x] != _mlh_current_data_in_lcd[y * LCD_Xmax + x]) {
                mlh_lcdSetAddr(y, (LCD_Xmax + 1 - x));
                mlh_lcdWriteData(mlh_lcd_buffer[y * LCD_Xmax + x]);
                _mlh_current_data_in_lcd[y * LCD_Xmax + x] = mlh_lcd_buffer[y * LCD_Xmax + x];
            }
#else
            mlh_lcdSetAddr(y, (LCD_Xmax + 1 - x));
            mlh_lcdWriteData(mlh_lcd_buffer[y * LCD_Xmax + x]);
#endif // defined(MLH_LCD_DYNAMIC_UPDATE)
        }
    }
    round += 1;
    if (round == 16) {
        round = 0;
        *done_flag = true;
    }
}

/**
 * @brief Invert the bits within the region
 * @param x Starting position x, valid value: 0 to 127
 * @param y Starting position y, valid value: 0 to 63
 * @param width_px The width of the region
 * @param height_px The height of the region
 */
void mlh_invert_region_lcd_buf(uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px)
{
    uint16_t i, j;
    // Boundary check
    if (x > (LCD_Xmax - width_px) || y > (LCD_Ymax - height_px)) return;
    // Invert bits in the region
    for (i = x; i < x + width_px; ++i) {
        for (j = y; j < y + height_px; ++j) {
            mlh_lcd_buffer[(j / 8) * LCD_Xmax + i] ^= (0x01 << (j % 8));
        }
    }
}

/*
// Functions for pattern on LCD
// {
*/

/**
 * @brief Draws single pixel to lcd buf
 * @param x The x position, valid value: 0 to 127
 * @param y The y position, valid value: 0 to 63
 * @param color What color this pixel should be, options: FG_COLOR or BG_COLOR
 */
void mlh_draw_pixel_lcd_buf(uint16_t x, uint16_t y, uint16_t color)
{
    if (color == FG_COLOR)
        mlh_lcd_buffer[(y / 8) * LCD_Xmax + x] |= (0x01 << (y % 8));
    else
        mlh_lcd_buffer[(y / 8) * LCD_Xmax + x] &= (~(0x01 << (y % 8)));
}

/**
 * @brief Draws a byte of pixels to lcd buf
 * @param pattern The pattern to draw
 * @param x Starting x position, valid value: 0 to (127-8)
 * @param y Starting y position, valid value: 0 to (63-8)
 * @param color What color the 1s should be, options: FG_COLOR or BG_COLOR
 * // TODO: Maybe add the force_draw parameter
 */
void mlh_draw_byte_lcd_buf(const uint8_t pattern, uint16_t x, uint16_t y, uint16_t color)
{
    uint16_t k;
    // Boundary check
    if (x > (LCD_Xmax) || y > (LCD_Ymax - 8)) return;
    for (k = 0; k < 8; ++k) {
        if (pattern & (0x01 << k)) {
            mlh_draw_pixel_lcd_buf(x, y + k, color);
        }
    }
}

/**
 * @brief Draws the bitmap to lcd buf
 * @param bitmap The bitmap to draw
 * @param x Starting x position, valid value: 0 to (127 - width_px)
 * @param y Starting y position, valid value: 0 to ( 63 - height_px)
 * @param width_px The width of the bitmap
 * @param height_px The height of the bitmap
 * @param color What color the 1s should be, options: FG_COLOR or BG_COLOR
 * @param force_draw Draws the full bitmap to lcd buffer, rather than ignore the 0s
 */
void mlh_draw_bitmap_lcd_buf(const uint8_t *bitmap, uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px, uint16_t color, bool force_draw)
{
    uint16_t i, j;
    uint8_t pattern;
    // Boundary check
    if (x > (LCD_Xmax - width_px) || y > (LCD_Ymax - height_px)) return;
    for (i = 0; i < width_px; i += 1) {
        for (j = 0; j < height_px; j += 1) {
            pattern = bitmap[(j / 8) * width_px + i];
            // Draws 1s
            if (pattern & (0x01 << (j % 8))) {
                // @note This only draws the bits that are 1
                mlh_draw_pixel_lcd_buf(x + i, y + j, color);
            }
            // @note This draws the full bitmap to lcd buffer
            // Draws 0s if force_draw is enabled
            else if (force_draw == true) {
                // if (color == FG_COLOR) {
                //     mlh_draw_pixel_lcd_buf(x + i, y + j, BG_COLOR);
                // } else {
                //     mlh_draw_pixel_lcd_buf(x + i, y + j, FG_COLOR);
                // }
                // Or even simpler
                mlh_draw_pixel_lcd_buf(x + i, y + j, ~(color));
            }
        }
    }
}

/**
 * @brief Draws the scaled bitmap to lcd buf
 * @param bitmap The bitmap to draw
 * @param x Starting x position, valid value: 0 to (127 - width_px)
 * @param y Starting y position, valid value: 0 to ( 63 - height_px)
 * @param width_px The width of the bitmap
 * @param height_px The height of the bitmap
 * @param color What color the 1s should be, options: FG_COLOR or BG_COLOR
 * @param scale The scale ratio to the bitmap
 * @param force_draw Draws the full bitmap to lcd buffer, rather than ignore the 0s
 */
void mlh_draw_bitmap_scaled_lcd_buf(const uint8_t* bitmap, uint16_t x, uint16_t y, uint16_t width_px, uint16_t height_px, uint16_t color, uint8_t scale, bool force_draw)
{
    int16_t i, j;
    if (x > (LCD_Xmax - width_px*scale) || y > (LCD_Ymax - height_px*scale)) return;
    for (i = 0; i < width_px; i += 1) {
        for (j = 0; j < height_px; j += 1) {
            // Draws 1s
            if (bitmap[(j / 8) * width_px + i] & (0x01 << (j % 8))) {
                // @note This only draws the bits that are 1
                mlh_draw_rectangle_lcd_buf(x + i * scale, y + j * scale, x + (i + 1) * (scale)-1, y + (j + 1) * (scale)-1, color, true);
            }
            // Draws 0s if force_draw is enabled
            else if (force_draw == true) {
                // if (color == FG_COLOR) {
                //     mlh_fill_rectangle_lcd_buf(x + i * scale, y + j * scale, x + (i + 1) * (scale)-1, y + (j + 1) * (scale)-1, BG_COLOR);
                // } else {
                //     mlh_fill_rectangle_lcd_buf(x + i * scale, y + j * scale, x + (i + 1) * (scale)-1, y + (j + 1) * (scale)-1, FG_COLOR);
                // }
                // Or even simpler
                mlh_draw_rectangle_lcd_buf(x + i * scale, y + j * scale, x + (i + 1) * (scale)-1, y + (j + 1) * (scale)-1, ~(color), true);
            }
        }
    }
}

/**
 * @brief Draws a line to lcd buf, starting from (x1, y1) to (x2, y2)
 * @param x1 The x1
 * @param y1 The y1
 * @param x2 The x2
 * @param y2 The y2
 * @param color What color the line should be, options: FG_COLOR or BG_COLOR
 */
void mlh_draw_line_lcd_buf(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t color)
{
    int16_t dy = y2 - y1;
    int16_t dx = x2 - x1;
    int16_t stepx, stepy;

    if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
    if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
    dy <<= 1;        // dy is now 2*dy
    dx <<= 1;        // dx is now 2*dx

    mlh_draw_pixel_lcd_buf(x1, y1, color);
    if (dx > dy) {
        int fraction = dy - (dx >> 1); // same as 2*dy - dx
        while (x1 != x2) {
            if (fraction >= 0) {
                y1 += stepy;
                fraction -= dx; // same as fraction -= 2*dx
            }
            x1 += stepx;
            fraction += dy; // same as fraction -= 2*dy
            mlh_draw_pixel_lcd_buf(x1, y1, color);
        }
    }
    else {
        int fraction = dx - (dy >> 1);
        while (y1 != y2) {
            if (fraction >= 0) {
                x1 += stepx;
                fraction -= dy;
            }
            y1 += stepy;
            fraction += dx;
            mlh_draw_pixel_lcd_buf(x1, y1, color);
        }
    }
}

/**
 * @brief Draws a rectangle to lcd buf, starting from (x0, y0) to (x1, y2)
 * @param x0 The x0
 * @param y0 The y0
 * @param x1 The x1
 * @param y1 The y1
 * @param color What color the rectangle should be, options: FG_COLOR or BG_COLOR
 * @param fill To fill the rectangle or not
 */
void mlh_draw_rectangle_lcd_buf(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, bool fill)
{
    int16_t x, y, tmp;
    if (x0 > x1) { tmp = x1; x1 = x0; x0 = tmp; }
    if (y0 > y1) { tmp = y1; y1 = y0; y0 = tmp; }
    if (fill) {
        for (x = x0; x <= x1; x++) {
            for (y = y0; y <= y1; y++) {
                mlh_draw_pixel_lcd_buf(x, y, color);
            }
        }
    } else {
        for (x = x0; x <= x1; x++) mlh_draw_pixel_lcd_buf(x, y0, color);
        for (y = y0; y <= y1; y++) mlh_draw_pixel_lcd_buf(x0, y, color);
        for (x = x0; x <= x1; x++) mlh_draw_pixel_lcd_buf(x, y1, color);
        for (y = y0; y <= y1; y++) mlh_draw_pixel_lcd_buf(x1, y, color);
    }
}

/**
 * @brief Draws a circle to lcd buf, center (xc, yc) and radius r
 * @param xc The center coordinate x
 * @param yc The center coordinate y
 * @param r The radius
 * @param color What color the circle should be, options: FG_COLOR or BG_COLOR
 * @param fill To fill the circle or not
 */
void mlh_draw_circle_lcd_buf(int16_t xc, int16_t yc, int16_t r, uint16_t color, bool fill)
{
    int16_t x = 0;
    int16_t y = r;
    int16_t p = 3 - 2 * r;
    if (!r) return;
    // only formulate 1/8 of circle
    while (y >= x) {
        if (fill) {
            // upper left left to lower left left
            mlh_draw_line_lcd_buf(xc - x, yc - y, xc - x, yc + y, color);
            // upper upper left to lower lower left
            mlh_draw_line_lcd_buf(xc - y, yc - x, xc - y, yc + x, color);
            // upper upper right to lower lower left
            mlh_draw_line_lcd_buf(xc + y, yc - x, xc + y, yc + x, color);
            // upper right right to lower right right
            mlh_draw_line_lcd_buf(xc + x, yc - y, xc + x, yc + y, color);
        } else {
            mlh_draw_pixel_lcd_buf(xc - x, yc - y, color); // upper left left
            mlh_draw_pixel_lcd_buf(xc - y, yc - x, color); // upper upper left
            mlh_draw_pixel_lcd_buf(xc + y, yc - x, color); // upper upper right
            mlh_draw_pixel_lcd_buf(xc + x, yc - y, color); // upper right right
            mlh_draw_pixel_lcd_buf(xc - x, yc + y, color); // lower left left
            mlh_draw_pixel_lcd_buf(xc - y, yc + x, color); // lower lower left
            mlh_draw_pixel_lcd_buf(xc + y, yc + x, color); // lower lower right
            mlh_draw_pixel_lcd_buf(xc + x, yc + y, color); // lower right right
        }
        if (p < 0) p += 4 * (x++) + 6;
        else p += 4 * ((x++) - y--) + 10;
    }
}

/**
 * @brief Draws a triangle to lcd buf, given the three points (x0, y0), (x1, y1) and (x2, y2)
 * @param x0 x0
 * @param y0 y0
 * @param x1 x1
 * @param y1 y1
 * @param x2 x2
 * @param y2 y2
 * @param color What color the triangle should be, options: FG_COLOR or BG_COLOR
 */
void mlh_draw_triangle_lcd_buf(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color)
{
	mlh_draw_line_lcd_buf(x0, y0, x1, y1, color);
	mlh_draw_line_lcd_buf(x1, y1, x2, y2, color);
	mlh_draw_line_lcd_buf(x0, y0, x2, y2, color);
}

// TODO: flood fill algorithm?
// void mlh_fill_triangle_lcd_buf();

/*
// }
// End (Functions for pattern on LCD)
*/

/*
// Functions for text on LCD
// {
*/

/**
 * @brief Print a character in lcd buf
 * @param x The x position, valid value: 0 to 128 - font_size
 * @param y The y position, valid value: 0 to (64 - 8)
 * @param font_size The font size, valid options: 8 or 5
 * @param ascii_code The ascii code for the character
 */
void mlh_print_char_lcd_buf(uint16_t x, uint16_t y, uint8_t font_size, unsigned char ascii_code)
{
    uint16_t i, j, base_idx;
    uint8_t font_bitmap[8];
    // Check font size and boundary
    if (font_size == 8 && (x <= (LCD_Xmax - 8) && y <= (LCD_Ymax - 8))) {
        base_idx = (ascii_code - 0x20) * 16;
        // Upper half and bottom half
        for (j = 0; j < 2; ++j) {
            for (i = 0; i < 8; ++i) {
                font_bitmap[i] = Font8x16[base_idx + j * 8 + i];
            }
            mlh_draw_bitmap_lcd_buf(font_bitmap, x, y + j * 8, 8, 8, FG_COLOR, true);
        }
    } else if (font_size == 5 && (x <= (LCD_Xmax - 5) && y <= (LCD_Ymax - 7))) {
        if (ascii_code < 0x20) ascii_code = 0x20;
        base_idx = (ascii_code - 0x20) * 5;
        for (i = 0; i < 5; ++i) {
            font_bitmap[i] = Font5x7[base_idx + i];
        }
        mlh_draw_bitmap_lcd_buf(font_bitmap, x, y, 5, 8, FG_COLOR, true);
        // TODO: If I don't need the last line of the font (the bottom line),
        // maybe I can just use below
        // Because the start position (x, y) can be specified by caller
        // mlh_draw_bitmap_lcd_buf(font_bitmap, x, y, 5, 7, FG_COLOR, true);
    }
}

/**
 * @brief Print a line in lcd buf
 * @param x The x position, valid value: 0 to 128 - font_size*strlen
 * @param y The y position, valid value: 0 to (64 - 8)
 * @param font_size The font size, valid options: 8 or 5
 * @param format Text format, like printf
 */
void mlh_print_line_lcd_buf(uint16_t x, uint16_t y, uint8_t font_size, const char *format, ...)
{
    // Stolen from https://github.com/poyu39/Nu_LB_NUC140/blob/main/Library/Nu-LB-NUC140/Source/NewLCD.c
    int16_t i;
    char buffer[32];
    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    for (i = 0; i < strlen(buffer); i++)
        mlh_print_char_lcd_buf(x + i * font_size, y, font_size, buffer[i]);
}

/**
 * @brief Print a line on lcd using 8 rounds
 * @param line Line no, valid value: 0 to 3
 * @param text The text to print, strlen(text) <= 16
 * @param done_flag A pointer to the flag that indicates the end of 8 rounds
 * @note Can be used if there is no timer interrupt and the lcd would make the 7seg blinking
 */
void mlh_print_line_8_round_lcd(uint8_t line, char *text, bool *done_flag)
{
    static uint8_t round = 0;
    uint8_t i, iEnd, textLen = strlen(text);
    // Clear 2 characters at a time
    // round = 01234567
    i = round * 2;
    iEnd = i + 2;
    for (; i < iEnd; ++i) {
        if (i >= textLen) {
            mlh_print_char_lcd_buf(i * 8, line * 16, 8, ' ');
        } else {
            mlh_print_char_lcd_buf(i * 8, line * 16, 8, text[i]);
        }
        // mlh_show_char_lcd(i * 8, line * 16);
        // mlh_show_region_lcd(i * 8, line * 16, 8, 16);
        mlh_show_lcd();
    }
    round += 1;
    if (round == 8) {
        round = 0;
        *done_flag = true;
    }
}

/*
// }
// End (Functions for text on LCD)
*/

#endif // MLH_LCD
/***********************/
// End LCD }
/***********************/

/***********************/
// TIMER 3, dedicate for 7seg {
/***********************/
#ifdef MLH_7SEG_INT

uint8_t _seg_index = 0;

/**
 * @brief Timer 3 IRQ handler
 */
void TMR3_IRQHandler(void) {
    TIMER_ClearIntFlag(TIMER3);
    mlh_close_7seg();
    mlh_show_buf_7seg(_seg_index);
    _seg_index = (_seg_index + 1) % 4;
}

/**
 * @brief Initialize timer 3
 * TODO: Maybe change the function name with prefix mlh_
 */
void Init_Timer3(void)
{
    TIMER_Open(TIMER3, TMR3_OPERATING_MODE, TMR3_OPERATING_FREQ);
    TIMER_EnableInt(TIMER3);
    NVIC_EnableIRQ(TMR3_IRQn);
    TIMER_Start(TIMER3);
}

#endif
/***********************/
// End TIMER 3, dedicate for 7seg }
/***********************/

/***********************/
// PWM Buzzer {
/***********************/
#ifdef MLH_PWM_BUZZER

#define mlh_config_pwm_buzzer(freq, duty_cycle) \
    PWM_ConfigOutputChannel(PWM1, PWM_CH0, freq, duty_cycle)

/**
 * @brief Set the pwm buzzer to output sound or not
 * @param enable True to enable the output, false to disable the output
 */
void mlh_set_output_pwm_buzzer(bool enable)
{
    if (enable)
        PWM_EnableOutput(PWM1, PWM_CH_0_MASK);
    else
        PWM_DisableOutput(PWM1, PWM_CH_0_MASK);
}

// TODO:
// mlh_toggle_output_pwm_buzzer(void)

/**
 * @brief Initialize the pwm buzzer
 * @param freq Initial frequency for the pwm buzzer
 * @param duty_cycle Initial duty cycle for the pwm buzzer, valid value 0 to 100
 * @param enable True to enable output, false to disable output
 * @note About duty cycle, because 0 is to let the buzzer buzz, so setting duty_cycle approach 0 makes it louder,
 *       while setting to approach 100 makes it buzz less louder
 */
void mlh_init_pwm_buzzer(uint32_t freq, uint32_t duty_cycle, bool enable)
{
    // Initial config: 50Hz frequency, 5% duty cycle
    PWM_ConfigOutputChannel(PWM1, PWM_CH0, freq, duty_cycle);
    mlh_set_output_pwm_buzzer(enable);
    PWM_Start(PWM1, PWM_CH_0_MASK);
}

#endif // MLH_PWM_BUZZER
/***********************/
// End PWM Buzzer }
/***********************/

/***********************/
// ADC7, VR {
/***********************/
#ifdef MLH_ADC_VR

uint32_t mlh_U32ADC_VALUE = 0, mlh_LAST_U32ADC_VALUE = 0;
volatile bool mlh_adc_converted = false;

/**
 * @brief ADC IRQ handler
 */
void ADC_IRQHandler(void)
{
    uint32_t u32Flag;

    // Get ADC conversion finish interrupt flag
    u32Flag = ADC_GET_INT_FLAG(ADC, ADC_ADF_INT);

    if(u32Flag & ADC_ADF_INT) {
        mlh_LAST_U32ADC_VALUE = mlh_U32ADC_VALUE;
        mlh_U32ADC_VALUE = ADC_GET_CONVERSION_DATA(ADC, 7);
        mlh_adc_converted = true;
    }

    ADC_CLR_INT_FLAG(ADC, u32Flag);
}

/**
 * @brief Initialize the ADC variable resistor
 * @note ADC_OPERATION_MODE needs to set to single mode
 */
void mlh_init_adc_vr(void)
{
    ADC_Open(ADC, ADC_INPUT_MODE, ADC_OPERATION_MODE, ADC_CHANNEL_MASK);
    ADC_POWER_ON(ADC);
    ADC_EnableInt(ADC, ADC_ADF_INT);
    NVIC_EnableIRQ(ADC_IRQn);
    // ADC_START_CONV(ADC);
}

/**
 * @brief Get the value from ADC 7
 * @return The ADC value
 */
uint32_t mlh_get_value_adc(void)
{
    ADC_START_CONV(ADC);
    while (!mlh_adc_converted);
    mlh_adc_converted = false;
    return mlh_U32ADC_VALUE;
}

#endif // MLH_ADC_VR
/***********************/
// End ADC7, VR }
/***********************/

/***********************/
// UART0 {
/***********************/
#ifdef MLH_UART

#define MLH_UART0_BAUD_RATE 115200
#define MLH_UART0_BUF_SIZE 64
#define MLH_UART0_END_OF_MSG '\n' // Single byte end of message flag

char mlh_tx_buf[MLH_UART0_BUF_SIZE];
char mlh_rx_buf[MLH_UART0_BUF_SIZE];
volatile uint32_t mlh_rx_idx = 0;
volatile bool RX_FLAG = false;

/**
 * @brief UART 0/2 IRQ handler
 * @note
 *   Even though the UART_Write writes many bytes at once,
 *   the IRQ handler will enter # bytes times
 */
void UART02_IRQHandler(void)
{
    uint8_t c;
    uint32_t u32IntSts = UART0->ISR;
    // check ISR on & RX is ready
    if (u32IntSts & UART_IS_RX_READY(UART0)) {
        while (!UART_GET_RX_EMPTY(UART0)) { // check RX is not empty
            c = UART_READ(UART0);           // read UART RX data
            if (RX_FLAG) continue;          // Read out the remaining bytes
            if (c == MLH_UART0_END_OF_MSG) {// check line-end
                mlh_rx_buf[mlh_rx_idx] = '\0';
                mlh_rx_idx = 0;
                RX_FLAG = true;
                break;
            } else {
                mlh_rx_buf[mlh_rx_idx] = c;
                mlh_rx_idx++;
            }
        }
    }
}

/**
 * @brief Initialize UART 0
 */
void mlh_init_uart(void)
{
    UART_Open(UART0, MLH_UART0_BAUD_RATE);
    UART_ENABLE_INT(UART0, UART_IER_RDA_IEN_Msk); // enable UART0 interrupt (triggerred by Read-Data-Available)
    NVIC_EnableIRQ(UART02_IRQn);                  // enable Cortex-M0 NVIC interrupt for UART02
}

/**
 * @brief Check if rx is ready
 * @return True if rx is ready, false if not
 * @note After get the rx data, remember to call mlh_clear_rx_flag() to clear the rx ready flag
 */
bool mlh_is_rx_ready(void)
{
    return RX_FLAG;
}

/**
 * @brief Clear the rx ready flag
 * @note Call it after getting the data from rx, to clear the flag to be ready for next receving
 */
void mlh_clear_rx_flag(void)
{
    RX_FLAG = false;
}

/**
 * @brief Send text using uart
 * @param text The text to send
 */
void mlh_write_text_uart(char *text)
{
    UART_Write(UART0, (uint8_t *)text, strlen(text));
}

/**
 * @brief Read text recieved from uart
 * @param dest A place to store the recieved text
 * @note Check rx ready, then call this function
 */
void mlh_read_text_uart(char *dest)
{
    strcpy(dest, mlh_rx_buf);
}

/**
 * @brief Send a number as text through uart
 * @param num The number to send
 */
void mlh_write_int_uart(int32_t num)
{
    sprintf(mlh_tx_buf, "%d", num);
    mlh_write_text_uart(mlh_tx_buf);
}

/**
 * @brief Read a integer number from uart, if the data is send by mlh_write_int_uart function
 * @return The integer read from uart
 */
int32_t mlh_read_int_uart(void)
{
    int32_t num = 0;
    char char_num[MLH_UART0_BUF_SIZE];
    uint8_t i = 0;
    bool is_neg = false;

    // Read the recieved num text
    mlh_read_text_uart(char_num);

    // Process the num text to num
    if (char_num[0] == '-') {
        is_neg = true;
        i = 1;
    }
    for (; i < strlen(char_num); ++i) {
        num = num * 10 + (char_num[i] - '0');
    }
    if (is_neg) num = -num;
    return num;
}

/**
 * @brief Send text through uart, the text is formatted
 * @param format The format, like printf
 */
void mlh_write_format_text_uart(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vsprintf(mlh_tx_buf, format, args);
    va_end(args);
    mlh_write_text_uart(mlh_tx_buf);
}

#endif // MLH_UART
/***********************/
// End UART0 }
/***********************/

// TODO: Add I2C EEPROM

#endif // _MY_LIB_H_

/**
 * Change Log:
 * 2024/12/09 *V0.1.0*
 *   - Release V0.1.0 (Using semver)
 *   - Add function descriptions to the new added functions
 *   - Modify show_7seg_with_pattern, and add a pattern_to_7seg_pattern function
 *   - Fix invert lcd region bug, forget to add x and y to width and height in for loop
 * 2024/12/04
 *   - Fix draw scaled bitmap, change fill rectangle to draw rectangle with filled parameter set to true
 * 2024/12/03
 *   - Add UART functions
 *   - Remove fill rectangle and fill circle, merge their functionality to draw rectangle and draw circle
 * 2024/12/02
 *   - Add draw fill circle function for lcd
 *   - Add adc to sample usage and function prototype
 * 2024/12/01
 *   - Add adc init and IRQ handler
 *   - Update mlh_init_lcd(), to automatically clear lcd when initialization
 * 2024/11/30
 *   - Update mlh_seg_buf[4]'s description
 *   - Add mlh_show_digit_7seg() and mlh_show_buf_7seg()
 *   - Remove mlh_show_7seg()
 *   - Update mlh_print_line_8_round_lcd()
 *   - Remove mlh_show_region_lcd function
 *   - Add example of MCU_init.h
 *   - Add pwm buzzer functions
 * 2024/11/29
 *   - Update keypad ISR to be able to detect key state, i.e. key down, up, pressing, no press
 * Earlier
 *   - led, keypad, 7seg, lcd functions
 */
