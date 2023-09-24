#pragma once
#include"../Prereq.h"
#include"BinderHelp.h"
#include"Renderer.h"

namespace Voidstar
{
	template <PipelineType v>
		struct Int2Type
	{
		static constexpr PipelineType value = v;
	};

	// amount of sets to duplicate sets
// desc amount how many descriptors are bind to the same number
// should use GUID instead of pair?

	typedef  Int2Type<PipelineType::RENDER> RENDER;
	typedef  Int2Type<PipelineType::COMPUTE> COMPUTE;

	template<typename T>
	struct Binder
	{
		static int currentIndex;
		int BeginBind(int amountOfSets = 1)
		{
			auto pipeline = T::value;
			Renderer::Instance()->GetSets()[{currentIndex, pipeline}] = amountOfSets;
			int retValue = currentIndex++;
			return retValue;
		}

		void Bind(int binding, int descAmount, vk::DescriptorType type, vk::ShaderStageFlags shaderAccess)
		{
			auto pipeline = T::value;
			// create descriptor binding
			auto descBinding = DescriptorBindingDescription(binding, type, shaderAccess, descAmount);
			Renderer::Instance()->GetBindings()[{currentIndex-1, pipeline}].emplace_back(descBinding);
			
		}
	};
	template<>
	int Binder<COMPUTE>::currentIndex = 0;
	template<>
	int Binder<RENDER>::currentIndex = 0;
}
