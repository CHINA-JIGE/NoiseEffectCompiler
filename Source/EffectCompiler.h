/***************************************************************

							NOISE EFFECT COMPILER

			Based on D3DCompile and other "Reflection" sort of 
			D3D api , compiler an .fx file into Noise Effect Binary
			(but some syntax might be extended).
			Noise Effect framework is designed to organized shaders
			in some forms, and this compiler is used to generate
			sufficient information for Noise Effect Framework

***********************************************************/

#pragma once
#pragma warning (disable : 4005)//macro redefined WIN8 SDKºÍDXSDK

#define ERROR_MSG(msg)\
std::cout<<msg<<std::endl;\

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <d3d11shader.h>
#include <D3DCompiler.h>
#include "IFactory.h" 
#include "_ShaderInclude.h"
#include "EffectTokenizer.h"
#include "EffectParser.h"
#include "ShaderReflector.h"

namespace NoiseEffectCompiler
{

	class IEffectCompiler
	{
	public:

		IEffectCompiler();

		bool		ParseCommandLine(int argc, char* argv[]);

		bool		Compile();

	private:

		bool		mFunction_LoadParameters(int argc, char* argv[]);

		bool		mFunction_LoadFiles();

		bool		mFunction_ParseEffect(std::vector<N_Shader>& outShaderList,std::vector<std::string>& outSourceFileList);

		bool		mFunction_CompileHLSL(const std::vector<N_Shader>& shaderList, const std::vector<std::string>& sourceFileList, std::vector<ID3DBlob*>& outCompiledShaderCode);

		bool		mFunction_ShaderReflection(const std::vector<N_Shader>& shaderInfoList,const std::vector<ID3DBlob*>& compiledShaderCode);

		bool		mFunction_OutputCompiledEffectFile();

		std::string mTargetFileName;
		std::string mEffectFileName;
		std::vector<unsigned char> mEffectFileBuffer;
		std::string mRelativePath;

	};

}

