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
 
         // 1. 写RIFF头
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
 
         // 2. 写FMT块
         strcpy(tag, "fmt ");
         memcpy(chunk.chChunkID, tag, 4);
         chunk.nChunkSize = sizeof(WAVEFORMATX);
         fwrite(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
         memset(&wfx, 0, sizeof(WAVEFORMATX));
         wfx.nFormatTag = 1;
         wfx.nChannels = 1; // 单声道
         wfx.nSamplesPerSec = 8000; // 8khz
         wfx.nAvgBytesPerSec = 16000;
         wfx.nBlockAlign = 2;
         wfx.nBitsPerSample = 16; // 16位
         fwrite(&wfx, 1, sizeof(WAVEFORMATX), fpwave);
 
         // 3. 写data块头
         strcpy(tag, "data");
         memcpy(chunk.chChunkID, tag, 4);
         chunk.nChunkSize = nFrame*160*sizeof(short);
         fwrite(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
}
 
const int roundi(const double x)
{
         return((int)(x+0.5));
} 
 
// 根据帧头计算当前帧大小
int caclAMRFrameSize(unsigned char frameHeader)
{
		 int amrEncodeMode[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200}; // amr 编码方式
         int mode;
         int temp1 = 0;
         int temp2 = 0;
         int frameSize;
 
         temp1 = frameHeader;
 
         // 编码方式编号 = 帧头的3-6位
         temp1 &= 0x78; // 0111-1000
         temp1 >>= 3;
 
         mode = amrEncodeMode[temp1];
 
         // 计算amr音频数据帧大小
         // 原理: amr 一帧对应20ms，那么一秒有50帧的音频数据
         temp2 = roundi((double)(((double)mode / (double)AMR_FRAME_COUNT_PER_SECOND) / (double)8));
 
         frameSize = roundi((double)temp2 + 0.5);
         return frameSize;
}
 
// 读第一个帧 - (参考帧)
// 返回值: 0-出错; 1-正确
int ReadAMRFrameFirst(FILE* fpamr, unsigned char frameBuffer[], int* stdFrameSize, unsigned char* stdFrameHeader)
{
         memset(frameBuffer, 0, sizeof(frameBuffer));
 
         // 先读帧头
         fread(stdFrameHeader, 1, sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         // 根据帧头计算帧大小
         *stdFrameSize = caclAMRFrameSize(*stdFrameHeader);
 
         // 读首帧
         frameBuffer[0] = *stdFrameHeader;
         fread(&(frameBuffer[1]), 1, (*stdFrameSize-1)*sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         return 1;
}
 
// 返回值: 0-出错; 1-正确
int ReadAMRFrame(FILE* fpamr, unsigned char frameBuffer[], int stdFrameSize, unsigned char stdFrameHeader)
{
         int bytes = 0;
         unsigned char frameHeader; // 帧头
 
         memset(frameBuffer, 0, sizeof(frameBuffer));
 
         // 读帧头
         // 如果是坏帧(不是标准帧头)，则继续读下一个字节，直到读到标准帧头
         while(1)
         {
                   bytes = fread(&frameHeader, 1, sizeof(unsigned char), fpamr);
                   if (feof(fpamr)) return 0;
                   if (frameHeader == stdFrameHeader) break;
         }
 
         // 读该帧的语音数据(帧头已经读过)
         frameBuffer[0] = frameHeader;
         bytes = fread(&(frameBuffer[1]), 1, (stdFrameSize-1)*sizeof(unsigned char), fpamr);
         if (feof(fpamr)) return 0;
 
         return 1;
}
 
// 将AMR文件解码成WAVE文件
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
 
         // 检查amr文件头
         fread(magic, sizeof(char), strlen(AMR_MAGIC_NUMBER), fpamr);
         if (strncmp(magic, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER)))
         {
                   fclose(fpamr);
                   return 0;
         }
 
         // 创建并初始化WAVE文件
         fpwave = fopen(pchWAVEFilename, "wb");
         WriteWAVEFileHeader(fpwave, nFrameCount);
 
         /* init decoder */
         destate = (int*)Decoder_Interface_init();
 
         // 读第一帧 - 作为参考帧
         memset(amrFrame, 0, sizeof(amrFrame));
         memset(pcmFrame, 0, sizeof(pcmFrame));
         ReadAMRFrameFirst(fpamr, amrFrame, &stdFrameSize, &stdFrameHeader);
 
         // 解码一个AMR音频帧成PCM数据
         Decoder_Interface_Decode(destate, amrFrame, pcmFrame, 0);
         nFrameCount++;
         fwrite(pcmFrame, sizeof(short), PCM_FRAME_SIZE, fpwave);
 
         // 逐帧解码AMR并写到WAVE文件里
         while(1)
         {
                   memset(amrFrame, 0, sizeof(amrFrame));
                   memset(pcmFrame, 0, sizeof(pcmFrame));
                   if (!ReadAMRFrame(fpamr, amrFrame, stdFrameSize, stdFrameHeader)) break;
 
                   // 解码一个AMR音频帧成PCM数据 (8k-16b-单声道)
                   Decoder_Interface_Decode(destate, amrFrame, pcmFrame, 0);
                   nFrameCount++;
                   fwrite(pcmFrame, sizeof(short), PCM_FRAME_SIZE, fpwave);
         }
 
         Decoder_Interface_exit(destate);
 
         fclose(fpwave);
 
         // 重写WAVE文件头
         fpwave = fopen(pchWAVEFilename, "r+");
         WriteWAVEFileHeader(fpwave, nFrameCount);
         fclose(fpwave);
 
         return nFrameCount;
}