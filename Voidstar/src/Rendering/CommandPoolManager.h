#pragma once
#include "vulkan/vulkan.hpp"
#include <vector>
namespace Voidstar
{
    class CommandPoolManager {
    public:
        CommandPoolManager() = default;
        ~CommandPoolManager() = default;

        void Release();


        vk::CommandPool GetFreePool();
        void FreePool(vk::CommandPool pool);

    private:

        std::vector<vk::CommandPool> m_InUsePools = {};
        std::vector<vk::CommandPool> m_FreePools = {};

        vk::CommandPool CreatePool() const;
    };
}