/***************************************************************

							IEffect Parser

				Parse passName/EntryPoint/Technique name etc.
				information from noiseEffect file

***********************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

/***********************EFFECT   PARSER*****************************/


NoiseEffectCompiler::IEffectParser::IEffectParser()
{
}

void  NoiseEffectCompiler::IEffectParser::Parse(const std::vector<N_TokenInfo>& list)
{

}

/***********************TECHNIQUE *****************************/

/***********************PASS*****************************/

/***********************SHADER*****************************/

void NoiseEffectCompiler::IPass::SetVS(IShader shader)
{
	mVS = shader;
}

void NoiseEffectCompiler::IPass::SetGS(IShader shader)
{
	mGS = shader;
}

void NoiseEffectCompiler::IPass::SetPS(IShader shader)
{
	mPS = shader;
}

void NoiseEffectCompiler::IPass::GetVS(IShader & outShader)
{
	outShader = mVS;
}

void NoiseEffectCompiler::IPass::GetGS(IShader & outShader)
{
	outShader = mGS;
}

void NoiseEffectCompiler::IPass::GetPS(IShader & outShader)
{
	outShader = mPS;
}
