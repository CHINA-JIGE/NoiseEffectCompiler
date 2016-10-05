/***************************************************************

							EFFECT TOKENIZER

***********************************************************/

#include "EffectCompiler.h"

using namespace NoiseEffectCompiler;


NoiseEffectCompiler::IEffectTokenizer::IEffectTokenizer()
	: mLexerState(LS_NORMAL)
{
}

NoiseEffectCompiler::IEffectTokenizer::~IEffectTokenizer()
{
	mLexerState = LS_NORMAL;
	mTokenList.clear();
}

void NoiseEffectCompiler::IEffectTokenizer::Tokenize(const std::vector<unsigned char>& effectFileBuffer)
{
	//to match tokens, Regular Expression or State Machine schemes can be used
	//so now I decided to use State Machine (refer to doc for state transition diagram )
	//to generate tokens. SEE file://
	//(2016.10.3)actually I can't really understand NFA->DFA conversion now... thus I could only
	//managed to borrow this state machine idea to implement my own algorithm


	//start iteration
	UINT totalByteSize = effectFileBuffer.size();
	for (UINT currPos = 0;currPos < totalByteSize;currPos++)
	{
		mFunction_LexerStateTransition(effectFileBuffer,currPos);
		mFunction_LexerStateMachineOutput();
	};

}

void NoiseEffectCompiler::IEffectTokenizer::mFunction_LexerStateTransition(const std::vector<unsigned char>& effectFileBuffer,UINT filePos)
{
	unsigned char c = effectFileBuffer.at(filePos);

	static std::string errorMsg = "";

	switch (mLexerState)
	{

	case	LS_NORMAL:
		if (c == '/') { mLexerState = LS_SLASH; }
		else if (c == ' ' || c=='\t') {}//skip spaces/tabs
		else if (c == '#') { mLexerState = LS_PREPROCESS_SHARP; }
		else if (isCharLetterUnderline(c)) { mLexerState = LS_IDENTIFIER_NEWTOKEN; }//identifier begin with _ or letter
		else if (isCharDelimiter(c)) { mLexerState = LS_DELIM_NEWTOKEN; }//new delimiter token
		else { mLexerState = LS_ERROR; }
			break;

	case LS_SLASH:
		if (c == '/') { mLexerState = LS_ANNOTATION_SINGLE_NEWTOKEN; }// double slashes now//
		else if (c == '*') { mLexerState = LS_ANNOTATION_MUL_NEWTOKEN; }// slash-star /*
		break;

	// annotation entry <//>
	case LS_ANNOTATION_SINGLE_NEWTOKEN:
		mLexerState = LS_ANNOTATION_SINGLE_BODY;//after create anno token, read the body

	//  annotation body <//>
	case LS_ANNOTATION_SINGLE_BODY:

		if (c == '\n') { mLexerState = LS_NORMAL; }
		break;

	case LS_ANNOTATION_MUL_NEWTOKEN:

		break;//start of /* annotation

	case LS_ANNOTATION_MUL_BODY:
		break;

	case LS_ANNOTATION_MUL_ENDSTAR:
		break;

	case LS_PREPROCESS_SHARP:
		break;

	case LS_PREPROCESS_NEWTOKEN:
		break;

	case LS_IDENTIFIER_NEWTOKEN:
		break;

	case LS_IDENTIFIER_APPEND_CHAR:
		break;

		//DELIMITER  ; { } ( ) ,  \n  , + - *  / \ = [ ] | <> :
	case LS_DELIM_NEWTOKEN:
		break;

		//LITERAL STRING ""
	case LS_LITERAL_STRING_NEWTOKEN:
		break;

	case LS_LITERAL_STRING_BODY:
		break;

	case LS_LITERAL_STRING_ESCAPED:
		break;

	case LS_ERROR:
		std::cout << "Noise Effect Compiler : Error Occur! " + errorMsg << std::endl;
		break;

	}
}

void NoiseEffectCompiler::IEffectTokenizer::mFunction_LexerStateMachineOutput()
{
}

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
	return (isCharLetterUnderline(c) || c=='_');
}

inline bool NoiseEffectCompiler::isCharDelimiter(uchar c)
{
	return (c == ';') || (c == '(') || (c == ')') || (c == '{') || (c == '}') || (c == '\n');//interested delimiter in NoiseEffect
}
