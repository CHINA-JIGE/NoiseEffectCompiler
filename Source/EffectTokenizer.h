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

	bool isCharSpaceTabNextline(uchar c);

	enum TOKEN_TYPE
	{
		TK_ANNOTATION=0,
		TK_DELIMITER=1,
		TK_PREPROCESS=2,
		TK_LITERAL_STR=3,
		TK_IDENTIFIER=4,
		TK_NUMBER=5,
		TK_KEYWORD=6,
	};

	struct N_TokenInfo
	{
		UINT line;//Line number in source file
		UINT byteOffset;//byte pos in file
		UINT byteSize;
		TOKEN_TYPE type;
		std::string content;
	};

	//lexical analyzer (tokenizer)
	class IEffectTokenizer
	{
		public:

			IEffectTokenizer();

			~IEffectTokenizer();

			bool	Tokenize(const std::vector<unsigned char>& effectFileBuffer);//step 1
		
			void GetTokenList(std::vector<N_TokenInfo>& outTokenList);//step2

			void GetAnnotationList(std::vector<N_TokenInfo>& outTokenList);

		private:

			enum LEXER_STATE
			{
				//COMMON STATE
				LS_NORMAL = 0,

				//  ANNOTATION
				LS_SLASH = 1,
				LS_ANNOTATION_SINGLE_NEWTOKEN = 2,
				LS_ANNOTATION_SINGLE_BODY = 3,// Enter annotation <//>
				LS_ANNOTATION_MUL_NEWTOKEN = 4, //start of /* annotation
				LS_ANNOTATION_MUL_BODY = 5,// /* annotation
				LS_ANNOTATION_MUL_ENDSTAR = 6,// /* * annotation might come to an end
				LS_ANNOTATION_MUL_ENDSLASH=7,

				//PREPROCESS INSTRUCTION
				LS_PREPROCESS_NEWTOKEN = 11, // #
				LS_PREPROCESS_INST_BODY = 12,//instruction body

				//IDENTIFIER
				LS_IDENTIFIER_NEWTOKEN = 21,
				LS_IDENTIFIER_BODY = 22,

				//DELIMITER  ; { } ( ) ,  \n  , + - *  / \ = [ ] | <> :
				LS_DELIM_NEWTOKEN = 31,

				//LITERAL STRING ""
				LS_LITERAL_STRING_NEWTOKEN = 41,
				LS_LITERAL_STRING_BODY = 42,//MAIN BODY OF LITERAL STRING
				LS_LITERAL_STRING_ESCAPED = 43, // ENTER ESCAPING STATE \ 

				//number
				LS_NUMBER_NEWTOKEN=51,
				LS_NUMBER_BODY=52,

				LS_ERROR = 9999,
			};

			LEXER_STATE mLexerState;

			UINT mLineNumberInSource;

			std::unordered_set<std::string> mKeywordList;//holds preset keywords for verification

			std::vector<N_TokenInfo> mTokenList;

			std::vector<N_TokenInfo> mAnnotationList;

			void mFunction_LexerStateTransition(const std::vector<unsigned char>& effectFileBuffer,UINT filePos);//transition according to input char

			bool mFunction_LexerStateMachineOutput(const std::vector<unsigned char>& effectFileBuffer, UINT filePos);

			void mFunction_FindKeywordsInIdentifier();

	};

};