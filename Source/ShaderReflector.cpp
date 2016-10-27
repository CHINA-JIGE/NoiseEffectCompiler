/*********************************************************************

							SHADER REFLECTOR

*********************************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

IShaderReflector::IShaderReflector()
{
}

bool IShaderReflector::Analyze(const std::vector<N_Shader>& shaderInfoList, const std::vector<ID3DBlob*>& compiledShaderCode)
{
	for (UINT i = 0;i < compiledShaderCode.size();i++)
	{
		//------------------------FOR EVERY SHADER------------------------ 

		//get compiled code
		ID3DBlob* pBlob = compiledShaderCode.at(i);

		if (!mFunction_ReflectShader(pBlob))return false;
	}

	return true;
}

bool IShaderReflector::mFunction_ReflectShader(ID3DBlob * compiledShaderCode)
{
	//Reflect -- more detail please refer to MSDN
	ID3D11ShaderReflection* pReflector = nullptr;

	D3DReflect(
		compiledShaderCode->GetBufferPointer(), 
		compiledShaderCode->GetBufferSize(), 
		IID_ID3D11ShaderReflection,
		(void**)&pReflector);

	//reflect shader fail
	if (pReflector == nullptr)
	{
		ERROR_MSG("---Failed to Reflect shader bytecode"); 
		return false;
	}

	//get  informations of a shader in a tree-like manner
	D3D11_SHADER_DESC shaderDesc;
	pReflector->GetDesc(&shaderDesc);

	if (!mFunction_ReflectConstantBuffer(pReflector, shaderDesc.ConstantBuffers))return false;

	return true;
}

bool IShaderReflector::mFunction_ReflectConstantBuffer(ID3D11ShaderReflection * pReflector, UINT cbCount)
{
	std::vector<ID3D11ShaderReflectionConstantBuffer*> cbReflectorList;

	//reflect constant buffer
	for (UINT i = 0;i < cbCount;i++)
	{
		ID3D11ShaderReflectionConstantBuffer* pCbReflector= pReflector->GetConstantBufferByIndex(i);
		if (pCbReflector == nullptr)return false;

		cbReflectorList.push_back(pCbReflector);

		//get constant buffer description
		D3D11_SHADER_BUFFER_DESC buffDesc;
		pCbReflector->GetDesc(&buffDesc);

		//CBuffer description:
		//uFlag for  D3D_SHADER_CBUFFER_FLAGS --- sth about packing
		//Type for D3D_CBUFFER_TYPE --- Constant, Texture

}
