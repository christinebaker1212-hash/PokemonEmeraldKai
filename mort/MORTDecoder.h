// SubDrag's MORT decoder (Pokemon Stadium's announcer), from jombo23/N64-Tools
// N64SoundListToolUpdated/N64SoundLibrary (public domain: the Unlicense). Kept as it was but for the encoder
// (MFC) being left out and its 32-bit longs spelled uint32_t / int32_t (mort/README.txt).
// Reverse Engineered by SubDrag from Pokemon Stadium
#pragma once

#include <vector>
#include <stdio.h>
#include <stdint.h>

class CMORTDecoder
{
public:
	CMORTDecoder(void);
	~CMORTDecoder(void);

public:
	bool Decode(unsigned char* ROM, int romSize, uint32_t address, uint32_t length, std::vector<unsigned short>& pcmSamples);

	static unsigned short CharArrayToShort(unsigned char* currentSpot);
	static uint32_t CharArrayToLong(unsigned char* currentSpot);
	static void WriteLongToBuffer(unsigned char* Buffer, uint32_t address, uint32_t data);
	static void WriteShortToBuffer(unsigned char* Buffer, uint32_t address, unsigned short data);
	static unsigned short Flip16Bit(unsigned short ShortValue);
	static uint32_t Flip32Bit(uint32_t inLong);
	static void WriteLongToFile(FILE* Buffer, uint32_t data);
	static void WriteShortToFile(FILE* Buffer, unsigned short data);

private:
	unsigned short buffer800D2940Predictor[0xA0];
	unsigned short lastPredictorUpdateBase; // 2-bytes buffer800D2940Predictor[0x189]
	bool currentSmootherPredictor; // 2-bytes buffer800D2940Predictor[0x166]
	signed short smootherPredictorA[8]; // 2-bytes buffer800D2940Predictor[0x168-0x178]
	signed short smootherPredictorB[8]; // 2-bytes buffer800D2940Predictor[0x178-0x188]
	int numberSkipResetPredictorCheck; // 2-bytes buffer800D2940Predictor[0x18A]
	int numberResetPredictor; // 2-bytes buffer800D2940Predictor[0x18C]
	uint32_t lastSampleValue; // 2-bytes buffer800D2940Predictor[0x164]
	int32_t sampleBuffer[8]; // 4-bytes buffer800D2940Predictor[0x140-0x15C]
	uint32_t buffer800D2940Subtraction;
	//800D2AD0 Pointer to function 800455DC (pull data?)
	// Need evaluate if these are part of buffer or not and the rest around there
	uint32_t variable800D2AD4ROMAddressMORTData;
	unsigned char buffer800D2AD8IntermediateValue[0x1400];
	uint32_t buffer800D2AD8Subtraction;
	unsigned char buffer800D3ED8MORTRawInputDataBuffer[0x1000]; // TWINE 800E1C60
	unsigned short variable800D4ED8Amountofsoundleft1_FirstWord; // TWINE 800E2C60
	unsigned short variable800D4EDA_3E80FirstWord;
	uint32_t variable800D4EDCAmountofCompressedWordsLeft_SecondWord;
	uint32_t variable800D4EE0;
	uint32_t variable800D4EE4;
	uint32_t variable800D4EE8;
	uint32_t variable800D4EEC;
	uint32_t variable800D4EF0;
	uint32_t variable800D4EF4;
	uint32_t variable800D4EFCInputChunkCurrentReadBitPosition;
	uint32_t variable800D4F00InputChunkUsed; // TWINE 800E2C8C
	uint32_t variable800D4F04InputChunkAmountLeft; // TWINE 800E2C90
	uint32_t variable800D4F08;
	uint32_t variable800D4F0C; //// TWINE 800E2C98
	unsigned char variable800D4F10Status1; // TWINE 800E2C9C
	unsigned char variable800D4F11Status2; //01 loading, 02 loaded, 03 playing, 04 ended 
	unsigned char variable800D4F12Status3; // TWINE 800E2C9E
	unsigned char* buffer800D4F20OutputBuffer;
	uint32_t buffer800D4F20Subtraction;

	uint32_t variable800FCED0;
	uint32_t variable800FCEDCPredictorPointer800D2940;
	uint32_t variable800FCEE8;
	uint32_t variable800FCEECCounter;
	unsigned char variable800FCEF0MORTStatus1; //Status of sound, 00 = Loading, 02 = none, 04 = Playing
	unsigned char variable800FCEF1MORTStatus2; //Status of sound, 00 = Loading, 02 = none, 04 = Playing
	uint32_t variable800FCEFCROMMORTSoundAddress;
	uint32_t variable800FCF04;
	uint32_t variable800FCF0CVolume;
	unsigned char variable800FCF20;
	unsigned char variable800FCF21;
	uint32_t variable800FCF2COutputBufferPointer800D4F20;
	uint32_t variable800FCF30CounterPlay1;
	uint32_t variable800FCF34CounterPlay2;
	uint32_t variable800FCF38CounterIncrementVariable1;
	uint32_t variable800FCF3CCounterIncrementVariable2;

	void Function800456D0();
	void Function80045780(unsigned char* ROM, bool& started, std::vector<unsigned short>& pcmSamples);
	void Function800459E0(uint32_t A1Param, uint32_t& V0);
	void Function8005E3A0(uint32_t A1Param, uint32_t A2Param);
	void ClearBuffer_Function80057FD0(uint32_t A0Param, uint32_t A1Param);
	void Function80059120(uint32_t A0Param, uint32_t A1Param);
	void Function800455DC(unsigned char* ROM, uint32_t A0Param, uint32_t A1Param, uint32_t A2Param);
	void Function8005E2F0(uint32_t A0Param, uint32_t A1Param);
	void Function8005E0F0();
	void Function80056AD0(uint32_t& V0);
	void Function80062A90(uint32_t& V0);
	void Function80045FF0(uint32_t currentIntermediateValueOffset, std::vector<unsigned short>& pcmSamples);
	void Function80045C78(int currentIntermediateValueOffset, unsigned short shortsSP60[4][0xD], unsigned short shortsSPC8[0x4], unsigned short shortsSPD0[0x4], unsigned short stackBuffer2Offsets[0x4], unsigned short shortsSPE0[0x4], unsigned short shortsSPE8[8], std::vector<unsigned short>& pcmSamples);
	void Function80048590(unsigned short shortsSPC8Value, unsigned short stackBuffer2Offset, unsigned short stackBuffer2[0x28], unsigned short shortsSP60[0xD]);
	void Function80048684(unsigned short shortsSPE0Value, unsigned short shortsSPD0Value, unsigned short stackBuffer2[0x28]);
	void Function80045A80(uint32_t currentIntermediateValueOffset, unsigned short shortsSPE8[8], std::vector<unsigned short>& pcmSamples);
	void Function80048904(uint32_t currentIntermediateValueOffset, std::vector<unsigned short>& pcmSamples);
	void CallT380048XXXFunction(uint32_t& T2, uint32_t T3, uint32_t T4);
	void Function80048AFC(uint32_t& T2, uint32_t T4);
	void Function80048B14(uint32_t& T2, uint32_t T4);
	void Function80048B24(uint32_t& T2, uint32_t T4);
	void Function80048B3C(uint32_t& T2);
	void Function80048740(uint32_t intermediateValueOffset, uint32_t predictorBufferOffset, int countValues, signed short adjusters[8], uint32_t& A3, uint32_t& S0, uint32_t& S1, uint32_t& S2, uint32_t& S3, uint32_t& S4, uint32_t& S5, uint32_t& S6, uint32_t& S7, std::vector<unsigned short>& pcmSamples);
	void Function80048A58(uint32_t T3Param, signed short adjuster[8]);
	void Function80045A48();
	void Function80045A68();

	uint32_t ReadBitsFrom80045FF0Buffer(int numberBits, unsigned char* buffer800D3ED8MORTRawInputDataBuffer, uint32_t& currentInputData, uint32_t& bitsleft, uint32_t& currentOverallBitPosition);

	FILE* outDebug;
	bool setInputChunk;

	void WriteBitsTo80045FF0Buffer(unsigned char* buffer, int& bufferOffset, int& bufferBitOffset, int numBits, unsigned char value);
};
