#pragma once
#include<list>
#include<unordered_map>
namespace Voidstar
{
	struct PageEntry
	{
		PageEntry() = default;
		PageEntry(float mipMap, glm::vec2 pageTableCoord, glm::vec2 physCoord):
			mipMap{ mipMap }, pageTableCoord{ pageTableCoord }, physCoord{physCoord}
		{

		}
		bool operator == (const PageEntry& page)
		{
			return page.mipMap == mipMap &&
				page.pageTableCoord == pageTableCoord &&
				page.physCoord == physCoord;
		}
		glm::vec2 pageTableCoord= { -1,-1 };
		float mipMap = -1;
		glm::vec2 physCoord = { -1,-1 };
	};

	template<int size>
	class Cache
	{
	public:
		PageEntry* Get(std::string key)
		{
			auto iter = m_Cached.find(key);
			auto isCached = iter != m_Cached.end();
			if (isCached)
			{
				return &(*iter->second);
			}	
			return nullptr;
		}

		void Add(PageEntry& data, std::string_view name)
		{
			if (m_Cached.find(name.data()) != m_Cached.end())return;
			m_Cache.push_front(data);
			m_Cached[name.data()] = m_Cache.begin();
		}
	private:
		void MoveToHead()
		{

		}
	private:

		std::unordered_map<std::string, std::list<PageEntry>::iterator> m_Cached;
		std::list<PageEntry> m_Cache;
		inline static int m_Id = 0;
		int m_CacheSize = size;
	};
}