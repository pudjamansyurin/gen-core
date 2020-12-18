#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_SAMPLE_RATE		48000		
#define BIT_PER_SAMPLE		16
#define AUDIO_FILE_SRC		"./rally-car-idle-loop-05-8k-mono.wav"

typedef struct {
  char ChunkID[4]; /* 0 */
  uint32_t FileSize; /* 4 */
  char FileFormat[4]; /* 8 */
  char SubChunk1ID[4]; /* 12 */
  uint32_t SubChunk1Size; /* 16*/
  uint16_t AudioFormat; /* 20 */
  uint16_t NbrChannels; /* 22 */
  uint32_t SampleRate; /* 24 */
  uint32_t ByteRate; /* 28 */
  uint16_t BlockAlign; /* 32 */
  uint16_t BitPerSample; /* 34 */
  char SubChunk2ID[4]; /* 36 */
  uint32_t SubChunk2Size; /* 40 */
} wave_t;

uint8_t checkAudioFile(wave_t *wave) {
  if (wave->SampleRate > MAX_SAMPLE_RATE) {
    printf("File WAV maksimum sampling rate 48kHz, convert pakai Audacity!!\n");
    return 0;
  } 
  if (wave->BitPerSample != BIT_PER_SAMPLE)  {
    printf("File WAV harus PCM 16bit, convert pakai Audacity!!\n");
    return 0;
  }
  return 1;
}

void writeAudioInformation(FILE *file, wave_t *wave) {
  fprintf(file, "// ChunkID\t\t\t: %.*s\n", sizeof(wave->ChunkID), wave->ChunkID);
  fprintf(file, "// FileSize\t\t\t: %zu (Bytes)\n", wave->FileSize);
  fprintf(file, "// FileFormat\t\t: %.*s\n", sizeof(wave->FileFormat), wave->FileFormat);
  fprintf(file, "// SubChunk1ID\t\t: %.*s\n", sizeof(wave->SubChunk1ID), wave->SubChunk1ID);
  fprintf(file, "// SubChunk1Size\t: %zu (Bytes)\n", wave->SubChunk1Size);
  fprintf(file, "// AudioFormat\t\t: %zu (PCM=1, value other than 1 indicate some compression.)\n", wave->AudioFormat);
  fprintf(file, "// NbrChannels\t\t: %zu (Mono=1, Stereo=2)\n", wave->NbrChannels);
  fprintf(file, "// SampleRate\t\t: %zu (Hz)\n", wave->SampleRate);
  fprintf(file, "// ByteRate\t\t\t: %zu (Bps)\n", wave->ByteRate);
  fprintf(file, "// BlockAlign\t\t: %zu (Bytes per sample)\n", wave->BlockAlign);
  fprintf(file, "// BitPerSample\t\t: %zu (bits)\n", wave->BitPerSample);
  fprintf(file, "// SubChunk2ID\t\t: %.*s\n", sizeof(wave->SubChunk2ID), wave->SubChunk2ID);
  fprintf(file, "// SubChunk2Size\t: %zu (Bytes)\n\n", wave->SubChunk2Size);
}

int main(int argc, char *argv[]) {
  uint8_t col = 1, tmp[2];
  uint8_t progress = 0;
  uint64_t byte, end, start, size;
  FILE *fDst, *fSrc;
  wave_t wave;

  // read wav file
  fSrc = fopen(AUDIO_FILE_SRC, "rb");		// Open the file in binary mode
  fread(&wave, sizeof(wave), 1, fSrc);		// Get wav file header

  // check audio file type
  if(!checkAudioFile(&wave)){
    fclose(fSrc);
    return 0;
  }

  // set start & length of real data
  start = sizeof(wave);
  end = (wave.FileSize + 8);
  size = end - start;
  
  // create dst c file
  fDst = fopen("audio16bit.c", "w");
  
  // print audio information
  writeAudioInformation(fDst, &wave);
  
  // print c code
  fprintf(fDst, "#include <stdint.h>\n\n");
  fprintf(fDst, "const uint32_t SOUND_FREQ = %d;\n", wave.SampleRate);
  fprintf(fDst, "const uint32_t SOUND_SIZE = %d;\n", (wave.NbrChannels == 1 ? size * 2 : size));
  
  fprintf(fDst, "// @formatter:off\n");
  fprintf(fDst, "const uint8_t SOUND_SAMPLE[] = {\n  ");

  // iterate each 2 bytes
  for (byte = start; byte < end; byte += 2) {
    fseek(fSrc, byte, SEEK_SET);          		// Move cursor
    fread(tmp, sizeof(uint8_t), 2, fSrc);		// Get data

    fprintf(fDst, "0x%02X, ", tmp[1]);
    fprintf(fDst, "0x%02X, ", tmp[0]);
    if (wave.NbrChannels == 1) {			// Duplicate if channel is mono
      fprintf(fDst, "0x%02X, ", tmp[1]);
      fprintf(fDst, "0x%02X, ", tmp[0]);
    }
    
    // move to the new line
    if (col == wave.NbrChannels) {
      fprintf(fDst, "\n");
      if (byte != (end - 1)) {
        fprintf(fDst, "  ");
      }
      col = 0;
    }
    col++;

    // show progress
    if (progress != ((byte - start) * 100 / end)) {
      progress = (byte - start) * 100 / end;
      printf("Progress : %d %%\n", progress);
    }
  }
  fprintf(fDst, "\n};\n");
  fprintf(fDst, "// @formatter:on\n");

  // Close files
  fclose(fDst);											
  fclose(fSrc); 											

  return 0;
}
