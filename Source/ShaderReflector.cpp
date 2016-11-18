/*********************************************************************

							SHADER REFLECTOR

*********************************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

IShaderReflector::IShaderReflector()
{
}

bool IShaderReflector::Reflect(std::vector<N_UNIQUE_SHADER>& in_out_UniqueShaders)
{

	for (N_UNIQUE_SHADER& sd: in_out_UniqueShaders)
	{
		//------------------------FOR EVERY SHADER------------------------ 
		
		//get compiled code
		ID3DBlob* pBlob = sd.pBlob;

		if (!mFunction_ReflectShader(pBlob,sd))return false;
	}

	return true;
}

/*const std::vector<N_SHADER_CBUFFERS>* NoiseEffectCompiler::IShaderReflector::GetShaderCbuffersList()
{
	return &mShaderCBuffersList;
}

const std::vector<N_SHADER_TBUFFERS>* NoiseEffectCompiler::IShaderReflector::GetShaderTBuffersList()
{
	return &mShaderTBuffersList;
}

const std::vector<N_SHADER_TEX_RESOURCES>* NoiseEffectCompiler::IShaderReflector::GetShaderTexResourcesList()
{
	return &mShaderTexResourcesList;
}

const std::vector<N_SHADER_SAMPLERS>* NoiseEffectCompiler::IShaderReflector::GetShaderSamplersList()
{
	return &mShaderSamplersList;
}*/

/************************************************************

								PRIVATE

************************************************************/

bool IShaderReflector::mFunction_ReflectShader(ID3DBlob * compiledShaderCode, N_UNIQUE_SHADER& in_out_Shader)
{
	//Reflect -- more detail please refer to MSDN
	ID3D11ShaderReflection* pReflector = nullptr;
	
	//DX IID_ must link "dxguid.lib"
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

	//shaderIndex means the ith shaders in compilation plan
	if (!mFunction_ReflectResources(pReflector, shaderDesc.BoundResources, in_out_Shader))return false;

	return true;
}

bool IShaderReflector::mFunction_ReflectResources(ID3D11ShaderReflection * pShaderReflector, UINT resCount ,N_UNIQUE_SHADER& in_out_Shader)
{
	for (UINT i = 0;i < resCount;i++)
	{
		//shader input binding (CBUFFER,TBUFFER,single TEXTURE,Samplers, etc. ,even UAV... anything that can be accessed
		//within this shader will be regarded as shader input
		D3D11_SHADER_INPUT_BIND_DESC bindDesc;

		pShaderReflector->GetResourceBindingDesc(i, &bindDesc);

		//add binding info according to input type
		switch (bindDesc.Type)
		{

		case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
		{
			//get cb size info
			ID3D11ShaderReflectionConstantBuffer* pCbReflector = pShaderReflector->GetConstantBufferByName(bindDesc.Name);
			D3D11_SHADER_BUFFER_DESC tmpDesc;
			pCbReflector->GetDesc(&tmpDesc);

			//add to shader info list
			N_CBUFFER_INFO info;
			info.bindCount = bindDesc.BindCount;
			info.bindSlot = bindDesc.BindPoint;
			info.name = bindDesc.Name;
			info.byteSize = tmpDesc.Size;
			in_out_Shader.cbInfo.push_back(info);
			break;
		}

		case D3D_SHADER_INPUT_TYPE::D3D_SIT_TBUFFER:
		{
			//get tb size info (will this work???)
			ID3D11ShaderReflectionConstantBuffer* pCbReflector = pShaderReflector->GetConstantBufferByName(bindDesc.Name);
			D3D11_SHADER_BUFFER_DESC tmpDesc;
			pCbReflector->GetDesc(&tmpDesc);

			//add to shader info list
			N_TBUFFER_INFO info;
			info.bindCount = bindDesc.BindCount;
			info.bindSlot = bindDesc.BindPoint;
			info.name = bindDesc.Name;
			info.byteSize = tmpDesc.Size;
			in_out_Shader.tbInfo.push_back(info);
			break;
		}

		case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
		{
			N_TEXTURE_RESOURCE_INFO info;
			info.bindCount = bindDesc.BindCount;
			info.bindSlot = bindDesc.BindPoint;
			info.name = bindDesc.Name;
			info.dimension = bindDesc.Dimension;
			info.numSamples = bindDesc.NumSamples;
			info.returnType = bindDesc.ReturnType;
			in_out_Shader.texInfo.push_back(info);
			break;
		}

		case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:
		{
			N_SAMPLER_INFO info;
			info.bindCount = bindDesc.BindCount;
			info.bindSlot = bindDesc.BindPoint;
			info.name = bindDesc.Name;
			in_out_Shader.samplerInfo.push_back(info);
			break;
		}

		default:
			std::cout << "Unsupported shader input type : ";
			switch (bindDesc.Type)
			{
			case D3D_SIT_BYTEADDRESS:
				std::cout << "ByteAdress Buffer" << std::endl;break;
			case D3D_SIT_STRUCTURED:
				std::cout << "Structured Buffer" << std::endl;break;
			case D3D_SIT_UAV_APPEND_STRUCTURED:
			case D3D_SIT_UAV_CONSUME_STRUCTURED:
			case D3D_SIT_UAV_RWBYTEADDRESS:
			case D3D_SIT_UAV_RWSTRUCTURED:
			case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER:
			case D3D_SIT_UAV_RWTYPED:
				std::cout << "UAV" << std::endl;break;
			}


		}
	}

	return true;
}
	
/*bool IShaderReflector::mFunction_ReflectConstantBuffer(ID3D11ShaderReflection * pShaderReflector, UINT cbCount)
{
	std::vector<ID3D11ShaderReflectionConstantBuffer*> cbReflectorList;


	//reflect constant buffer/texture buffer (both are called by a joint name "constantBuffer")
	for (UINT i = 0;i < cbCount;i++)
	{
		ID3D11ShaderReflectionConstantBuffer* pCbReflector = pShaderReflector->GetConstantBufferByIndex(i);
		if (pCbReflector == nullptr)return false;

		cbReflectorList.push_back(pCbReflector);

		//get constant buffer description
		D3D11_SHADER_BUFFER_DESC buffDesc;
		pCbReflector->GetDesc(&buffDesc);

		ID3D11ShaderReflectionVariable* pVar = pCbReflector->GetVariableByIndex(0);
		ID3D11ShaderReflectionType* pType = pVar->GetType();
		//CBuffer description:
		//uFlag for  D3D_SHADER_CBUFFER_FLAGS --- sth about packing (0 for default packing, 1 for user-defined packing)
		//Type for D3D_CBUFFER_TYPE --- Constant buffer/ Texture Buffer 
		
	}

}*/
