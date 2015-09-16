#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "amrFileCodec.h"
#include "codec/sp_dec.h"
 
void WriteWAVEFileHeader(FILE* fpwave, int nFrame)
{
		 RIFFHEADER riff;
		 XCHUNKHEADER chunk;
		 WAVEFORMATX wfx;
         char tag[10] = "";
 
         // 1. дRIFFͷ
         strcpy(tag, "RIFF");
         memcpy(riff.chRiffID, tag, 4);
         riff.nRiffSize = 4                                     // WAVE
                   + sizeof(XCHUNKHEADER)               // fmt 
                   + sizeof(WAVEFORMATX)           // WAVEFORMATX
                   + sizeof(XCHUNKHEADER)               // DATA
                   + nFrame*160*sizeof(short);    // 
         strcpy(tag, "WAVE");
         memcpy(riff.chRiffFormat, tag, 4);
         fwrite(&riff, 1, sizeof(RIFFHEADER), fpwave);
 
         // 2. дFMT��
         strcpy(tag, "fmt ");
         memcpy(chunk.chChunkID, tag, 4);
         chunk.nChunkSize = sizeof(WAVEFORMATX);
         fwrite(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
         memset(&wfx, 0, sizeof(WAVEFORMATX));
         wfx.nFormatTag = 1;
         wfx.nChannels = 1; // ������
         wfx.nSamplesPerSec = 8000; // 8khz
         wfx.nAvgBytesPerSec = 16000;
         wfx.nBlockAlign = 2;
         wfx.nBitsPerSample = 16; // 16λ
         fwrite(&wfx, 1, sizeof(WAVEFORMATX), fpwave);
 
         // 3. дdata��ͷ
         strcpy(tag, "data");
         memcpy(chunk.chChunkID, tag, 4);
         chunk.nChunkSize = nFrame*160*sizeof(short);
         fwrite(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
}
 
const int roundi(const double x)
{
         return((int)(x+0.5));
} 
 
// ����֡ͷ���㵱ǰ֡��С
int caclAMRFrameSize(unsigned char frameHeader)
{
		 int amrEncodeMode[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200}; // amr ���뷽ʽ
         int mode;
         int temp1 = 0;
         int temp2 = 0;
         int frameSize;
 
         temp1 = frameHeader;
 
         // ���뷽ʽ��� = ֡ͷ��3-6λ
         temp1 &= 0x78; // 0111-1000
         temp1 >>= 3;
 
         mode = amrEncodeMode[temp1];
 
         // ����amr��Ƶ����֡��С
         // ԭ��: amr һ֡��Ӧ20ms����ôһ����50֡����Ƶ����
         temp2 = roundi((double)(((double)mode / (double)AMR_FRAME_COUNT_PER_SECOND) / (double)8));
 
         frameSize = roundi((double)temp2 + 0.5);
         return frameSize;
}
 
// ����һ��֡ - (�ο�֡)
// ����ֵ: 0-����; 1-��ȷ
int ReadAMRFrameFirst(FILE* fpamr, unsigned char frameBuffer[], int* stdFrameSize, unsigned char* stdFrameHeader)
{
         memset(frameBuffer, 0, sizeof(frameBuffer));
 
         // �ȶ�֡ͷ
         fread(stdFrameHeader, 1, sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         // ����֡ͷ����֡��С
         *stdFrameSize = caclAMRFrameSize(*stdFrameHeader);
 
         // ����֡
         frameBuffer[0] = *stdFrameHeader;
         fread(&(frameBuffer[1]), 1, (*stdFrameSize-1)*sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         return 1;
}
 
// ����ֵ: 0-����; 1-��ȷ
int ReadAMRFrame(FILE* fpamr, unsigned char frameBuffer[], int stdFrameSize, unsigned char stdFrameHeader)
{
         int bytes = 0;
         unsigned char frameHeader; // ֡ͷ
 
         memset(frameBuffer, 0, sizeof(frameBuffer));
 
         // ��֡ͷ
         // ����ǻ�֡(���Ǳ�׼֡ͷ)�����������һ���ֽڣ�ֱ��������׼֡ͷ
         while(1)
         {
                   bytes = fread(&frameHeader, 1, sizeof(unsigned char), fpamr);
                   if (feof(fpamr)) return 0;
                   if (frameHeader == stdFrameHeader) break;
         }
 
         // ����֡����������(֡ͷ�Ѿ�����)
         frameBuffer[0] = frameHeader;
         bytes = fread(&(frameBuffer[1]), 1, (stdFrameSize-1)*sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         return 1;
}
 
// ��AMR�ļ������WAVE�ļ�
int DecodeAMRFileToWAVEFile(const char* pchAMRFileName, const char* pchWAVEFilename)
{
         FILE* fpamr = NULL;
         FILE* fpwave = NULL;
         char magic[8];
         int * destate;
         int nFrameCount = 0;
         int stdFrameSize;
         unsigned char stdFrameHeader;
 
         unsigned char amrFrame[MAX_AMR_FRAME_SIZE];
         short pcmFrame[PCM_FRAME_SIZE];
 
         fpamr = fopen(pchAMRFileName, "rb");
         if ( fpamr==NULL ) return 0;
 
         // ���amr�ļ�ͷ
         fread(magic, sizeof(char), strlen(AMR_MAGIC_NUMBER), fpamr);
         if (strncmp(magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER)))
         {
                   fclose(fpamr);
                   return 0;
         }
 
         // ��������ʼ��WAVE�ļ�
         fpwave = fopen(pchWAVEFilename, "wb");
         WriteWAVEFileHeader(fpwave, nFrameCount);
 
         /* init decoder */
         destate = (int*)Decoder_Interface_init();
 
         // ����һ֡ - ��Ϊ�ο�֡
         memset(amrFrame, 0, sizeof(amrFrame));
         memset(pcmFrame, 0, sizeof(pcmFrame));
         ReadAMRFrameFirst(fpamr, amrFrame, &stdFrameSize, &stdFrameHeader);
 
         // ����һ��AMR��Ƶ֡��PCM����
         Decoder_Interface_Decode(destate, amrFrame, pcmFrame, 0);
         nFrameCount++;
         fwrite(pcmFrame, sizeof(short), PCM_FRAME_SIZE, fpwave);
 
         // ��֡����AMR��д��WAVE�ļ���
         while(1)
         {
                   memset(amrFrame, 0, sizeof(amrFrame));
                   memset(pcmFrame, 0, sizeof(pcmFrame));
                   if (!ReadAMRFrame(fpamr, amrFrame, stdFrameSize, stdFrameHeader)) break;
 
                   // ����һ��AMR��Ƶ֡��PCM���� (8k-16b-������)
                   Decoder_Interface_Decode(destate, amrFrame, pcmFrame, 0);
                   nFrameCount++;
                   fwrite(pcmFrame, sizeof(short), PCM_FRAME_SIZE, fpwave);
         }
 
         Decoder_Interface_exit(destate);
 
         fclose(fpwave);
 
         // ��дWAVE�ļ�ͷ
         fpwave = fopen(pchWAVEFilename, "r+");
         WriteWAVEFileHeader(fpwave, nFrameCount);
         fclose(fpwave);
 
         return nFrameCount;
}