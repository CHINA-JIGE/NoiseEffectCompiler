/*********************************************************************

									FILE OUTPUT

			Intermediate Effect Bytecode generator and
			file writer, .nxo will be output by this class
			

*********************************************************************/

#pragma once

namespace NoiseEffectCompiler
{

	class IFileWriter
	{
	public:

		void OutputBinary(
			const std::string& outPath,
			const std::vector<N_UNIQUE_SHADER>& uniqueShaderList,
			const IEffect* pEffect);

	private:

		void mFunction_WritePass(IPass* pPass);

		std::vector<uchar> mFileBuffer;

		template <typename T>
		void mFunction_WriteBuffer(T val);

	};
	
};