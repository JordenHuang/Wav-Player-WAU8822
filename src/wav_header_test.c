#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "wave_sample.h"
#include "wav_lib.h"


int main(void)
{
    wav_header_t wav_header;
    unsigned char *ptr;
    uint32_t offset;

    // const char filename[80] = "../audio_sample/ImperialMarch60.wav";
    // const char filename[80] = "../audio_sample/gettysburg10.wav";
    const char filename[80] = "../audio_sample/test.wav";

    FILE *fptr = fopen(filename, "rb");
    if (fptr == NULL) {
        fprintf(stderr, "ERROR during opening file\n");
        exit(1);
    }

    // parse_wav(wav_sample, sizeof(wav_sample), &wav_header, (void*)(&ptr));
    printf("Status of parse_wav: %s\n\n", parse_wav(fptr, &wav_header, &offset) == 0 ? "Success" : "Fail");

    printf("format_chunk_id: %x\n", wav_header.format_chunk_id);
    printf("format_chunk_size: %d\n", wav_header.format_chunk_size);

    printf("audio_format: %d\n", wav_header.audio_format);
    printf("num_of_channels: %d\n", wav_header.num_of_channels);
    printf("sample_rate: %d\n", wav_header.sample_rate);
    printf("byte_per_sec: %d\n", wav_header.byte_per_sec);
    printf("block_align: %d\n", wav_header.block_align);
    printf("bits_per_sample: %d\n", wav_header.bits_per_sample);

    printf("Data Chunk Found: Size %u bytes\n", wav_header.data_chunk_size);

    return 0;
}