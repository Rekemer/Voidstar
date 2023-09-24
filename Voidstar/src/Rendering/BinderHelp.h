#pragma once
namespace Voidstar
{
	enum class PipelineType
	{
		RENDER = 0,
		COMPUTE = 10
	};
	struct EnumClassHash
	{
		template <typename T>
		std::size_t operator()(T t) const
		{
			return static_cast<std::size_t>(t);
		}
		template <>
		std::size_t operator() < std::pair<int, PipelineType > > (std::pair<int, PipelineType> t) const
		{
			return static_cast<std::size_t>(t.first) + static_cast<std::size_t>(t.second);
		}
	};
}
