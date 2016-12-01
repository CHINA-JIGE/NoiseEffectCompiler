/***************************************************************

					IMPORTANT CLASS DECLARATION

	Generation of intermediate EFFECT bytecode is the last
	step this compiler tends to do. But in Noise3D, intermediate
	bytecode must be re-read to create certain interfaces.
	Among those operations, techniques like Serialize and Un-Serialize
	might be used, thus synchronization of declarations of 
	certain important classes seems to be essential. (otherwise 
	Un-serialize can't function correctly because import/export
	the same block of data but organizing them in different way
	simply just won't work.  :)

	And, this header is to declare some common-use class
***********************************************************/

#pragma once

namespace NoiseEffectCompiler
{
	typedef unsigned char uchar;


	//---------------------------RESOURCE BINDING----------------------------
	struct N_BASIC_BIND_INFO
	{
		//the "slot" number  in the XSSetConstantBuffer,
		//a very important number to map resource in app to resource in shader!!
		UINT bindSlot;//startSlot

		UINT bindCount;// slotCount

		std::string name;
	};

	//---Constant Buffer
	struct N_CBUFFER_INFO :public N_BASIC_BIND_INFO
	{
		UINT byteSize;
	};

	//---Texture Buffer
	struct N_TBUFFER_INFO :public N_BASIC_BIND_INFO
	{
		UINT byteSize;
	};

	//---Texture Resources
	struct N_TEXTURE_RESOURCE_INFO :public N_BASIC_BIND_INFO
	{
		UINT numSamples;//sample of a texture

		UINT dimension;// 2D/3D  D3D_SRV_DIMENSION

		UINT returnType;//D3D_RESOURCE_RETURN_TYPE
	};

	//---Sampler
	struct N_SAMPLER_INFO :public N_BASIC_BIND_INFO {};


	//--------------------HIERARCHY----------------
	//	Effect
	//		|---Technique
	//		|			|---Pass
	//		|			|		|----VertexShader(crucial)
	//		|			|		|----PixelShader(crucial)
	//		|			|		|----GeometryShader(optional)
	//		|			|---Pass
	//		|			|		|----VertexShader(crucial)
	//		|			|		|----PixelShader(crucial)
	//		|			|		|----GeometryShader(optional)
	//		|			|  ...................
	//		|
	//		|---Technique
	//		|			|---Pass
	//		|			|		|   ...................
	//		|			|   ...................
	//		|
	//		| ...................
	//-----------------------------
	struct N_UNIQUE_SHADER
	{
		std::string entryPoint;
		std::string version;
		std::vector<N_CBUFFER_INFO> cbInfo;
		std::vector<N_TBUFFER_INFO> tbInfo;
		std::vector<N_TEXTURE_RESOURCE_INFO> texInfo;
		std::vector<N_SAMPLER_INFO> samplerInfo;
		ID3DBlob* pBlob;

		//to generate unique shader, which is identified by uid
		std::string GetUID()const  { return entryPoint + "@@" + version; }
	};

	struct N_SHADER_DESC
	{
		//only store names to provide a way to reference resources from global pool
		std::string entryPoint;
		std::string version;
		std::vector<std::string> cbNames;
		std::vector<std::string> tbNames;
		std::vector<std::string> texNames;
		std::vector<std::string> samplerNames;

		//to generate unique shader, which is identified by uid
		std::string GetUID()const  { return entryPoint + "@@" + version; }
	};


	class IPass
	{
	public:

		void SetVS(const N_SHADER_DESC& shader) { mVS = shader; };

		void SetGS(const N_SHADER_DESC& shader) { mGS = shader; };

		void SetPS(const N_SHADER_DESC& shader) { mPS = shader; };

		N_SHADER_DESC* GetVS() { if (mVS.entryPoint != "" && mVS.version != "")return &mVS; else return nullptr; };

		N_SHADER_DESC* GetGS() { if (mGS.entryPoint != "" && mGS.version != "")return &mGS; else return nullptr;};

		N_SHADER_DESC* GetPS() { if (mPS.entryPoint != "" && mPS.version != "")return &mPS; else return nullptr;
	};

	private:

		N_SHADER_DESC mVS;
		N_SHADER_DESC mGS;
		N_SHADER_DESC mPS;

	};


	class ITechnique : public IFactory<IPass>
	{
	public:

		//theoretically, pass count won't be limited,
		//but too many pass could cause performance overhead
		ITechnique() :IFactory<IPass>(32) {};

	private:
	};


	class IEffect : public IFactory<ITechnique>
	{
	public:

		//Root interface of Effect Framework, owns Technique child object
		//for a specific render effect
		IEffect() :IFactory<ITechnique>(100000) {};

	private:

	};



	//---------------------------INTERMEDIATE BYTECODE INSTRUCTION----------------------------
	//version : (2016.11.1)
	//EBI = Effect Bytecode Instruction
	//the structure of .nxo (NOISE EFFECT OUTPUT) will be similar to .3ds file with tree-like
	//hierarchy and can be parsed linearly maintaining a set of states variable.
	//and Instruction+Data description will be given in annotation before every constant enumeration.
	//(or refer to documentation)
	
	//Desc : 4 bytes header
	//Father : Global
	//Begin of : magic number
	//-----used to recognized noiseEffect file
	const UINT NOISE_EBI_MAGIC_NUMBER = 0xabcd1234;


	//Desc: EBI(1 byte) + NAME(string) + binaryBlockSize(4 bytes) + binaryData (variable)
	//Father : Global
	//Begin of : new GLOBAL BLOCK
	//-----	create resources/buffers in unique Global Resource Pool
	const uchar NOISE_EBI_NEW_CB = 10;
	const uchar NOISE_EBI_NEW_TB = 11;
	const uchar NOISE_EBI_NEW_TEXTURE = 12;
	const uchar NOISE_EBI_NEW_SAMPLER = 13;
	const uchar NOISE_EBI_NEW_SHADER_BINARY_BLOCK = 14;


	//----- Create new Effect/Technique/Pass belong to current father node
	//----- please carefully manager current states

	//Desc: EBI(1 byte) + Technique Count(2 byte) + sub-block
	//Father:Global
	//Begin of: new effect block
	const uchar NOISE_EBI_NEW_EFFECT = 0;
	
	//Desc: EBI(1 byte) + Passes Count(2 bytes)  + Technique name(string)+ sub-block
	//Father: Effect Block
	//Begin of : new technique block
	const uchar NOISE_EBI_NEW_TECHNIQUE = 1;

	//Desc: EBI(1 byte) +  Pass Name(string)+ sub-block
	//Father: Technique Block
	//Begin of : new pass block
	const uchar NOISE_EBI_NEW_PASS = 2;


	//Desc: EBI(1 byte)  + sub-block
	//Father: Pass Block
	//Begin of : all kinds of new shader blocks
	//----- Create new shaders, and identify a start of sub-blocks
	const uchar NOISE_EBI_NEW_VS = 200;
	const uchar NOISE_EBI_NEW_PS = 201;
	const uchar NOISE_EBI_NEW_GS = 202;


	//Desc: EBI (1 byte) + name(string)
	//Father : Shader Blocks
	//Begin of: names of bound resources 
	//----- resource/buffers can be REFERENCED BY NAME in global resource pool
	//----- and these resource (names) belong to current shader
	const uchar NOISE_EBI_NAME_CB = 60;
	const uchar NOISE_EBI_NAME_TB = 61;
	const uchar NOISE_EBI_NAME_TEXTURE = 62;
	const uchar NOISE_EBI_NAME_SAMPLER = 63;
	const uchar NOISE_EBI_NAME_SHADER_BINARY_BLOCK = 64;

};