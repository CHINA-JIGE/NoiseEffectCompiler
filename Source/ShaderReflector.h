/*********************************************************************

							SHADER REFLECTOR

			Use D3DReflect() to get shader resource binding 
			information. The Main Objective is to get slot-resource
			mapping relations to such that NoiseEffect Framework
			can easily update resource by means of setting slots

*********************************************************************/

#pragma once

namespace NoiseEffectCompiler
{


	//---shader reflector
	class IShaderReflector
	{
	public:

		IShaderReflector();

		bool Reflect(std::vector<N_UNIQUE_SHADER>& in_out_UniqueShaders);

		/*const std::vector<N_SHADER_CBUFFERS>* GetShaderCbuffersList();

		const std::vector<N_SHADER_TBUFFERS>* GetShaderTBuffersList();

		const std::vector<N_SHADER_TEX_RESOURCES>* GetShaderTexResourcesList();

		const std::vector<N_SHADER_SAMPLERS> * GetShaderSamplersList();*/

	private:


		bool mFunction_ReflectShader(ID3DBlob* compiledShaderCode, N_UNIQUE_SHADER& in_out_Shader);

		bool mFunction_ReflectResources(ID3D11ShaderReflection* pShaderReflector, UINT resCount, N_UNIQUE_SHADER& in_out_Shader);

		//bool mFunction_ReflectConstantBuffer(ID3D11ShaderReflection* pShaderReflector, UINT cbCount);

		/*std::vector<N_SHADER_CBUFFERS> mShaderCBuffersList;

		std::vector<N_SHADER_TBUFFERS> mShaderTBuffersList;

		std::vector<N_SHADER_TEX_RESOURCES> mShaderTexResourcesList;

		std::vector<N_SHADER_SAMPLERS> mShaderSamplersList;*/

	};
};