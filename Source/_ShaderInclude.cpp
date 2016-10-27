/***************************************************************

							IShaderInclude

				Inherited ID3DInclude to override functions
				this class is used in D3DCompile

***********************************************************/
#include "EffectCompiler.h" 

using namespace NoiseEffectCompiler;

IShaderInclude::IShaderInclude(const std::string& relativePath)
{
	mSystemDir.resize(MAX_PATH + 1);
	mRelativePath = relativePath;
	GetSystemDirectoryA(&mSystemDir.at(0), mSystemDir.size());
}

HRESULT IShaderInclude::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pRelativeFileName, LPCVOID pParentData, LPCVOID * ppData, UINT * pBytes)
{
	try {
		std::string finalPath;
		switch (IncludeType)
		{
		case D3D_INCLUDE_LOCAL:
			finalPath = mRelativePath+pRelativeFileName;//relative dir to
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
			UINT fileSize = 0;
			long long LLFileSize = includeFile.tellg();

			//UINT can't overflow
			if (LLFileSize <= UINT_MAX)
			{
				fileSize = UINT(LLFileSize);
			}
			else
			{
				throw std::exception("File Error : Open hlsl Include file failed!! file is too large!");
			}
			char* buf = new char[fileSize];
			includeFile.seekg(0, std::ios::beg);
			includeFile.read(buf, fileSize);
			includeFile.close();
			*ppData = buf;
			*pBytes = fileSize;
		}
		else 
		{
			return E_FAIL;
		}
		return S_OK;
	}
	catch (std::exception& e) {
		ERROR_MSG(e.what());
		return E_FAIL;
	}
}

HRESULT IShaderInclude::Close(LPCVOID pData)
{
	char* buf = (char*)pData;
	delete[] buf;
	return S_OK;
}
