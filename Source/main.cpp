/***************************************************************

							Main Program

***********************************************************/

#include "EffectCompiler.h"

NoiseEffectCompiler::IEffectCompiler compiler;

int main(int argc, char* argv[])
{
	//title
	std::cout << "Noise Effect Compiler -- Powered by Jige" << std::endl << std::endl;

	if (!compiler.ParseCommandLine(argc, argv))
	{
		std::cout << "Command Line Invalid!.";
		system("pause");
		return 0;
	};

	//compile
	compiler.Compile();

	return 1;
}

