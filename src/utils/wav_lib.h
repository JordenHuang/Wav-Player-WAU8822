/**
 * @brief Functions for parsing the wav file header
 * @author Jorden Huang
 */

#ifndef _WAV_LIB_
#define _WAV_LIB_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


typedef struct wav_header_t {
    // Master RIFF chunk
    uint32_t riff;
    uint32_t file_size;
    uint32_t format;
    // Chunk describing the data format
    uint32_t format_chunk_id;
    uint32_t format_chunk_size;
    uint16_t audio_format; // (1: PCM integer, 3: IEEE 754 float)
    uint16_t num_of_channels;
    uint32_t sample_rate;
    uint32_t byte_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;
    // Chunk containing the sampled data
    uint32_t data_chunk_id;
    uint32_t data_chunk_size;
} wav_header_t;


uint8_t parse_wav_arr(const unsigned char* wav_data, wav_header_t *wav_header, const void **audio_data_ptr);
uint8_t parse_wav(FILE *wav_fptr, wav_header_t *wav_header, uint32_t *offset);


uint8_t parse_wav_arr(const unsigned char* wav_data, wav_header_t *wav_header, const void **audio_data_ptr)
{
    const unsigned char *fmt_ptr;
    const unsigned char* data_ptr;

    if (memcmp(wav_data, "RIFF", 4) != 0 || memcmp(wav_data + 8, "WAVE", 4) != 0) {
        printf("Invalid WAV file\n");
        return 1;
    }

    // printf("Valid WAV file detected\n");

    // Fill in master RIFF chunk data
    memcpy(&wav_header->riff,      wav_data,     4);
    memcpy(&wav_header->file_size, wav_data + 4, 4);
    memcpy(&wav_header->format,    wav_data + 8, 4);

    // Read fmt chunk (data format chunk)
    fmt_ptr = wav_data + 12;
    if (memcmp(fmt_ptr, "fmt ", 4) != 0) {
        return 2;
    }
    // Fill in data format chunk
    memcpy(&wav_header->format_chunk_id,   fmt_ptr,      4);
    memcpy(&wav_header->format_chunk_size, fmt_ptr + 4,  4);
    memcpy(&wav_header->audio_format,      fmt_ptr + 8,  2);
    memcpy(&wav_header->num_of_channels,   fmt_ptr + 10, 2);
    memcpy(&wav_header->sample_rate,       fmt_ptr + 12, 4);
    memcpy(&wav_header->byte_per_sec,      fmt_ptr + 16, 4);
    memcpy(&wav_header->block_align,       fmt_ptr + 20, 2);
    memcpy(&wav_header->bits_per_sample,   fmt_ptr + 22, 2);

    // Read sample data chunk
    data_ptr = fmt_ptr + 24;
    if (memcmp(data_ptr, "data", 4) != 0) {
        return 3;
    }
    // Fill in sample data chunk
    memcpy(&wav_header->data_chunk_id,   data_ptr,     4);
    memcpy(&wav_header->data_chunk_size, data_ptr + 4, 4);

    *audio_data_ptr = data_ptr + 8;

    return 0;
}

uint8_t parse_wav(FILE *wav_fptr, wav_header_t *wav_header, uint32_t *offset)
{
    // unsigned char buf[sizeof(wav_header_t)];
    wav_header_t buf;
    fread(&buf, sizeof(buf), 1, wav_fptr);

    if (memcmp(&buf.riff, "RIFF", 4) != 0) {
        printf("Invalid WAV file\n");
        return 1;
    }
    if (memcmp(&buf.format, "WAVE", 4) != 0) {
        printf("Invalid WAV file\n");
        return 1;
    }

    printf("Valid WAV file detected\n");

    // Fill in master RIFF chunk data
    // memcpy(&wav_header->riff,      buf.riff,     4);
    // memcpy(&wav_header->file_size, buf.file_size, 4);
    // memcpy(&wav_header->format,    buf.format, 4);
    wav_header->riff      = buf.riff;
    wav_header->file_size = buf.file_size;
    wav_header->format    = buf.format;

    // Read fmt chunk (data format chunk)
    if (memcmp(&buf.format_chunk_id, "fmt ", 4) != 0) {
        return 2;
    }
    // Fill in data format chunk
    // memcpy(&wav_header->format_chunk_id,   buf.format_chunk_id,   4);
    // memcpy(&wav_header->format_chunk_size, buf.format_chunk_size, 4);
    // memcpy(&wav_header->audio_format,      buf.audio_format,      2);
    // memcpy(&wav_header->num_of_channels,   buf.num_of_channels,   2);
    // memcpy(&wav_header->sample_rate,       buf.sample_rate,       4);
    // memcpy(&wav_header->byte_per_sec,      buf.byte_per_sec,      4);
    // memcpy(&wav_header->block_align,       buf.block_align,       2);
    // memcpy(&wav_header->bits_per_sample,   buf.bits_per_sample,   2);
    wav_header->format_chunk_id   = buf.format_chunk_id;
    wav_header->format_chunk_size = buf.format_chunk_size;
    wav_header->audio_format      = buf.audio_format;
    wav_header->num_of_channels   = buf.num_of_channels;
    wav_header->sample_rate       = buf.sample_rate;
    wav_header->byte_per_sec      = buf.byte_per_sec;
    wav_header->block_align       = buf.block_align;
    wav_header->bits_per_sample   = buf.bits_per_sample;

    // Read sample data chunk
    if (memcmp(&buf.data_chunk_id, "data", 4) != 0) {
        return 3;
    }
    // Fill in sample data chunk
    // memcpy(&wav_header->data_chunk_id,   buf.data_chunk_id,   4);
    // memcpy(&wav_header->data_chunk_size, buf.data_chunk_size, 4);
    wav_header->data_chunk_id   = buf.data_chunk_id;
    wav_header->data_chunk_size = buf.data_chunk_size;

    *offset = sizeof(wav_header_t) + 4;

    return 0;
}


#endif // _WAV_LIB_
