/***************************************************************

							IEffect Parser

				Parse passName/EntryPoint/Technique name etc.
				information from noiseEffect file

***********************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;

/***********************EFFECT   PARSER*****************************/


IEffectParser::IEffectParser()
	:mTokenIndex(0)
{

}

bool  IEffectParser::Parse( std::vector<N_TokenInfo>&& tokenList,IEffectDesc* pEffect)
{
	mTokenList = tokenList;
	mTokenIndex = 0;
	m_pEffect = pEffect;

	while (mTokenIndex < tokenList.size())
	{
		const N_TokenInfo& currToken = tokenList.at(mTokenIndex);

		switch (currToken.type)
		{
			//------Annotation-----
		case TK_ANNOTATION:
			break;

			//------Preprocess Instruction-----
		case TK_PREPROCESS:
		{
			if (currToken.content == "include")
			{
				if (!mFunction_ParseIncludeInstruction())return false;
			}
			break;
		}

		case TK_DELIMITER:
			mFunction_ReportError("unexpected delimiter '" + currToken.content + "'");
			return false;
			break;

		case TK_IDENTIFIER:
			mFunction_ReportError("unexpected identifier '" + currToken.content + "'");
			return false;
			break;

		case TK_LITERAL_STR:
			mFunction_ReportError("unexpected literal string '" + currToken.content + "'");
			return false;
			break;

		case TK_NUMBER:
			mFunction_ReportError("unexpected number '" + currToken.content + "'");
			return false;
			break;

		case TK_KEYWORD:
			// ID-{ -BODY-} pattern matching leaves ParseTechnique to manage
			//KEYWORD only indicates a sub-parsing procedure starts
			if (currToken.content == "Technique") 
			{
				if (!mFunction_ParseTechnique())return false;
			}
			break;
		}

		++mTokenIndex;
	}
	return true;
}

void IEffectParser::GetHLSLFileList(std::vector<std::string>& outFileList)
{
	outFileList = mSourceFileList;
}

void IEffectParser::GetCompilationPlan(std::vector<N_SHADER_DESC>& outShaderList)
{
	for (auto s : mUniqueShaderTable)
	{
		outShaderList.push_back(s.second);
	}
}


/*********************** P R I V A T E *****************************/

bool IEffectParser::mFunction_ParseTechnique()
{
	//new technique
	ITechniqueDesc* pTech = nullptr;

	//****************PATTERN*************************
	//	[Identifier - Name] [Delimiter, '{'] [Keyword, "Pass" ][Keyword, "Pass" ]...[Keyword, "Pass" ] [Delimiter, '}' ]

	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_IDENTIFIER))
	{
		//create ITechnique with NAME (identifier)
		pTech = m_pEffect->CreateObject(mTokenList.at(mTokenIndex).content);
	}
	else
	{
		mFunction_ReportError("Technique Name missing !!");
		return false;//un-recoverable error
	}

	++mTokenIndex;
	//-------- { L-Brace---------
	if (!mFunction_MatchCurrentToken(TK_DELIMITER, "{"))
	{
		mFunction_ReportError("mismatching brace, Left Brace '{' missing!");
		return false;//un-recoverable error
	}


	while (1)
	{
		++mTokenIndex;

		//all tokens had been traversed but R-Brace NOT MATCH (still in loop)
		if (mTokenIndex >= mTokenList.size())
		{
			mFunction_ReportError("mismatching Brace, Right Brace '}' missing");
			return false;
		}

		//Try to match PASS indicator keyword
		if (mFunction_MatchCurrentToken(TK_KEYWORD, "Pass"))
		{
			//call Pass parsing sub-procedure
			if (!mFunction_ParsePass(pTech))return false;
		}
		else if (mFunction_MatchCurrentToken(TK_DELIMITER, "}"))
		{
			//end of TECHNIQUE
			return true;
		}
		else
		{
			mFunction_ReportError("Illegal token '" + mTokenList.at(mTokenIndex).content
				+ "'Found within a Technique block. A 'Pass' block is expected to be here.");
			return false;
		}
	}

	return true;
}

bool IEffectParser::mFunction_ParsePass(ITechniqueDesc* pFatherTechnique)
{
	//new pass
	IPassDesc* pPass;

	//****************PATTERN*************************
	//	[Identifier - Name] [Delimiter, '{'] [Keyword, "VS" ][Keyword, "PS" ]...[Keyword, "Pass" ] [Delimiter, '}' ]

	//----Name----
	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_IDENTIFIER))
	{
		pPass = pFatherTechnique->CreateObject(mTokenList.at(mTokenIndex).content);
	}
	else
	{
		mFunction_ReportError("Pass Name missing !!");
		return false;//un-recoverable error
	}


	//--- {  L-Brace---
	++mTokenIndex;
	if (!mFunction_MatchCurrentToken(TK_DELIMITER, "{"))
	{
		mFunction_ReportError("mismatching brace, Left Brace '{' missing!");
		return false;//un-recoverable error
	}


	while (1)
	{
		++mTokenIndex;

		//all tokens had been traversed but R-Brace NOT MATCH (still in loop)
		if (mTokenIndex >= mTokenList.size())
		{
			mFunction_ReportError("mismatching Brace, Right Brace '}' missing");
			return false;
		}

		//try match end of technique 
		if (mFunction_MatchCurrentToken(TK_DELIMITER, "}"))
		{
			//end of PASS
			return true;
		}
		else if (mFunction_MatchCurrentToken(TK_KEYWORD, "VS"))
		{
			//call shader-setting function parsing sub-procedure
			if(!mFunction_ParseShaderConfig(pPass,NOISE_SHADER_TYPE_VS))return false;
		}
		else if (mFunction_MatchCurrentToken(TK_KEYWORD, "PS"))
		{
			//call shader-setting function parsing sub-procedure
			if (!mFunction_ParseShaderConfig(pPass, NOISE_SHADER_TYPE_PS))return false;
		}
		else if (mFunction_MatchCurrentToken(TK_KEYWORD, "GS"))
		{
			if (!mFunction_ParseShaderConfig(pPass, NOISE_SHADER_TYPE_GS))return false;
		}
		else if (mFunction_MatchCurrentToken(TK_DELIMITER, ";"))
		{
			//skip
		}
		else
		{
			mFunction_ReportError("Illegal token '" + mTokenList.at(mTokenIndex).content
				+ "' Found within a Technique block. A 'Pass' block is expected to be here.");
			return false;
		}
	}

	return true;
}

bool IEffectParser::mFunction_ParseShaderConfig(IPassDesc* pFatherPass, NOISE_SHADER_TYPE st)
{
	//new shader
	N_SHADER_DESC shader;

	//****************PATTERN*************************
	//	[Delimiter - '(' ] [Identifier, entryPoint ] [ Delimiter, ',' ] [KEYWORD, shaderVersion ]	[Delimiter - ')' ] 

	//------ ( -----
	++mTokenIndex;
	if (!mFunction_MatchCurrentToken(TK_DELIMITER, "("))
	{
		mFunction_ReportError("left parenthese '(' missing, failed to set VertexShader!");
		return false;
	}

	//-----entryPoint----
	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_IDENTIFIER))
	{
		shader.entryPoint = mTokenList.at(mTokenIndex).content;
	}
	else
	{
		mFunction_ReportError("Shader entry point missing, failed to set VertexShader!");
		return false;
	}

	//----- Comma ,  ----
	++mTokenIndex;
	if (!mFunction_MatchCurrentToken(TK_DELIMITER, ","))
	{
		mFunction_ReportError("Comma delimiter ',' missing, failed to set VertexShader!");
		return false;
	}

	//---shader version---
	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_KEYWORD))
	{
		shader.version = mTokenList.at(mTokenIndex).content;
	}
	else
	{
		mFunction_ReportError("Shader entry point error (missing or illegal)?, failed to set VertexShader!");
		return false;
	}

	//--- ) ---
	++mTokenIndex;
	if (!mFunction_MatchCurrentToken(TK_DELIMITER, ")"))
	{
		mFunction_ReportError("right parenthese ')' missing, failed to set VertexShader!");
		return false;
	}

	//--- SEMICOLON ; ---
	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_DELIMITER, ";"))
	{
		//Set parsed shader 
		switch (st)
		{
		case NOISE_SHADER_TYPE_VS:
			pFatherPass->SetVS(shader);
			break;

		case  NOISE_SHADER_TYPE_PS:
			pFatherPass->SetPS(shader);
			break;

		case NOISE_SHADER_TYPE_GS:
			pFatherPass->SetGS(shader);
			break;
		}

		//add to need compiling list (unique shader list)
		if (mUniqueShaderTable.find(shader.GetUID()) == mUniqueShaderTable.end())
		{
			mUniqueShaderTable.insert(std::make_pair(shader.GetUID(),shader));
		}

		return true;
	}
	else
	{
		mFunction_ReportError("semicolon ';' missing after a shader setting statement, !!!");
		return false;
	}
}

bool IEffectParser::mFunction_ParseIncludeInstruction()
{
	++mTokenIndex;
	if (mFunction_MatchCurrentToken(TK_LITERAL_STR))
	{
		mSourceFileList.push_back(mTokenList.at(mTokenIndex).content);
		return true;
	}
	else
	{
		mFunction_ReportError("unrecognized token after '#include' instruction");
		return false;
	}
}

bool IEffectParser::mFunction_MatchCurrentToken(N_TOKEN_TYPE type)
{
	const auto& token = mTokenList.at(mTokenIndex);

	if (token.type == type)
		return true;
	else 
		return false;
}

bool IEffectParser::mFunction_MatchCurrentToken(N_TOKEN_TYPE type, const std::string & content)
{
	const auto& token = mTokenList.at(mTokenIndex);
	if (token.type == type && token.content == content)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void IEffectParser::mFunction_ReportError(const std::string & msg)
{
	std::cout<<"LINE :"<< mTokenList.at(mTokenIndex).line <<std::endl
		<<"---Syntax Error :"<< msg << std::endl;
}

