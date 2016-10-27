

#pragma once

//used in D3DCompile function
namespace NoiseEffectCompiler
{

	class IShaderInclude : public ID3DInclude {
	public:
		IShaderInclude(const std::string& relativePath);

		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes);
		HRESULT __stdcall Close(LPCVOID pData);

	private:
		std::string mSystemDir;
		std::string mRelativePath;
	};
};