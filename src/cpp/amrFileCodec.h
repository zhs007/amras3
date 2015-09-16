#ifndef amrFileCodec_h
#define amrFileCodec_h
 
#define AMR_MAGIC_NUMBER "#!AMR\n"
 
#define PCM_FRAME_SIZE 160 // 8khz 8000*0.02=160
#define MAX_AMR_FRAME_SIZE 32
#define AMR_FRAME_COUNT_PER_SECOND 50
//int amrEncodeMode[] = {4750, 5150, 5900, 6700, 7400, 7950, 10200, 12200}; // amr ���뷽ʽ
 
typedef struct
{
         char chChunkID[4];
         int nChunkSize;
}XCHUNKHEADER;
 
typedef struct
{
         short nFormatTag;
         short nChannels;
         int nSamplesPerSec;
         int nAvgBytesPerSec;
         short nBlockAlign;
         short nBitsPerSample;
}WAVEFORMAT;
 
typedef struct
{
         short nFormatTag;
         short nChannels;
         int nSamplesPerSec;
         int nAvgBytesPerSec;
         short nBlockAlign;
         short nBitsPerSample;
         short nExSize;
}WAVEFORMATX;
 
typedef struct
{
         char chRiffID[4];
         int nRiffSize;
         char chRiffFormat[4];
}RIFFHEADER;
 
typedef struct
{
         char chFmtID[4];
         int nFmtSize;
         WAVEFORMAT wf;
}FMTBLOCK;
 
// WAVE��Ƶ����Ƶ����8khz 
// ��Ƶ������Ԫ�� = 8000*0.02 = 160 (�ɲ���Ƶ�ʾ���)
// ������ 1 : 160
//        2 : 160*2 = 320
// bps��������(sample)��С
// bps = 8 --> 8λ unsigned char
//       16 --> 16λ unsigned short
int EncodeWAVEFileToAMRFile(const char* pchWAVEFilename, const char* pchAMRFileName, int nChannels, int nBitsPerSample);
 
// ��AMR�ļ������WAVE�ļ�
int DecodeAMRFileToWAVEFile(const char* pchAMRFileName, const char* pchWAVEFilename);
 
#endif