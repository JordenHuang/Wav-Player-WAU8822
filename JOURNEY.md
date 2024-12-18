# Journey

It took me a week to finally heard sound output from the board.

It took me a lot of time to find out what config is missing, so that the I2S is not sending data, or not even power up.

In [SYS_Init.c](./Library/Nu-LB-NUC140/Source/SYS_init.c), some settings about I2S is not provided.

And I found what needs to be config in [NUC100Series.h](./Library/Device/Nuvoton/NUC100Series/Include/NUC100Series.h). Like in the line 7524, the function setting about PA15, `PA15_I2SMCLK` and `GPA_MFP` needs to be set to select I2S_MCLK function, but `PA15_I2SMCLK` is not provided originally, so it needs to be added.

The SD card driver provided in the library is using the older version of I2C functions, which I can't use it directly, so I found one on Nuvoton's website, and made some modify to fit the pin setting of our board.

Also, the image about WAU8822 in Nu-LB-NUC140 User Manual on page 22 is wrong. Line-in should be the middle one, Line-out should be the third one

<br>

Special thanks to:
- Professor Wang, for let me take the learning board home
- TA, [poyu39](https://github.com/poyu39), for sharing some experiences
- AI tool, for helping me address the possible points to the problems, and provide the direction to solve the issues
- The sample codes on Github, for giving me an example of WAU8822 setting function and I2S IRQ handler

<br>

Next step:
- Make the sound output correct, I think it's caused by the WAU8822 setting
- Implement the audio playback utility, plays the sound directly to the Line-out that captured from the Line-in
- Implement the audio recorder, records the sound to a wav file

Written by, Jorden Huang
