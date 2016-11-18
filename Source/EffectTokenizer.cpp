/***************************************************************

							EFFECT TOKENIZER

***********************************************************/

//there is something should be noted ,that keyword is categorized into 'identifier'

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;


NoiseEffectCompiler::IEffectTokenizer::IEffectTokenizer()
	: mLexerState(LS_NORMAL),
	mKeywordList({
	"Technique","Pass",
	"VS","PS","GS",
	"vs_5_0","ps_5_0","gs_5_0"})
{

}

NoiseEffectCompiler::IEffectTokenizer::~IEffectTokenizer()
{
	mLexerState = LS_NORMAL;
	mTokenList.clear();
}

bool NoiseEffectCompiler::IEffectTokenizer::Tokenize(const std::vector<unsigned char>& effectFileBuffer)
{
	//to match tokens, Regular Expression or State Machine schemes can be used
	//so now I decided to use State Machine (refer to doc for state transition diagram )
	//to generate tokens. SEE file://
	//(2016.10.3)actually I can't really understand NFA->DFA conversion now... thus I could only
	//managed to borrow this state machine idea to implement my own algorithm
	mTokenList.clear();
	mAnnotationList.clear();
	mLexerState = LS_NORMAL;
	mLineNumberInSource = 1;

	//start iteration
	bool isSucceeded = true;
	UINT totalByteSize = effectFileBuffer.size();
	for (UINT currPos = 0;currPos < totalByteSize;currPos++)
	{
		mFunction_LexerStateTransition(effectFileBuffer,currPos);
		//once failed, isSucceeded will be marked false until the end
		isSucceeded &= mFunction_LexerStateMachineOutput(effectFileBuffer, currPos);
	};

	if (isSucceeded)
	{
		//all 'word's are recognized as identifier at the very beginning, now convert qualified word into 'keyword'
		//only modified "type" field
		mFunction_FindKeywordsInIdentifier();
		return true;
	}
	else
	{
		return false; 
	}
}

void NoiseEffectCompiler::IEffectTokenizer::GetTokenList(std::vector<N_TokenInfo>& outTokenList)
{
	outTokenList =mTokenList;
}

void NoiseEffectCompiler::IEffectTokenizer::GetAnnotationList(std::vector<N_TokenInfo>& outTokenList)
{
	outTokenList = mAnnotationList;
}

void NoiseEffectCompiler::IEffectTokenizer::mFunction_LexerStateTransition(const std::vector<unsigned char>& effectFileBuffer,UINT filePos)
{
	unsigned char c = effectFileBuffer.at(filePos);//current char
	unsigned char peek = 0;
	if(filePos < effectFileBuffer.size()-1) peek = effectFileBuffer.at(filePos + 1);
	static std::string errorMsg = "";

	//trace current LINE number in source file to present more error message
	if (c == '\n') { ++mLineNumberInSource; }

	//encounter file end
	if (c == '\0') { mLexerState = LS_NORMAL;return; }

	//encounter unrecognized char (might be Unicode?)
	if (isascii(c) == 0) 
	{
		mLexerState = LS_ERROR;
		errorMsg = "Unrecognized Character '";
		errorMsg.push_back(c);errorMsg += " ' Found! (Non-Ascii char?)";
		return;
	}

	//some common state transitions which interrupt  many token generation state
	auto func_commonTransition=[&]()->bool
	{
		if (isCharDelimiter(c)) { mLexerState = LS_DELIM_NEWTOKEN; return true; }
		else if (c == '/') { mLexerState = LS_SLASH; return true;}
		else if (isCharSpaceTabNextline(c)) { mLexerState = LS_NORMAL;return true;}
		//true for transition occur
		return false;
	};

	switch (mLexerState)
	{
	case	LS_NORMAL:
		if (func_commonTransition()){}		//true for transition occur
		else if (c == '#') { mLexerState = LS_PREPROCESS_NEWTOKEN; }
		else if (c == '\"') { mLexerState = LS_LITERAL_STRING_NEWTOKEN; }// for the start of a literal string
		else if (isCharLetterUnderline(c)) { mLexerState = LS_IDENTIFIER_NEWTOKEN; }//identifier begin with _ or letter
		else if (isCharDigit(c)) { mLexerState = LS_NUMBER_NEWTOKEN; }
		else { mLexerState = LS_ERROR; errorMsg = "Unrecognized Character Found!"; }
		break;

	case LS_SLASH:
		if (c == '/') { mLexerState = LS_ANNOTATION_SINGLE_NEWTOKEN; }// double slashes now//
		else if (c == '*') { mLexerState = LS_ANNOTATION_MUL_NEWTOKEN; }// slash-star /*
		else { mLexerState = LS_ERROR; errorMsg = "Unexpected character after '/' !"; }
		break;

	// annotation entry <//>
	case LS_ANNOTATION_SINGLE_NEWTOKEN:
		if (c == '\n') { mLexerState = LS_NORMAL; }
		else { mLexerState = LS_ANNOTATION_SINGLE_BODY; }//after create anno token, read the body
		break;

	//  annotation body <//>
	case LS_ANNOTATION_SINGLE_BODY:
		if (c == '\n') { mLexerState = LS_NORMAL; }
		break;

	case LS_ANNOTATION_MUL_NEWTOKEN:
		if (c == '*' && peek == '/') { mLexerState = LS_ANNOTATION_MUL_ENDSTAR; }
		else { mLexerState = LS_ANNOTATION_MUL_BODY; }
		break;//start of /* annotation

	case LS_ANNOTATION_MUL_BODY:
		if (c == '*' && peek == '/') { mLexerState = LS_ANNOTATION_MUL_ENDSTAR; }
		break;

	case LS_ANNOTATION_MUL_ENDSTAR:
		mLexerState = LS_ANNOTATION_MUL_ENDSLASH;// */ must occur together to be meaningful, thus peek is used
		break;

	case LS_ANNOTATION_MUL_ENDSLASH:
		mLexerState = LS_NORMAL;
		break;

	case LS_PREPROCESS_NEWTOKEN:
		if (func_commonTransition()) {}	//true for transition occur
		else { mLexerState = LS_PREPROCESS_INST_BODY; }
		break;

	case LS_PREPROCESS_INST_BODY://instruction body
		if (isCharLetter(c)){}
		else if (func_commonTransition()) {}
		else if (c == '/') { mLexerState = LS_SLASH; }
		else { mLexerState = LS_ERROR; errorMsg = "Unexpected Character in preprocessor instruction occur!"; }
		break;

	case LS_IDENTIFIER_NEWTOKEN:
		if (func_commonTransition()) {}	//true for transition occur
		else { mLexerState = LS_IDENTIFIER_BODY; }
		break;

	case LS_IDENTIFIER_BODY:
		if(isCharLetterDigitUnderline(c)){}//append char to token name
		else 	if (func_commonTransition()) {}	//true for transition occur
		else { mLexerState = LS_ERROR;errorMsg = "Unexpected character in an identifier!"; }
		break;

		//DELIMITER  ; { } ( ) ,  \n  , 
	case LS_DELIM_NEWTOKEN:
		//the 'if' prevents the situation that several delimiters occurs one by one
		if (isCharLetterUnderline(c)) { mLexerState = LS_IDENTIFIER_NEWTOKEN; }
		else { func_commonTransition(); };	//true for transition occur
		break;

	//LITERAL STRING ""
	case LS_LITERAL_STRING_NEWTOKEN:
		if (c == '\"') { mLexerState = LS_NORMAL; }
		else { mLexerState = LS_LITERAL_STRING_BODY; }
		break;

	case LS_LITERAL_STRING_BODY:
		if (c == '\\') { mLexerState = LS_LITERAL_STRING_ESCAPED; }
		else if (c == '\"') {mLexerState = LS_NORMAL;}//end string
		break;

	case LS_LITERAL_STRING_ESCAPED:
		mLexerState = LS_LITERAL_STRING_BODY;
		break;

	case LS_NUMBER_NEWTOKEN:
		if(func_commonTransition()) {}	//true for transition occur)
		else { mLexerState = LS_NUMBER_BODY; }
		break;

	case LS_NUMBER_BODY:
		if(isCharDigit(c) || c=='.' ){}//number body (only integer??)
		else 	if (func_commonTransition()) {}	//true for transition occur
		else { mLexerState = LS_ERROR;errorMsg =  "Unexpected Character in a number!"; }
		break;

	case LS_ERROR:
		std::cout << "LINE:" << mLineNumberInSource <<std::endl<<
			"---Tokenizer Error : " << errorMsg <<std::endl<< std::endl;
		mLexerState = LS_NORMAL;//recover from error
		break;

	default:
		std::cout << "OMG...Some unexpected problem occur in Tokenizer= =. Please Report to developer.";
		break;
	}

}

bool NoiseEffectCompiler::IEffectTokenizer::mFunction_LexerStateMachineOutput(const std::vector<unsigned char>& effectFileBuffer, UINT filePos)
{
	switch (mLexerState)
	{
	case	LS_NORMAL:
		//do nothing
		break;

	case LS_SLASH:
		//do nothing
		break;

	// annotation entry <//>
	case LS_ANNOTATION_SINGLE_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 0;
		token.content = "";
		token.type = TK_ANNOTATION;
		token.line = mLineNumberInSource;
		mAnnotationList.push_back(token);
		break;
	}

	//  annotation body <//>
	case LS_ANNOTATION_SINGLE_BODY:
		mAnnotationList.back().content.push_back(effectFileBuffer.at(filePos));
		mAnnotationList.back().byteSize++;
		break;

	case LS_ANNOTATION_MUL_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 0;
		token.content = "";
		token.type = TK_ANNOTATION;
		token.line = mLineNumberInSource;
		mAnnotationList.push_back(token);
		break;
	}

	case LS_ANNOTATION_MUL_BODY:
		mAnnotationList.back().content.push_back(effectFileBuffer.at(filePos));
		mAnnotationList.back().byteSize++;
		break;

	case LS_ANNOTATION_MUL_ENDSTAR:
		//do nothing
		break;

	case LS_ANNOTATION_MUL_ENDSLASH:
		//do nothing
		break;

	case LS_PREPROCESS_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 0;
		token.content ="";
		token.type = TK_PREPROCESS;
		token.line = mLineNumberInSource;
		mTokenList.push_back(token);
		break;
	}

	case LS_PREPROCESS_INST_BODY://instruction body
		mTokenList.back().content.push_back(effectFileBuffer.at(filePos));
		mTokenList.back().byteSize++;
		break;

	case LS_IDENTIFIER_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 1;
		token.content = effectFileBuffer.at(filePos);
		token.type = TK_IDENTIFIER;
		token.line = mLineNumberInSource;
		mTokenList.push_back(token);
		break;
	}

	case LS_IDENTIFIER_BODY:
		mTokenList.back().content.push_back(effectFileBuffer.at(filePos));
		mTokenList.back().byteSize++;
		break;

		//DELIMITER  ; { } ( ) ,  \n  , 
	case LS_DELIM_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 1;
		token.content = effectFileBuffer.at(filePos);
		token.type = TK_DELIMITER;
		token.line = mLineNumberInSource;
		mTokenList.push_back(token);
		break;
	}


		//LITERAL STRING ""
	case LS_LITERAL_STRING_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 0;
		token.content = "";
		token.type = TK_LITERAL_STR;
		token.line = mLineNumberInSource;
		mTokenList.push_back(token);
		break;
	}

	case LS_LITERAL_STRING_BODY:
		mTokenList.back().content.push_back(effectFileBuffer.at(filePos));
		mTokenList.back().byteSize++;
		break;

	case LS_LITERAL_STRING_ESCAPED:
	{
		//escaped char
		uchar escapedChar = effectFileBuffer.at(filePos);
		switch (escapedChar)
		{
		case 'a':escapedChar = '\a';break;
		case 'b':escapedChar = '\b';break;
		case 'f':escapedChar = '\f';break;
		case 'n':escapedChar = '\n';break;
		case 'r':escapedChar = '\r';break;
		case 't':escapedChar = '\t';break;
		case 'v':escapedChar = '\v';break;
		case '0':escapedChar = '\0';break;
		case '?':escapedChar = '\?';break;
		// \" \' \\ are not neccessary
		}
		mTokenList.back().content.push_back(escapedChar);
		mTokenList.back().byteSize++;
		break;
	}

	case LS_NUMBER_NEWTOKEN:
	{
		N_TokenInfo token;
		token.byteOffset = filePos;
		token.byteSize = 1;
		token.content = effectFileBuffer.at(filePos);
		token.type = TK_NUMBER;
		token.line = mLineNumberInSource;
		mTokenList.push_back(token);
		break;
	}

	case LS_NUMBER_BODY:
		mTokenList.back().content.push_back(effectFileBuffer.at(filePos));
		mTokenList.back().byteSize++;
		break;

	case LS_ERROR:
		return false;
		break;

	default:
		std::cout << "OMG...Some unexpected problem occur in Tokenizer= =. Please Report to developer.";
		break;
	}	
	return true;
}

void NoiseEffectCompiler::IEffectTokenizer::mFunction_FindKeywordsInIdentifier()
{
	for (auto& t : mTokenList)
	{
		//if this token match preset string pattern in the keyword unordered_set, then it's a KEYWORD
		if (mKeywordList.find(t.content) != mKeywordList.end())
		{
			t.type = TK_KEYWORD;
		}
	}
}



//-------------------helper function----------------
inline bool NoiseEffectCompiler::isCharDigit(uchar c)
{
	return (c>=uchar('0') && c<=uchar('9'));
}

inline bool NoiseEffectCompiler::isCharLetter(uchar c)
{
	return (c >= uchar('a') && c <= uchar('z')) || (c >= uchar('A') && c <= uchar('Z'));
}

inline bool NoiseEffectCompiler::isCharLetterUnderline(uchar c)
{
	return (isCharLetter(c)||c=='_');
}

inline bool NoiseEffectCompiler::isCharLetterDigitUnderline(uchar c)
{
	return (isCharLetterUnderline(c) || isCharDigit(c)|| c=='_');
}

inline bool NoiseEffectCompiler::isCharDelimiter(uchar c)
{
	return (c == ';') || (c == '(') || (c == ')') || (c == '{') || (c == '}') ||(c==',');//interested delimiter in NoiseEffect
}

bool NoiseEffectCompiler::isCharSpaceTabNextline(uchar c)
{
	return (c==' ' || c=='\t' || c=='\n' || c=='\r');
}
