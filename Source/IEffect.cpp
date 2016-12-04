/***************************************************************

							IEffect Interface

***********************************************************/
#include "EffectCompiler.h"

void Noise3D::Effect::IPass::SetVS(const NoiseEffectCompiler::N_SHADER_DESC & shader)
{
	mVS = shader; ;
}

void Noise3D::Effect::IPass::SetGS(const NoiseEffectCompiler::N_SHADER_DESC & shader)
{
	mGS = shader;
}

void Noise3D::Effect::IPass::SetPS(const NoiseEffectCompiler::N_SHADER_DESC & shader)
{
	mPS = shader;
}

NoiseEffectCompiler::N_SHADER_DESC * Noise3D::Effect::IPass::GetVS()
{
	if (mVS.entryPoint != "" && mVS.version != "")
		return &mVS; 
	else 
		return nullptr;
}

NoiseEffectCompiler::N_SHADER_DESC * Noise3D::Effect::IPass::GetGS()
{
	if (mGS.entryPoint != "" && mGS.version != "")
		return &mGS; 
	else 
		return nullptr;
}

NoiseEffectCompiler::N_SHADER_DESC * Noise3D::Effect::IPass::GetPS()
{
	if (mPS.entryPoint != "" && mPS.version != "")
		return &mPS; 
	else 
		return nullptr;
}

Noise3D::Effect::IPass::IPass()
{
}

Noise3D::Effect::IPass::~IPass()
{
}





Noise3D::Effect::ITechnique::ITechnique()
	:NoiseEffectCompiler::IFactory<IPass>(32)
{
}

Noise3D::Effect::ITechnique::~ITechnique()
{
}





Noise3D::Effect::IEffect::IEffect()
	: NoiseEffectCompiler::IFactory<ITechnique>(1000)
{
}

bool Noise3D::Effect::IEffect::Init(const std::string & nxoFilePath)
{
	return false;
}


