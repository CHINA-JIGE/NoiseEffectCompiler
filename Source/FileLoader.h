/*********************************************************************

									FILE Loader

			Intermediate Effect Bytecode loader
			
*********************************************************************/

#pragma once

namespace Noise3D
{
	namespace Effect
	{

		class IFileLoader
		{
		public:

			bool LoadNXOFromFile(const std::string& filePath, IEffect* pEffect);

		private:

			bool mFunction_LoadGlobalResources(IEffect* pEffect);

			bool mFunction_LoadEffectHierarchy(IEffect* pEffect);

			bool mFunction_LoadPass(IPass* pEffect);

			bool mFunction_LoadShader(NoiseEffectCompiler::N_SHADER_DESC& desc);

		};

	}
}