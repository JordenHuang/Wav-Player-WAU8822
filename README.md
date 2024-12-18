# Wav-Player-WAU8822

A wav player using the WAU8822 chip on Nuvoton Nu-LB-NUC140 learning board.

## Modes

- [x] audio player
- [ ] audio recorder (WIP)
- [ ] audio playback (WIP)

## Requirements

- Arm Keil MDK (Keil μVision)
- Nu-Link Keil Driver V3
- SD card, in FAT32, with some wav audio file in it (file name less than or equal to 8 characters)
- A earphone with 3.5 mm aux connection, connected to Line-out (HP out, J2)

## Dependencies

- [FatFs by ChaN, R0.13b](http://elm-chan.org/fsw/ff/archives.html)
- [SD card driver by Nuvoton Technology Corp](https://www.nuvoton.com/products/microcontrollers/arm-cortex-m0-mcus/nuc140-240-connectivity-series/?group=Software&tab=2)

## Usage

After cloning the project:

1. Change the `wav_file_path` array in main.c, to match the file names in the SD card
2. Insert the SD card and earphone to Nu-LB-NUC140 learning board
3. Compile and download to the board
4. Use
    - key 2 (or 1) to go Up
    - key 8 (or 7) to go Down
    - key 5 to select

## Note

Due to some skill issue, I can't setup WAU8822 properly (or maybe some other problems), so the sound output from it sounds very weird.

To protect your ears, do not wear the earphone too tight.

See [journey](./JOURNEY.md) if you are interested about my reserch journey

## References

- [WAU8822](https://github.com/OpenNuvoton/NUC472_442BSP/blob/master/SampleCode/StdDriver/I2S_MP3PLAYER/nau8822.c)

- [NAU8822 datasheet (I didn't find WAU8822's datasheet)](https://www.nuvoton.com/resource-files/NAU8822DataSheetRev3.3.pdf)

- [I2S](https://doc.embedfire.com/mcu/stm32/f407batianhu/std/zh/latest/book/I2S.html)

- [FatFS explaination](https://doc.embedfire.com/mcu/stm32/f407batianhu/std/zh/latest/book/FLASH_FatFs.html)

- [FatFs by ChaN](http://elm-chan.org/fsw/ff/00index_e.html)

- [sdcard driver code sample from nuvoton](https://www.nuvoton.com/products/microcontrollers/arm-cortex-m0-mcus/nuc140-240-connectivity-series/?group=Software&tab=2)
