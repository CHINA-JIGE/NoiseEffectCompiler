/******************************************************************

							IEffect Interface

		This is a Effect test interface which belong to the 
		Noise3D::Effect namespace. Unit test performed here 
		would be much more efficient, so this implementation 
		of Effect Interface will be transplanted into Noise3D  
		engine in the future.

********************************************************************/

#pragma once

namespace Noise3D
{
	namespace Effect
	{
		
		//this effect interface only rely on the Effect Type declaration header.
		//EffectTypeDecl.h is a common header that lies in both 2 projects.

		//------------------------------PASS-------------------------
		class IPass
		{
		public:

			void Apply();

		private:

			//NoiseEffectCompiler namespace shouldn't be exposed to Noise3D namespace

			void SetVS(const NoiseEffectCompiler::N_SHADER_DESC& shader);

			void SetGS(const NoiseEffectCompiler::N_SHADER_DESC& shader);

			void SetPS(const NoiseEffectCompiler::N_SHADER_DESC& shader);

			NoiseEffectCompiler::N_SHADER_DESC* GetVS();

			NoiseEffectCompiler::N_SHADER_DESC* GetGS();

			NoiseEffectCompiler::N_SHADER_DESC* GetPS();

			IPass();

			~IPass();

			friend NoiseEffectCompiler::IFactory< NoiseEffectCompiler::IPassDesc>;

			NoiseEffectCompiler::N_SHADER_DESC mVS;
			NoiseEffectCompiler::N_SHADER_DESC mGS;
			NoiseEffectCompiler::N_SHADER_DESC mPS;

		};


		//-------------------------TECHNIQUE--------------------------
		class ITechnique : public NoiseEffectCompiler::IFactory<IPass>
		{
		public:


			void Apply();

		private:

			//theoretically, pass count won't be limited,
			//but too many pass could cause performance overhead
			ITechnique();

			~ITechnique();

			friend IFactory<ITechnique>;

		};



		//-------------------------EFFECT------------------------
		class IEffect : public NoiseEffectCompiler::IFactory<ITechnique>
		{
		public:

			//Root interface of Effect Framework, owns Technique child object
			//for a specific render effect
			IEffect();

			bool Init(const std::string& nxoFilePath);


		private:
			std::unordered_map<std::string, N_CBUFFER_INFO> mGlobalCBs;
			std::unordered_map<std::string, N_TBUFFER_INFO> mGlobalTBs;
			std::unordered_map<std::string, N_TEXTURE_RESOURCE_INFO> mGlobalShaderResources;
			std::unordered_map<std::string, N_SAMPLER_INFO> mGlobalSamplers;
			std::unordered_map<std::string, ID3DBlob*> uniqueShaerBinary;

		};



	}
}