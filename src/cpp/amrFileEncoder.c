#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "amrFileCodec.h"
#include "codec/sp_enc.h"
 
// 从WAVE文件中跳过WAVE文件头，直接到PCM音频数据
void SkipToPCMAudioData(FILE* fpwave)
{
         RIFFHEADER riff;
         FMTBLOCK fmt;
         XCHUNKHEADER chunk;
         WAVEFORMATX wfx;
         int bDataBlock = 0;
 
         // 1. 读RIFF头
         fread(&riff, 1, sizeof(RIFFHEADER), fpwave);
 
         // 2. 读FMT块 - 如果 fmt.nFmtSize>16 说明需要还有一个附属大小没有读
         fread(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
         if ( chunk.nChunkSize>16 )
         {
                   fread(&wfx, 1, sizeof(WAVEFORMATX), fpwave);
         }
         else
         {
                   memcpy(fmt.chFmtID, chunk.chChunkID, 4);
                   fmt.nFmtSize = chunk.nChunkSize;
                   fread(&fmt.wf, 1, sizeof(WAVEFORMAT), fpwave);
         }
 
         // 3.转到data块 - 有些还有fact块等。
         while(!bDataBlock)
         {
                   fread(&chunk, 1, sizeof(XCHUNKHEADER), fpwave);
                   if ( !memcmp(chunk.chChunkID, "data", 4) )
                   {
                            bDataBlock = 1;
                            break;
                   }
                   // 因为这个不是data块,就跳过块数据
                   fseek(fpwave, chunk.nChunkSize, SEEK_CUR);
         }
}
 
// 从WAVE文件读一个完整的PCM音频帧
// 返回值: 0-错误 >0: 完整帧大小
int ReadPCMFrame(short speech[], FILE* fpwave, int nChannels, int nBitsPerSample)
{
         int nRead = 0;
         int x = 0, y=0;
         unsigned short ush1=0, ush2=0, ush=0;
 
         // 原始PCM音频帧数据
         unsigned char pcmFrame_8b1[PCM_FRAME_SIZE];
         unsigned char pcmFrame_8b2[PCM_FRAME_SIZE<<1];
         unsigned short pcmFrame_16b1[PCM_FRAME_SIZE];
         unsigned short pcmFrame_16b2[PCM_FRAME_SIZE<<1];
 
         if (nBitsPerSample==8 && nChannels==1)
         {
                   nRead = fread(pcmFrame_8b1, (nBitsPerSample/8), PCM_FRAME_SIZE*nChannels, fpwave);
                   for(x=0; x<PCM_FRAME_SIZE; x++)
                   {
                            speech[x] =(short)((short)pcmFrame_8b1[x] << 7);
                   }
         }
         else
         if (nBitsPerSample==8 && nChannels==2)
         {
                   nRead = fread(pcmFrame_8b2, (nBitsPerSample/8), PCM_FRAME_SIZE*nChannels, fpwave);
                   for( x=0, y=0; y<PCM_FRAME_SIZE; y++,x+=2 )
                   {
                            // 1 - 取两个声道之左声道
                            speech[y] =(short)((short)pcmFrame_8b2[x+0] << 7);
                            // 2 - 取两个声道之右声道
                            //speech[y] =(short)((short)pcmFrame_8b2[x+1] << 7);
                            // 3 - 取两个声道的平均值
                            //ush1 = (short)pcmFrame_8b2[x+0];
                            //ush2 = (short)pcmFrame_8b2[x+1];
                            //ush = (ush1 + ush2) >> 1;
                            //speech[y] = (short)((short)ush << 7);
                   }
         }
         else
         if (nBitsPerSample==16 && nChannels==1)
         {
                   nRead = fread(pcmFrame_16b1, (nBitsPerSample/8), PCM_FRAME_SIZE*nChannels, fpwave);
                   for(x=0; x<PCM_FRAME_SIZE; x++)
                   {
                            speech[x] = (short)pcmFrame_16b1[x+0];
                   }
         }
         else
         if (nBitsPerSample==16 && nChannels==2)
         {
                   nRead = fread(pcmFrame_16b2, (nBitsPerSample/8), PCM_FRAME_SIZE*nChannels, fpwave);
                   for( x=0, y=0; y<PCM_FRAME_SIZE; y++,x+=2 )
                   {
                            //speech[y] = (short)pcmFrame_16b2[x+0];
                            speech[y] = (short)((int)((int)pcmFrame_16b2[x+0] + (int)pcmFrame_16b2[x+1])) >> 1;
                   }
         }
 
         // 如果读到的数据不是一个完整的PCM帧, 就返回0
         if (nRead<PCM_FRAME_SIZE*nChannels) return 0;
 
         return nRead;
}
 
// WAVE音频采样频率是8khz 
// 音频样本单元数 = 8000*0.02 = 160 (由采样频率决定)
// 声道数 1 : 160
//        2 : 160*2 = 320
// bps决定样本(sample)大小
// bps = 8 --> 8位 unsigned char
//       16 --> 16位 unsigned short
int EncodeWAVEFileToAMRFile(const char* pchWAVEFilename, const char* pchAMRFileName, int nChannels, int nBitsPerSample)
{
         FILE* fpwave;
         FILE* fpamr;
 
         /* input speech vector */
         short speech[160];
 
         /* counters */
         int byte_counter, frames = 0, bytes = 0;
 
         /* pointer to encoder state structure */
         int *enstate;
         
         /* requested mode */
         enum Mode req_mode = MR122;
         int dtx = 0;
 
         /* bitstream filetype */
         unsigned char amrFrame[MAX_AMR_FRAME_SIZE];
 
         fpwave = fopen(pchWAVEFilename, "rb");
         if (fpwave == NULL)
         {
                   return 0;
         }
 
         // 创建并初始化amr文件
         fpamr = fopen(pchAMRFileName, "wb");
         if (fpamr == NULL)
         {
                   fclose(fpwave);
                   return 0;
         }
         /* write magic number to indicate single channel AMR file storage format */
         bytes = fwrite(AMR_MAGIC_NUMBER, sizeof(char), strlen(AMR_MAGIC_NUMBER), fpamr);
 
         /* skip to pcm audio data*/
         SkipToPCMAudioData(fpwave);
 
         enstate = (int*)Encoder_Interface_init(dtx);
 
         while(1)
         {
                   // read one pcm frame
                   if (!ReadPCMFrame(speech, fpwave, nChannels, nBitsPerSample)) break;
 
                   frames++;
 
                   /* call encoder */
                   byte_counter = Encoder_Interface_Encode(enstate, req_mode, speech, amrFrame, 0);
 
                   bytes += byte_counter;
                   fwrite(amrFrame, sizeof (unsigned char), byte_counter, fpamr );
         }
 
         Encoder_Interface_exit(enstate);
 
         fclose(fpamr);
         fclose(fpwave);
 
         return frames;
}

// 从WAVE流读一个完整的PCM音频帧
// 返回值: 0-错误 >0: 完整帧大小
int ReadPCMFrameEx(short speech[], const char* pBuff, int offset, int len, int nChannels, int nBitsPerSample)
{
    int nRead = 0;
    int x = 0, y=0;
    unsigned short ush1=0, ush2=0, ush=0;
 
    // 原始PCM音频帧数据
    unsigned char pcmFrame_8b1[PCM_FRAME_SIZE];
    unsigned char pcmFrame_8b2[PCM_FRAME_SIZE<<1];
    unsigned short pcmFrame_16b1[PCM_FRAME_SIZE];
    unsigned short pcmFrame_16b2[PCM_FRAME_SIZE<<1];
	
	if (offset >= len) 
		return 0;
 
    if (nBitsPerSample==8 && nChannels==1)
    {
		nRead = (nBitsPerSample/8) * PCM_FRAME_SIZE * nChannels;
		if (offset + nRead > len)
			return 0;
		memcpy(pcmFrame_8b1, pBuff + offset, nRead);
        for(x=0; x<PCM_FRAME_SIZE; x++)
        {
            speech[x] =(short)((short)pcmFrame_8b1[x] << 7);
        }
    }
    else if (nBitsPerSample==8 && nChannels==2)
    {
		nRead = (nBitsPerSample/8) * PCM_FRAME_SIZE * nChannels;
		if (offset + nRead > len)
			return 0;
		memcpy(pcmFrame_8b2, pBuff + offset, nRead);
        for( x=0, y=0; y<PCM_FRAME_SIZE; y++,x+=2 )
        {
            // 1 - 取两个声道之左声道
            speech[y] =(short)((short)pcmFrame_8b2[x+0] << 7);
            // 2 - 取两个声道之右声道
            //speech[y] =(short)((short)pcmFrame_8b2[x+1] << 7);
            // 3 - 取两个声道的平均值
            //ush1 = (short)pcmFrame_8b2[x+0];
            //ush2 = (short)pcmFrame_8b2[x+1];
            //ush = (ush1 + ush2) >> 1;
            //speech[y] = (short)((short)ush << 7);
        }
    }
    else if (nBitsPerSample==16 && nChannels==1)
    {
		nRead = (nBitsPerSample/8) * PCM_FRAME_SIZE * nChannels;
		if (offset + nRead > len)
			return 0;		
		memcpy(pcmFrame_16b1, pBuff + offset, nRead);
		for(x=0; x<PCM_FRAME_SIZE; x++)
		{
			speech[x] = (short)pcmFrame_16b1[x+0];
		}
	}
	else if (nBitsPerSample==16 && nChannels==2)
    {
		nRead = (nBitsPerSample/8) * PCM_FRAME_SIZE * nChannels;
		if (offset + nRead > len)
			return 0;		
		memcpy(pcmFrame_16b2, pBuff + offset, nRead);
        for( x=0, y=0; y<PCM_FRAME_SIZE; y++,x+=2 )
        {
            //speech[y] = (short)pcmFrame_16b2[x+0];
            speech[y] = (short)((int)((int)pcmFrame_16b2[x+0] + (int)pcmFrame_16b2[x+1])) >> 1;
        }
    }
 
    // 如果读到的数据不是一个完整的PCM帧, 就返回0
    if (nRead<PCM_FRAME_SIZE*nChannels) return 0;
 
    return nRead;
}

// WAVE音频采样频率是8khz 
// 音频样本单元数 = 8000*0.02 = 160 (由采样频率决定)
// 声道数 1 : 160
//        2 : 160*2 = 320
// bps决定样本(sample)大小
// bps = 8 --> 8位 unsigned char
//       16 --> 16位 unsigned short
const char* EncodeAMR(const char* wavBuf, int lenBuf, int nChannels, int nBitsPerSample, int* pBuffLen)
{
	char* destBuf = malloc(lenBuf);
	int lenDest = lenBuf;
	int usedDest = 0;
    /* input speech vector */
    short speech[160];
    /* counters */
    int byte_counter, frames = 0, bytes = 0;
    /* pointer to encoder state structure */
    int *enstate;
    /* requested mode */
    enum Mode req_mode = MR122;
    int dtx = 0;
    /* bitstream filetype */
    unsigned char amrFrame[MAX_AMR_FRAME_SIZE];
	int srcOffset = 0;
	int curLen = 0;
 
    /* write magic number to indicate single channel AMR file storage format */
    memcpy(destBuf, AMR_MAGIC_NUMBER, strlen(AMR_MAGIC_NUMBER));
	usedDest += strlen(AMR_MAGIC_NUMBER);
 
    enstate = (int*)Encoder_Interface_init(dtx);
 
    while(1)
    {
		curLen = ReadPCMFrameEx(speech, wavBuf, srcOffset, lenBuf, nChannels, nBitsPerSample);
        // read one pcm frame
        if (curLen <= 0) 
			break;
		
		frames++;
 
		/* call encoder */
        byte_counter = Encoder_Interface_Encode(enstate, req_mode, speech, amrFrame, 0);
 
        bytes += byte_counter;
		memcpy(destBuf + usedDest, amrFrame, byte_counter);
		usedDest += byte_counter;
    }
 
    Encoder_Interface_exit(enstate);
 
    return destBuf;
}