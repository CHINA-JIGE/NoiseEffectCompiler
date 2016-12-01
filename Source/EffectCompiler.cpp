/***************************************************************

							NOISE EFFECT COMPILER

***********************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

IEffectCompiler::IEffectCompiler():
	mTargetFileName(""),
	mEffectFileName("")
{
}

bool IEffectCompiler::ParseCommandLine(int argc, char* argv[])
{
	if (!mFunction_LoadParameters(argc, argv))return false;
	if (!mFunction_LoadFiles())return false;

	return true;
}

bool IEffectCompiler::Compile()
{
	//---------------------------
	std::cout << "Effect parsing stage...." << std::endl;

	std::vector<N_SHADER_DESC> shaderDescList;
	std::vector<N_UNIQUE_SHADER> shaderList;
	std::vector<std::string> sourceFileList;

	if (!mFunction_ParseEffect(shaderDescList,sourceFileList))return false;
	
	std::cout << "Effect parsing stage completed." << std::endl;
	
	//---------------------------
	std::cout << "HSLS compilation stage ...." << std::endl;

	if (!mFunction_CompileHLSL(shaderDescList, shaderList,sourceFileList))return false;

	std::cout << "HSLS compilation stage Completed ...." << std::endl;

	//------------------------------
	std::cout << "Shader resource binding analysis Stage ...." << std::endl;

	if (!mFunction_ShaderReflection(shaderList))return false;

	std::cout << "Shader resource binding analysis stage Completed...." << std::endl;
	//-----------------------------
	
	if (!mFunction_FillShaderDescOfEffectHierarchy())return false;

	//-----------------------------
	std::cout << "Start  writing compiled effect file ...." << std::endl;

	if (!mFunction_OutputCompiledEffectFile())return false;

	std::cout << "Compiled file output completed." << std::endl;

	return true;
}

IEffect * IEffectCompiler::GetEffectInterface()
{
	return &mEffect;
}

/******************************************************

								 P R I V A T E

*********************************************************/

//parse user-input arguments
bool IEffectCompiler::mFunction_LoadParameters(int argc, char * argv[])
{

		//if User didnt use cmd to pass enough parameter into the program
		if (argc == 0 || argc == 1)
		{
			//then exit
			std::cout << "Compile Error!Not enough Parameters was passed in." << std::endl;
			return false;
		}

		//loop through all argv(some could be command, some could be param of specific command, some might be invalid)
		std::string currArgv = "";//the first argument is path of executing program
		int index = 1; //skip app path
		while (index<argc)
		{
			currArgv = argv[index];

			//trim the double quotation marks at the first and end.
			if (currArgv.at(0) == '\"')		currArgv.erase(currArgv.begin());
			if (currArgv.back() == '\"')	currArgv.pop_back();


			//-------------command: output compiled effect file
			if (currArgv == "/Out")
			{
				++index;

				if (index < argc)
				{
					mTargetFileName = argv[index];
					++index;
				}
				else
				{std::cout << "/Out command found with no parameters." << std::endl;}

				goto label_nextLoop;
			}

			//-------------command: input compile config file
			if (currArgv == "/EffectPath")
			{
				++index;

				if (index < argc)
				{
					mEffectFileName = argv[index];
					++index;
				}
				else
				{std::cout << "/EffectPath command found with no parameters." << std::endl;};

				goto label_nextLoop;
			}

			//--------------unrecognized string
			std::cout << "encounter unrecognized string : \'" << argv[index] <<"\'"<< std::endl;
			++index;

		label_nextLoop:;
		}

		if (mTargetFileName.size() != 0 &&
			mEffectFileName.size() != 0)
		{
			return true;
		}
		else
		{
			return false;
		}
}

bool IEffectCompiler::mFunction_LoadFiles()
{
	auto func_computeRelativePath = [](const std::string& wholePath,std::string& outRelativePath)
	{
		for (UINT i = wholePath.size() - 1;i >= 0;--i)
		{
			if (wholePath.at(i) == '\\' || wholePath.at(i) == '/')
			{
				outRelativePath = wholePath.substr(0, i);
				return;
			}
		}
	};

	auto func_LoadFile = [](const std::string& filePath,std::vector<unsigned char>& outDataBuffer)
	{
		//check file path
		std::fstream  sourceFile(filePath);
		if (!sourceFile.good())
		{
			//file not exist , exit
			std::cout << "Compile Error!file path not existed!";
			system("pause");
			return false;
		}

		//load source file
		sourceFile.seekg(0, std::ios::end);
		std::streamoff fileSize = sourceFile.tellg();
		sourceFile.seekg(0);
		outDataBuffer.resize(UINT(fileSize));
		sourceFile.read((char*)&outDataBuffer.at(0), fileSize);
		sourceFile.close();

		return true;
	};

	//load noise effect file 
	func_LoadFile(mEffectFileName, mEffectFileBuffer);

	func_computeRelativePath(mEffectFileName, mRelativePath);

	return true;
}

bool IEffectCompiler::mFunction_ParseEffect(std::vector<N_SHADER_DESC>& outShaderList, std::vector<std::string>& outSourceFileList)
{
	//tokenize

	if (!mTokenizer.Tokenize(mEffectFileBuffer)) { return false; }
	std::vector<N_TokenInfo> tokenList;
	mTokenizer.GetTokenList(tokenList);

	//parse token stream
	if (!mParser.Parse(std::move(tokenList), &mEffect)) { return false; }
	mParser.GetCompilationPlan(outShaderList);
	mParser.GetHLSLFileList(outSourceFileList);

	return true;
}

bool IEffectCompiler::mFunction_CompileHLSL(std::vector<N_SHADER_DESC>& in_uniqueShaderDescList, std::vector<N_UNIQUE_SHADER>& out_uniqueShaderList, const std::vector<std::string>& sourceFileList)
{

	//only reserve "#include" + sourceFile in this intermediate file for d3dcompile to compile hlsl
	//std::string intermediateUncompiledSource;

	std::string intermediateUncompiledSource("");
	for (auto& sf : sourceFileList)intermediateUncompiledSource += ("#include \"" + sf + "\"\n");

	//------------COMPILE HLSL---------------
	NoiseEffectCompiler::IShaderInclude shaderInclude(mRelativePath);
	std::vector<ID3DBlob*> errorMsgBlobList(in_uniqueShaderDescList.size());
	std::wstring tmpRelativePathW(mRelativePath.begin(), mRelativePath.end());


	for (auto& sdDesc: in_uniqueShaderDescList)
	{
		N_UNIQUE_SHADER sd;
		sd.entryPoint = sdDesc.entryPoint;
		sd.version = sdDesc.version;

		errorMsgBlobList.push_back(nullptr);

		std::cout << "Compile Shader--- EntryPoint:" << sd.entryPoint << ",targetVersion:" << sd.version << std::endl;

		//---------------------------D3D COMPILE (HLSL)---------------------
		//an serious problem occur!!! when a .sample exist in a "if-else" branch which is nested inside a "for" loop,
		//a catastrophic LOOP UNROLL will occur and thus led to a tremendous code size increasement
		//---> horrible compilation time increasement
		//---> ("if-else" branch with tex.sample ("gradient instruction")should be removed.
		//---> one-branch-one-shader strategy might be used. (then N switches variables will need 2^n shaders)

		//compile shaders binary from intermediate source code string
		//D3DCompileFromFile() in win8sdk can also be used
		HRESULT hr = D3DCompile(
			intermediateUncompiledSource.c_str(),
			intermediateUncompiledSource.size(),
			NULL,
			NULL,
			&shaderInclude,
			sd.entryPoint.c_str(),
			sd.version.c_str(),
			D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR,
			NULL,//we're not compiling an fx effect, set to null
			&sd.pBlob,//compiled binary code saved to corresponding shader
			&errorMsgBlobList.back()
		);

		//check if HLSL compilation failed
		if (FAILED(hr))
		{
			std::cout << "shader compilation failed!!" << std::endl;
			//retrieve Error message from ID3DBlobs
			if (errorMsgBlobList.back()->GetBufferPointer() != nullptr)
			{
				std::string errorStr((char*)errorMsgBlobList.back()->GetBufferPointer(), errorMsgBlobList.back()->GetBufferSize());
				std::cout << errorStr << std::endl << std::endl;
			}
			break;
		}
		else
		{
			std::cout << "shader compilation success!! code size :" << sd.pBlob->GetBufferSize()
				<< " bytes." << std::endl << std::endl;
		}

		out_uniqueShaderList.push_back(sd);
	}

	return true;
}

bool IEffectCompiler::mFunction_ShaderReflection(std::vector<N_UNIQUE_SHADER>& in_out_uniqueShaderList)
{
	//resource binding information has been completed in this "Reflect" function
	mShaderReflector.Reflect(in_out_uniqueShaderList);

	//build a unique shader hash table
	for (auto& s : in_out_uniqueShaderList)
	{
		mUniqueShaderTable[s.GetUID()] = s;
	}

	return true;
}

bool IEffectCompiler::mFunction_FillShaderDescOfEffectHierarchy()
{
	//up to now, all unique data block in global data pool are acquired, 
	//but they need to be bound to each pass/shader in the Effect Hierarchy by names.

	//traverse every pass
	UINT techCount = mEffect.GetObjectCount();
	
	for (UINT i = 0;i < techCount;++i)
	{
		ITechnique* pTech = mEffect.GetObjectPtr(i);
		UINT passCount = pTech->GetObjectCount();

		for (UINT j = 0;j < passCount; ++j)
		{
			IPass* pPass = pTech->GetObjectPtr(j);

			// entry/version information in an Effect has been generated by parser.
			// Now we have to fetch complete data from unique shader table and copy
			// to shaders in effect hierarchy
			const UINT maxShaderCountInPass = 3;
			N_SHADER_DESC* pShaderDesc[maxShaderCountInPass] = { pPass->GetVS(), pPass->GetGS(),pPass->GetPS() };

			for (UINT k = 0;k < maxShaderCountInPass;++k)
			{
				//current shader not in use, continue to next shader
				if (pShaderDesc[k] == nullptr)continue;

				//only copy NAME of data block to effect hierarchy
				auto pShader = mUniqueShaderTable.find(pShaderDesc[k]->GetUID());
				if (pShader != mUniqueShaderTable.end())
				{
					for (auto & cb : pShader->second.cbInfo)
						pShaderDesc[k]->cbNames.push_back(cb.name);

					for (auto & tb : pShader->second.tbInfo)
						pShaderDesc[k]->tbNames.push_back(tb.name);

					for (auto & tex : pShader->second.texInfo)
						pShaderDesc[k]->texNames.push_back(tex.name);

					for (auto & sp : pShader->second.cbInfo)
						pShaderDesc[k]->samplerNames.push_back(sp.name);
				}
			}
		}
	}

	return true;
}

bool IEffectCompiler::mFunction_OutputCompiledEffectFile()
{
	std::vector<N_UNIQUE_SHADER> uniqueShaderList;

	for (auto& e : mUniqueShaderTable)uniqueShaderList.push_back(e.second);

	mFileWriter.OutputBinaryToFile(
		mTargetFileName,
		uniqueShaderList,
		&mEffect);

	return true;
}
