/********************************************************************

							FILE OUTPUT

			Intermediate Effect Bytecode generator and
			file writer, .nxo will be output by this class
			
*********************************************************************/
#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

bool IFileWriter::OutputBinaryToFile(
	const std::string& outPath,
	const std::vector<N_UNIQUE_SHADER>& uniqueShaderList,
	const IEffectDesc* pEffect)
{
	//use information generated in previous stages, to output 
	//---------------------INTERMEDIATE EFFECT BYTECODE-----------------------
	//!!!!!There is a critical, urgent problem that need to be considered. If I handwrite an
	//"N_CBUFFUER_INFO" (or other class) 's SERIALIZER, UN-SERIALIZE operation must
	//highly depend on the definition of class. Then here comes the problem  :
	// :: how to synchronize the class definition in 2 different systems where Serialize or
	//Un-serialize happens??????????????????????????
	//Perhaps, I'll put those important declaration in a same header, and 
	//put this compiler project into Noise3D SOLUTION!!

	mFileBuffer.clear();

	//Desc : 4 bytes header
	//Father : Global
	//Begin of : magic number
	//-----used to recognized noiseEffect file
	mFunction_WriteBuffer(NOISE_EBI_MAGIC_NUMBER_VERSION_01);

	std::unordered_map<std::string,N_CBUFFER_INFO> uniqueCBs;
	std::unordered_map<std::string,N_TBUFFER_INFO> uniqueTBs;
	std::unordered_map<std::string,N_TEXTURE_RESOURCE_INFO> uniqueTextures;
	std::unordered_map<std::string,N_SAMPLER_INFO> uniqueSamplers;
	std::vector<std::pair<std::string,ID3DBlob*>> uniqueShaderBinaryBlocks;

	int shaderIndex = 0;
	//build some unique resources lists
	for (auto& s : uniqueShaderList)
	{
		for (auto& cb : s.cbInfo)
			uniqueCBs.insert(std::make_pair(cb.name,cb));

		for (auto& tb : s.tbInfo)
			uniqueTBs.insert(std::make_pair(tb.name, tb));

		for (auto& tex : s.texInfo)
			uniqueTextures.insert(std::make_pair(tex.name, tex));

		for (auto& sp : s.samplerInfo)
			uniqueSamplers.insert(std::make_pair(sp.name, sp));

		//shaders'  UID-binaryBlock pair
		uniqueShaderBinaryBlocks.push_back(std::make_pair(s.GetUID(),s.pBlob));
	}

	mFunction_WriteGlobalResources(
		uniqueCBs,
		uniqueTBs,
		uniqueTextures,
		uniqueSamplers,
		uniqueShaderBinaryBlocks
	);

	mFunction_WriteEffectHierarchy(pEffect);

	//write to file
	std::ofstream outFile(outPath,std::ios::binary);
	if (!outFile.is_open()) { std::cout << "File Writer: Open output file failed!";return false; }
	outFile.write((char*)&mFileBuffer.at(0), mFileBuffer.size());
	outFile.close();

	return true;
}

/************************************************

								PRIVATE

************************************************/

void IFileWriter::mFunction_WriteGlobalResources(
	const std::unordered_map<std::string, N_CBUFFER_INFO>& uniqueCBs,
	const std::unordered_map<std::string, N_TBUFFER_INFO>& uniqueTBs,
	const std::unordered_map<std::string, N_TEXTURE_RESOURCE_INFO>& uniqueTextures,
	const std::unordered_map<std::string, N_SAMPLER_INFO>& uniqueSamplers,
	const std::vector<std::pair<std::string, ID3DBlob*>> & uniqueShaderBinarys)
{

	//WARNING!!!!! : In the xxxInfo struct , there is a std::string that can't be directly write to 
	//file , because string is a variable-length object


	//CONSTANT BUFFERS

	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	for (auto& e : uniqueCBs)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_CB);//1byte uchar identifier
		mFunction_WriteBuffer(e.second.name);
		mFunction_WriteBuffer((uint32_t)sizeof(N_CBUFFER_INFO));//4 byte for info size
		mFunction_WriteBuffer(e.second);//serialization
	}

	//TEXTURE BUFFERS

	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	for (auto& e : uniqueTBs)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_TB);//1byte uchar identifier
		mFunction_WriteBuffer(e.second.name);
		mFunction_WriteBuffer((uint32_t)sizeof(N_TBUFFER_INFO));//4 byte data size
		mFunction_WriteBuffer(e.second);//serialization
	}
	
	//SAMPLERS

	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	for (auto& e : uniqueSamplers)
	{
		mFunction_WriteBuffer(NOISE_EBI_NAME_SAMPLER);//1byte uchar identifier
		mFunction_WriteBuffer(e.second.name);
		mFunction_WriteBuffer((uint32_t)sizeof(N_SAMPLER_INFO));//4 byte data size
		mFunction_WriteBuffer(e.second);//serialization
	}

	//	TEXTURE	/	SHADER RESOURCE

	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	for (auto& e : uniqueTextures)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_TEXTURE);//1byte uchar identifier
		mFunction_WriteBuffer(e.second.name);
		mFunction_WriteBuffer((uint32_t)sizeof(N_TEXTURE_RESOURCE_INFO));//4 byte data size
		mFunction_WriteBuffer(e.second);//serialization
	}


	//SHADER BINARY BLOCK

	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	for (auto& e : uniqueShaderBinarys)
	{
		mFunction_WriteBuffer(NOISE_EBI_NEW_SHADER_BINARY_BLOCK);//1byte uchar identifier
		mFunction_WriteBuffer(e.first);
		mFunction_WriteBuffer((uint32_t)sizeof(e.second->GetBufferSize()));//4 byte data size
		mFunction_WriteBuffer(e.second);//write the whole binary block of ID3DBlob ; function overloaded
	}

}

void IFileWriter::mFunction_WriteEffectHierarchy(const IEffectDesc* pEffect)
{

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
		ITechniqueDesc* pTech = pEffect->GetObjectPtr(i);
		uint16_t passCount = pTech->GetObjectCount();
		std::string techName = pEffect->GetUID(i);

		mFunction_WriteBuffer(NOISE_EBI_NEW_TECHNIQUE);
		mFunction_WriteBuffer(passCount);
		mFunction_WriteBuffer(techName);


		//Desc: EBI(1 byte)+  Pass Name(string)+ sub-block
		//Father: Technique Block
		//Begin of : new pass block
		for (uint16_t j = 0;j < passCount;++j)
		{
			IPassDesc* pPass = pTech->GetObjectPtr(j);
			std::string passName = pTech->GetUID(j);

			mFunction_WriteBuffer(NOISE_EBI_NEW_PASS);
			mFunction_WriteBuffer(passName);
			mFunction_WritePass(pPass);
		}
	}

}

void  IFileWriter::mFunction_WritePass(IPassDesc * pPass)
{
	N_SHADER_DESC* pVS = pPass->GetVS();
	N_SHADER_DESC* pGS = pPass->GetGS();
	N_SHADER_DESC* pPS = pPass->GetPS();

	if (pVS != nullptr)
	{
		//a new Vertex shader block

		//Desc: EBI(1 byte)  + sub-block
		//Father: Pass Block
		//Begin of : all kinds of new shader blocks
		//----- Create new shaders, and identify a start of sub-blocks
		mFunction_WriteBuffer(NOISE_EBI_NEW_VS);
		mFunction_WriteShader(pVS);
	}

	if (pGS != nullptr)
	{
		//a new Vertex shader block

		//Desc: EBI(1 byte)  + sub-block
		//Father: Pass Block
		//Begin of : all kinds of new shader blocks
		//----- Create new shaders, and identify a start of sub-blocks
		mFunction_WriteBuffer(NOISE_EBI_NEW_GS);
		mFunction_WriteShader(pGS);
	}

	if (pPS != nullptr)
	{
		//a new Vertex shader block

		//Desc: EBI(1 byte)  + sub-block
		//Father: Pass Block
		//Begin of : all kinds of new shader blocks
		//----- Create new shaders, and identify a start of sub-blocks
		mFunction_WriteBuffer(NOISE_EBI_NEW_PS);
		mFunction_WriteShader(pPS);
	}

}

void IFileWriter::mFunction_WriteShader(N_SHADER_DESC * pShader)
{
	//names of CBuffers which belong to this shader
	for (auto& e : pShader->cbNames)
	{
		mFunction_WriteBuffer(NOISE_EBI_NAME_CB);
		mFunction_WriteBuffer(e);
	}

	//names of TBuffers which belong to this shader
	for (auto& e : pShader->tbNames)
	{
		mFunction_WriteBuffer(NOISE_EBI_NAME_TB);
		mFunction_WriteBuffer(e);
	}

	//names of Textures which belong to this shader
	for (auto& e : pShader->texNames)
	{
		mFunction_WriteBuffer(NOISE_EBI_NAME_TEXTURE);
		mFunction_WriteBuffer(e);
	}

	//names of Samplers which belong to this shader
	for (auto& e : pShader->samplerNames)
	{
		mFunction_WriteBuffer(NOISE_EBI_NAME_SAMPLER);
		mFunction_WriteBuffer(e);
	}

	//name of compiled Binary code block of current shader
	mFunction_WriteBuffer(NOISE_EBI_NEW_SHADER_BINARY_BLOCK);
	mFunction_WriteBuffer(pShader->GetUID());

};



template<typename T>
inline void IFileWriter::mFunction_BinaryWrite(const T& val)
{
	/*constexpr size_t size = sizeof T;
	uchar charArr[size];
	memcpy_s(charArr, size, (uchar*)&val, size);*/

	for(UINT i=0;i<sizeof(val);i++)mFileBuffer.push_back( *((uchar*)&val + i) );
}

//specialization for string
void  IFileWriter::mFunction_WriteBuffer(std::string str)
{
	for (UINT i = 0;i< str.size();i++)mFileBuffer.push_back(str.at(i));
	mFileBuffer.push_back('\0');//as a terminator
}

void IFileWriter::mFunction_WriteBuffer(const N_CBUFFER_INFO& val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(const N_TBUFFER_INFO& val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(const N_TEXTURE_RESOURCE_INFO& val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(const N_SAMPLER_INFO& val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(ID3DBlob * val)
{
	void* pBuff = val->GetBufferPointer();
	for (UINT i = 0;i<val->GetBufferSize();i++)mFileBuffer.push_back(*((char*)pBuff + i));
}

void IFileWriter::mFunction_WriteBuffer(uchar val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(uint16_t val)
{
	mFunction_BinaryWrite(val);
}

void IFileWriter::mFunction_WriteBuffer(uint32_t val)
{
	mFunction_BinaryWrite(val);
}

//specialization for string
 void IFileWriter::mFunction_WriteBuffer(const char*  str)
{
	char* p = (char*)str;
	while (*p != '\0') { mFileBuffer.push_back(*p);++p; }
	mFileBuffer.push_back('\0');//as a terminator
}