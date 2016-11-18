/********************************************************************

							FILE OUTPUT

			Intermediate Effect Bytecode generator and
			file writer, .nxo will be output by this class
			
*********************************************************************/
#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

void IFileWriter::OutputBinary(
	const std::string& outPath,
	const std::vector<N_UNIQUE_SHADER>& uniqueShaderList,
	const IEffect* pEffect)
{
	//use information generated in previous stages, to output 
	//---------------------INTERMEDIATE EFFECT BYTECODE------------------------

	//!!!!!There is a critical, urgent problem that need to be considered. If I handwrite an
	//"N_CBUFFUER_INFO" (or other class) 's SERIALIZER, UN-SERIALIZE operation must
	//highly depend on the definition of class. Then here comes the problem  :
	// :: how to synchronize the class definition in 2 different systems where Serialize or
	//Un-serialize happens??????????????????????????
	//Perhaps, I'll put those important declaration in a same header, and 
	//put this compiler project into Noise3D SOLUTION!!

	std::fstream outFile(outPath, std::ios::binary | std::ios::out);

	mFileBuffer.clear();

	//-----file start, used to recognized this compiled Noise Effect binary output (.nxo)
	mFunction_WriteBuffer(NOISE_EBI_MAGIC_NUMBER);

	//------ global resources----
	for (auto e : uniqueCBuffers)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_CB);//1byte uchar identifier
		mFunction_WriteBuffer((uint32_t)sizeof(N_CBUFFER_INFO));//4 byte for info size
		mFunction_WriteBuffer(e);//forcedly serialization
	}

	for (auto e : uniqueTBuffers)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_TB);//1byte uchar identifier
		mFunction_WriteBuffer((uint32_t)sizeof(N_TBUFFER_INFO));//4 byte data size
		mFunction_WriteBuffer(e);//forcedly serialization
	}

	for (auto e : uniqueSamplers)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_TB);//1byte uchar identifier
		mFunction_WriteBuffer((uint32_t)sizeof(N_SAMPLER_INFO));//4 byte data size
		mFunction_WriteBuffer(e);//forcedly serialization
	}

	for (auto e : uniqueTexResources)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_TB);//1byte uchar identifier
		mFunction_WriteBuffer((uint32_t)sizeof(N_TEXTURE_RESOURCE_INFO));//4 byte data size
		mFunction_WriteBuffer(e);//forcedly serialization
	}

	//-----Effect hierarchy

	//Desc: EBI(1 byte) + Technique Count(2 byte) +sub-block
	//Father:Global
	//Begin of: new effect block
	uint16_t techCount = pEffect->GetObjectCount();
	mFunction_WriteBuffer(NOISE_EBI_NEW_EFFECT);
	mFunction_WriteBuffer(techCount);

	for (uint16_t i = 0;i < techCount;++i)
	{
		//Desc: EBI(1 byte) + Passes Count(2 bytes)  + Technique name(string)+ sub-block
		//Father: Effect Block
		//Begin of : new technique block
		ITechnique* pTech = pEffect->GetObjectPtr(i);
		uint16_t passCount = pTech->GetObjectCount();
		std::string pTechName = pEffect->GetUID(i);

		mFunction_WriteBuffer(NOISE_EBI_NEW_TECHNIQUE);
		mFunction_WriteBuffer(passCount);
		mFunction_WriteBuffer(pTechName);

		for (uint16_t j=0;j < passCount;++j)
		{
			//Desc: EBI(1 byte) + Shaders Count(2 bytes) +  Pass Name(string)+ sub-block
			//Father: Technique Block
			//Begin of : new pass block
			IPass* pPass = pTech->GetObjectPtr(j);


			mFunction_WriteBuffer(NOISE_EBI_NEW_PASS);

		}
	}

}

void  IFileWriter::mFunction_WritePass(IPass * pPass)
{
}

/************************************************

								PRIVATE

************************************************/

template<typename T>
inline void IFileWriter::mFunction_WriteBuffer(T val)
{
	constexpr size_t size = sizeof T;
	uchar charArr[size];
	memcpy_s(charArr, size, (uchar*)&val, size);

	for(int i=0;i<size;i++)mFileBuffer.push_back(charArr[i]);
}

//specialization for string
template<>
void IFileWriter::mFunction_WriteBuffer(std::string str)
{
	for (int i = 0;i< str.size();i++)mFileBuffer.push_back(str.at(i));
	mFileBuffer.push_back('\0');//as a terminator
}

//specialization for string
template<>
inline void IFileWriter::mFunction_WriteBuffer(const char*  str)
{
	char* p = (char*)str;
	while (*p != '\0') { mFileBuffer.push_back(*p);++p; }
	mFileBuffer.push_back('\0');//as a terminator
}