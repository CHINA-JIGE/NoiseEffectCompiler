/***************************************************************

							IShaderInclude

				Inherited ID3DInclude to override functions
				this class is used in D3DCompile

***********************************************************/
#include "EffectCompiler.h" 

using namespace NoiseEffectCompiler;

IShaderInclude::IShaderInclude()
{
	mSystemDir.resize(MAX_PATH + 1);
	GetSystemDirectoryA(&mSystemDir.at(0), mSystemDir.size());
}

HRESULT IShaderInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pRelativeFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes)
{
	try {
		std::string finalPath;
		switch (IncludeType)
		{
		case D3D_INCLUDE_LOCAL:
			finalPath =pRelativeFileName;//relative dir to
			break;
		case D3D_INCLUDE_SYSTEM:
			finalPath = mSystemDir + "\"" + pRelativeFileName;
			break;
		default:
			std::cout << "IShaderInclude : IncludeType Error" << std::endl;
			break;
		}


		std::ifstream includeFile(finalPath.c_str(), std::ios::in | std::ios::binary | std::ios::ate);


		if (includeFile.is_open()) {
			long long fileSize = includeFile.tellg();
			char* buf = new char[fileSize];
			includeFile.seekg(0, std::ios::beg);
			includeFile.read(buf, fileSize);
			includeFile.close();
			*ppData = buf;
			*pBytes = fileSize;
		}
		else {
			return E_FAIL;
		}
		return S_OK;
	}
	catch (std::exception& e) {
		return E_FAIL;
	}
}

HRESULT IShaderInclude::Close(LPCVOID pData)
{
	char* buf = (char*)pData;
	delete[] buf;
	return S_OK;
}
