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

		bool OutputBinaryToFile(
			const std::string& outPath,
			const std::vector<N_UNIQUE_SHADER>& uniqueShaderList,
			const IEffectDesc* pEffect);

	private:

		void mFunction_WriteGlobalResources(
			const std::unordered_map<std::string, N_CBUFFER_INFO>& uniqueCBs,
			const std::unordered_map<std::string, N_TBUFFER_INFO>& uniqueTBs,
			const std::unordered_map<std::string, N_TEXTURE_RESOURCE_INFO>& uniqueTextures,
			const std::unordered_map<std::string, N_SAMPLER_INFO>& uniqueSamplers,
			const std::vector<std::pair<std::string,ID3DBlob*>>& uniqueShaderBinarys);

		void mFunction_WriteEffectHierarchy(const IEffectDesc* pEffect);

		void mFunction_WritePass(IPassDesc* pPass);

		void mFunction_WriteShader(N_SHADER_DESC* pShader);

		template<typename T>
		inline void mFunction_BinaryWrite(const T& val);

		void mFunction_WriteBuffer(uchar val);

		void mFunction_WriteBuffer(uint16_t val);

		void mFunction_WriteBuffer(uint32_t val);

		void mFunction_WriteBuffer(const char* val);

		void mFunction_WriteBuffer(std::string val);

		void mFunction_WriteBuffer(const N_CBUFFER_INFO& val);

		void mFunction_WriteBuffer(const N_TBUFFER_INFO& val);

		void mFunction_WriteBuffer(const N_TEXTURE_RESOURCE_INFO& val);

		void mFunction_WriteBuffer(const N_SAMPLER_INFO& val);

		void mFunction_WriteBuffer(ID3DBlob* val);

		std::vector<uchar> mFileBuffer;//a buffer to output .nxo file

	};
	
};