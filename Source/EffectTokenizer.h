/***************************************************************

							EFFECT TOKENIZER

			1.generate tokens from .noiseEffect file

***********************************************************/
#pragma once


namespace NoiseEffectCompiler
{
	typedef unsigned char uchar;

	bool isCharDigit(uchar c);

	bool isCharLetter(uchar c);

	bool isCharLetterUnderline(uchar c);

	bool isCharLetterDigitUnderline(uchar c);

	bool isCharDelimiter(uchar c);

	enum TOKEN_TYPE
	{
		TK_ANNOTATION=0,
		TK_DELIMITER=1,
		TK_PREPROCESS=2,
		TK_LITERAL_STR=3,
		TK_IDENTIFIER=4,
		TK_NUMBER=5
	};

	struct N_TokenInfo
	{
		UINT offset;//byte pos in file
		UINT size;
		TOKEN_TYPE type;
		std::string content;
	};

	//lexical analyzer (tokenizer)
	class IEffectTokenizer
	{
		public:

			IEffectTokenizer();

			~IEffectTokenizer();

			void	Tokenize(const std::vector<unsigned char>& effectFileBuffer);//step 1
															  
		private:

			enum LEXER_STATE
			{
				//COMMON STATE
				LS_NORMAL = 0,

				//  ANNOTATION
				LS_SLASH = 1,
				LS_ANNOTATION_SINGLE_NEWTOKEN=2,
				LS_ANNOTATION_SINGLE_BODY = 3,// Enter annotation <//>
				LS_ANNOTATION_MUL_NEWTOKEN = 4, //start of /* annotation
				LS_ANNOTATION_MUL_BODY = 5,// /* annotation
				LS_ANNOTATION_MUL_ENDSTAR = 6,// /* * annotation might come to an end

				//PREPROCESS INSTRUCTION
				LS_PREPROCESS_SHARP = 7, // #
				LS_PREPROCESS_NEWTOKEN = 8,//#INCLUDE

				//IDENTIFIER
				LS_IDENTIFIER_NEWTOKEN = 9,
				LS_IDENTIFIER_APPEND_CHAR = 10,

				//DELIMITER  ; { } ( ) ,  \n  , + - *  / \ = [ ] | <> :
				LS_DELIM_NEWTOKEN = 11,

				//LITERAL STRING ""
				LS_LITERAL_STRING_NEWTOKEN = 12,
				LS_LITERAL_STRING_BODY = 13,//MAIN BODY OF LITERAL STRING
				LS_LITERAL_STRING_ESCAPED = 14, // ENTER ESCAPING STATE \ 

				LS_ERROR = 9999,
			};

			LEXER_STATE mLexerState;

			std::vector<N_TokenInfo> mTokenList;

			void mFunction_LexerStateTransition(const std::vector<unsigned char>& effectFileBuffer,UINT filePos);//transition according to input char

			void mFunction_LexerStateMachineOutput();

	};

};