#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WRITE_WAV_OUT 1
#if (WRITE_WAV_OUT == 1)
#include "./wave_file.txt"
#endif // (WRITE_WAV_OUT == 1)

// #define SAMPLE_RATE 44100  // Standard sample rate
// #define DURATION 1         // Duration in seconds
// #define FREQUENCY 523      // Frequency of the sine wave (Hz)
// #define AMPLITUDE 32767    // Max amplitude for 16-bit audio
#define SAMPLE_RATE 8192  // Standard sample rate
#define DURATION 0.5         // Duration in seconds
#define FREQUENCY 523      // Frequency of the sine wave (Hz)
#define AMPLITUDE 32767    // Max amplitude for 16-bit audio

// WAV header structure
unsigned char wavHeader[44] = {
    'R', 'I', 'F', 'F', 0, 0, 0, 0, // ChunkID and ChunkSize (to be filled later)
    'W', 'A', 'V', 'E',             // Format
    'f', 'm', 't', ' ', 16, 0, 0, 0, 1, 0, 1, 0, // Subchunk1ID, Subchunk1Size, AudioFormat, NumChannels
    SAMPLE_RATE & 0xFF, (SAMPLE_RATE >> 8) & 0xFF, (SAMPLE_RATE >> 16) & 0xFF, (SAMPLE_RATE >> 24) & 0xFF, // SampleRate
    0, 0, 0, 0,                     // ByteRate (to be filled later)
    2, 0, 16, 0,                    // BlockAlign, BitsPerSample
    'd', 'a', 't', 'a', 0, 0, 0, 0  // Subchunk2ID, Subchunk2Size (to be filled later)
};

#if (WRITE_WAV_OUT == 1)
void write_wave()
{
    const char filename[] = "test.wav";
    FILE *fptr = fopen(filename, "wb");
    if (fptr == NULL) {
        fprintf(stderr, "Can not open file\n");
        exit(1);
    }

    fwrite(wavFile, sizeof(wavFile), 1, fptr);
}
#endif // (WRITE_WAV_OUT == 1)

int main() {
    int numSamples = SAMPLE_RATE * DURATION;
    int dataSize = numSamples * 2; // 2 bytes per sample for 16-bit audio
    int byteRate = SAMPLE_RATE * 2; // SampleRate * BlockAlign

    // Fill in ChunkSize, ByteRate, and Subchunk2Size
    wavHeader[4] = (36 + dataSize) & 0xFF;
    wavHeader[5] = ((36 + dataSize) >> 8) & 0xFF;
    wavHeader[6] = ((36 + dataSize) >> 16) & 0xFF;
    wavHeader[7] = ((36 + dataSize) >> 24) & 0xFF;

    wavHeader[28] = byteRate & 0xFF;
    wavHeader[29] = (byteRate >> 8) & 0xFF;
    wavHeader[30] = (byteRate >> 16) & 0xFF;
    wavHeader[31] = (byteRate >> 24) & 0xFF;

    wavHeader[40] = dataSize & 0xFF;
    wavHeader[41] = (dataSize >> 8) & 0xFF;
    wavHeader[42] = (dataSize >> 16) & 0xFF;
    wavHeader[43] = (dataSize >> 24) & 0xFF;

    // Generate sine wave data
    short waveData[numSamples];
    for (int i = 0; i < numSamples; i++) {
        double t = (double)i / SAMPLE_RATE; // Time in seconds
        waveData[i] = (short)(AMPLITUDE * sin(2 * M_PI * FREQUENCY * t));
    }

    const char filename[] = "wave_file.txt";
    FILE *fptr = fopen(filename, "w");
    if (fptr == NULL) {
        fprintf(stderr, "Can not open file [%s]\n", filename);
        exit(1);
    }

    // Output C array
    // printf("unsigned char wavFile[] = {\n");
    fprintf(fptr, "unsigned char wavFile[] = {\n");
    // Print header
    for (int i = 0; i < 44; i++) {
        // printf("0x%02X, ", wavHeader[i]);
        fprintf(fptr, "0x%02X, ", wavHeader[i]);
        if ((i + 1) % 16 == 0) {
            // printf("\n");
            fprintf(fptr, "\n");
        }
    }
    // Print wave data
    for (int i = 0; i < numSamples; i++) {
        // printf("0x%02X, 0x%02X, ", waveData[i] & 0xFF, (waveData[i] >> 8) & 0xFF);
        fprintf(fptr, "0x%02X, 0x%02X, ", waveData[i] & 0xFF, (waveData[i] >> 8) & 0xFF);
        if ((i + 1) % 8 == 0) {
            // printf("\n");
            fprintf(fptr, "\n");
        }
    }
    // printf("};\n");
    fprintf(fptr, "\n};\n");

#if (WRITE_WAV_OUT == 1)
    write_wave();
#endif // (WRITE_WAV_OUT == 1)

    return 0;
}

const int i8WavHeader[] =
{ 
    0x52,0x49,0x46,0x46, // "RIFF"
    0x00,0x00,0x00,0x00, // File Size  (placeholder)
    0x57,0x41,0x56,0x45, // "WAVE"
    0x66,0x6D,0x74,0x20, // "fmt "
    0x14,0x00,0x00,0x00, // Subchunk1Size (20 bytes, non-PCM)
    0x11,0x00,           // AudioFormat (0x11 = ADPCM)
    0x01,0x00,           // NumChannels (1 = Mono)
    0x40,0x1F,0x00,0x00, // SampleRate (8000 Hz)
    0xDF,0x0F,0x00,0x00, // ByteRate (calculated based on ADPCM)
    0x00,0x01,           // BlockAlign (determined by ADPCM format)
    0x04,0x00,           // BitsPerSample (4 for ADPCM)
    0x02,0x00,           // ExtraParamSize (ADPCM-specific)
    0xF9,0x01,           // Fact chunk data (used for ADPCM)
    0x66,0x61,0x63,0x74, // "fact"
    0x04,0x00,0x00,0x00, // Fact chunk size
    0x00,0x00,0x00,0x00, // Placeholder for number of samples
    0x64,0x61,0x74,0x61, // "data"
    0x00,0x00,0x00,0x00  // Placeholder for Data Size
};
