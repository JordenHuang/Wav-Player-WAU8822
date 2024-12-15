#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "ff15a/source/ffconf.h"
#include "ff15a/source/diskio.h"
#include "ff15a/source/ff.h"

FIL wav_file_object;
FRESULT res;
const char wav_file_path[] = "\\Do_5sec.wav";

