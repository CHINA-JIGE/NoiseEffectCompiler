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

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include "d3d11.h"
#include "d3dcompiler.h"
#include "IFactory.h" 
#include "_ShaderInclude.h"
#include "EffectTokenizer.h"
#include "EffectParser.h"

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

		bool		mFunction_ParseEffect();

		std::string mTargetFileName;
		std::string mEffectFileName;
		std::vector<unsigned char> mEffectFileBuffer;

		ID3D11ShaderReflection* pSR;


	};

}

