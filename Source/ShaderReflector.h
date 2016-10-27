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
	class IShaderReflector
	{
	public:

		IShaderReflector();

		bool Analyze(const std::vector<N_Shader>& shaderInfoList, const std::vector<ID3DBlob*>& compiledShaderCode);

	private:

		bool mFunction_ReflectShader(ID3DBlob* compiledShaderCode);

		bool mFunction_ReflectConstantBuffer(ID3D11ShaderReflection* pReflector,UINT cbCount);

	};
};