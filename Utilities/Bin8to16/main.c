#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

typedef struct
{
  char   	 ChunkID[4];       	/* 0 */ 
  uint32_t   FileSize;      	/* 4 */
  char		 FileFormat[4];    	/* 8 */
  char		 SubChunk1ID[4];   	/* 12 */
  uint32_t   SubChunk1Size; 	/* 16*/  
  uint16_t   AudioFormat;   	/* 20 */ 
  uint16_t   NbrChannels;   	/* 22 */   
  uint32_t   SampleRate;    	/* 24 */
  
  uint32_t   ByteRate;      	/* 28 */
  uint16_t   BlockAlign;    	/* 32 */  
  uint16_t   BitPerSample;  	/* 34 */  
  char		 SubChunk2ID[4];   	/* 36 */   
  uint32_t   SubChunk2Size; 	/* 40 */    

} WAVE_FormatTypeDef;

int main(int argc, char *argv[]) {
	uint8_t col=1, tmpData[2]; 
	uint16_t data;
	uint64_t i, end, start, size;
	FILE *fDst, *fSrc;
	WAVE_FormatTypeDef waveformat;
	
	// read wav file
	fSrc = fopen("creepy-lullaby-a-8k-stereo.wav", "rb");			// Open the file in binary mode
	fread(&waveformat, sizeof(waveformat), 1, fSrc);				// Get wav file header
	
	// check if it is not mono & not PCM 16 bit
	if(waveformat.BitPerSample != 16 || waveformat.SampleRate > 48000){
		if(waveformat.SampleRate > 48000){
			printf("File WAV maksimum sampling rate 48kHz, convert pakai Audacity!!\n");
		} else {
			printf("File WAV harus PCM 16bit, convert pakai Audacity!!\n");
		}
		fclose(fSrc); 
		return 0;
	} 
	
	// set start & length of real data
	start  = sizeof(waveformat);
	end    = (waveformat.FileSize+8);
	size   = end - start;
	// create dst c file					
	fDst = fopen("audio16bit.c", "w");
	// print information
	fprintf(fDst, "// ChunkID\t\t\t: %.*s\n", sizeof(waveformat.ChunkID), waveformat.ChunkID);
	fprintf(fDst, "// FileSize\t\t\t: %zu (Bytes)\n", waveformat.FileSize);
	fprintf(fDst, "// FileFormat\t\t: %.*s\n", sizeof(waveformat.FileFormat), waveformat.FileFormat);
	fprintf(fDst, "// SubChunk1ID\t\t: %.*s\n", sizeof(waveformat.SubChunk1ID), waveformat.SubChunk1ID);
	fprintf(fDst, "// SubChunk1Size\t: %zu (Bytes)\n", waveformat.SubChunk1Size);
	fprintf(fDst, "// AudioFormat\t\t: %zu (PCM=1, value other than 1 indicate some compression.)\n", waveformat.AudioFormat);
	fprintf(fDst, "// NbrChannels\t\t: %zu (Mono=1, Stereo=2)\n", waveformat.NbrChannels);
	fprintf(fDst, "// SampleRate\t\t: %zu (Hz)\n", waveformat.SampleRate);
	fprintf(fDst, "// ByteRate\t\t\t: %zu (Bps)\n", waveformat.ByteRate);
	fprintf(fDst, "// BlockAlign\t\t: %zu (Bytes per sample)\n", waveformat.BlockAlign);
	fprintf(fDst, "// BitPerSample\t\t: %zu (bits)\n", waveformat.BitPerSample);
	fprintf(fDst, "// SubChunk2ID\t\t: %.*s\n", sizeof(waveformat.SubChunk2ID), waveformat.SubChunk2ID);
	fprintf(fDst, "// SubChunk2Size\t: %zu (Bytes)\n\n", waveformat.SubChunk2Size);
	
	// print c data
	fprintf(fDst, "#include <stdint.h>\n\n");
	fprintf(fDst, "const uint32_t AUDIO_SAMPLE_FREQ = %d;\n", waveformat.SampleRate);
	fprintf(fDst, "const uint32_t AUDIO_SAMPLE_SIZE = %d;\n", (waveformat.NbrChannels == 1 ? size*2 : size));
	fprintf(fDst, "// @formatter:off\n");
	fprintf(fDst, "const uint8_t AUDIO_SAMPLE[] = {\n  ");
	// iterate each 2 bytes
	for(i=start; i<end; i+= 2){
		fseek(fSrc, i, SEEK_SET);          					// Move cursor
		fread(tmpData, sizeof(uint8_t), 2, fSrc);			// Get data
		// combine 2 bytes (16bit)
//		data = tmpData[1] << 8 | tmpData[1];
//		fprintf(fDst, "0x%04X, ", data);
		
		fprintf(fDst, "0x%02X, ", tmpData[1]);
		fprintf(fDst, "0x%02X, ", tmpData[0]);
		if(waveformat.NbrChannels == 1){
			fprintf(fDst, "0x%02X, ", tmpData[1]);
			fprintf(fDst, "0x%02X, ", tmpData[0]);
		}
		// move to the new line
		if(col == waveformat.NbrChannels){	
			fprintf(fDst, "\n");
			if(i != (end - 1)){
				fprintf(fDst, "  ");
			}
			col=0;
		}
		col++;
		
		// show progress
		printf("Progress : %d %%\n", (i-start)*100/end);
	}
	fprintf(fDst, "\n};\n");
	fprintf(fDst, "// @formatter:on\n");
	
    fclose(fDst);											// Close dst file
	fclose(fSrc); 											// Close src file
	
	return 0;
}
