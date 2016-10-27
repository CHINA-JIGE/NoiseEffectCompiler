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
	std::cout << "Effect Parsing Stage...." << std::endl;

	std::vector<N_Shader> shaderList;
	std::vector<std::string> sourceFileList;

	if (!mFunction_ParseEffect(shaderList,sourceFileList))return false;
	
	std::cout << "Effect Parsing Stage Completed." << std::endl;
	
	//---------------------------
	std::vector<ID3DBlob*> compiledCodeBlobList(shaderList.size());
	if (!mFunction_CompileHLSL(shaderList, sourceFileList, compiledCodeBlobList))return false;

	std::cout << "HSLS Compilation Stage Completed." << std::endl;

	//------------------------------
	//D3D Reflection
	if (!mFunction_ShaderReflection(compiledCodeBlobList))return false;

	return true;
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

bool IEffectCompiler::mFunction_ParseEffect(std::vector<N_Shader>& outShaderList, std::vector<std::string>& outSourceFileList)
{
	//tokenize
	NoiseEffectCompiler::IEffectTokenizer tokenizer;
	if (!tokenizer.Tokenize(mEffectFileBuffer)) { return false; }
	std::vector<N_TokenInfo> tokenList;
	tokenizer.GetTokenList(tokenList);

	//parse token stream
	NoiseEffectCompiler:: IEffectParser parser;
	if (!parser.Parse(std::move(tokenList))) { return false; }
	parser.GetCompilationPlan(outShaderList);
	parser.GetHLSLFileList(outSourceFileList);

	return true;
}

bool IEffectCompiler::mFunction_CompileHLSL(const std::vector<N_Shader>& shaderList, const std::vector<std::string>& sourceFileList, std::vector<ID3DBlob*>& outCompiledShaderCode)
{

	//only reserve "#include" + sourceFile in this intermediate file for d3dcompile to compile hlsl
	//std::string intermediateUncompiledSource;
	std::cout << "HSLS Compilation Stage...." << std::endl;

	std::string intermediateUncompiledSource("");
	for (auto& sf : sourceFileList)intermediateUncompiledSource += ("#include \"" + sf + "\"\n");

	//------------COMPILE HLSL---------------
	NoiseEffectCompiler::IShaderInclude shaderInclude(mRelativePath);
	std::vector<ID3DBlob*> errorMsgBlobList(shaderList.size());
	std::wstring tmpRelativePathW(mRelativePath.begin(), mRelativePath.end());

	for (UINT i = 0;i<shaderList.size();i++)
	{
		auto & sd = shaderList.at(i);

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
			NULL,//we're not compiling an effect, set to null
			&outCompiledShaderCode.at(i),
			&errorMsgBlobList.at(i)
		);

		//check if HLSL compilation failed
		if (FAILED(hr))
		{
			std::cout << "shader compilation failed!!" << std::endl;
			//retrieve Error message from ID3DBlobs
			if (errorMsgBlobList.at(i)->GetBufferPointer() != 0)
			{
				std::string errorStr((char*)errorMsgBlobList.at(i)->GetBufferPointer(), errorMsgBlobList.at(i)->GetBufferSize());
				std::cout << errorStr << std::endl << std::endl;
			}
			break;
		}
		else
		{
			std::cout << "shader compilation success!! code size :" << outCompiledShaderCode.at(i)->GetBufferSize()
				<< " bytes." << std::endl << std::endl;
		}
	}

	return true;
}

bool IEffectCompiler::mFunction_ShaderReflection(const std::vector<N_Shader>& shaderInfoList, const std::vector<ID3DBlob*>& compiledShaderCode)
{
	IShaderReflector analyzer;
	analyzer.Analyze(compiledShaderCode);
	return true;
}

bool IEffectCompiler::mFunction_OutputCompiledEffectFile()
{
	return true;
}
