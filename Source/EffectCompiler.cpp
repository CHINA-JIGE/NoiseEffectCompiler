/***************************************************************

							NOISE EFFECT COMPILER

***********************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

IEffectCompiler::IEffectCompiler():
	pSR(nullptr),
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
	
	NoiseEffectCompiler::IShaderInclude shaderInclude;
	ID3DBlob* compiledCodeBlob;
	ID3DBlob* errorMsgBlob;

	std::vector<N_Shader> shaderList;
	std::vector<std::string> sourceFileList;

	if (!mFunction_ParseEffect(shaderList,sourceFileList))return false;
	
	//compile shaders binary from files
	/*HRESULT hr = D3DCompile(
		mEffectFileName.c_str(),
		mEffectFileName.size(),
		NULL,
		NULL,
		&shaderInclude,
		entryPointNeedToBeParsed,
		shaderTargetVersionNeedToBeParsed,
		D3DCOMPILE_OPTIMIZATION_LEVEL3 | D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR, //D3DCompile_
		NULL,//we're not compiling an effect, set to null
		&compiledCodeBlob,
		&errorMsgBlob
	);*/

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
			system("pause");
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
		outDataBuffer.resize(fileSize);
		sourceFile.read((char*)&outDataBuffer.at(0), fileSize);
		sourceFile.close();

		return true;
	};

	//load source file
	//func_LoadFile(mSourceFileName, mSourceCodeFileBuffer);

	//load noise effect file 
	func_LoadFile(mEffectFileName, mEffectFileBuffer);

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
