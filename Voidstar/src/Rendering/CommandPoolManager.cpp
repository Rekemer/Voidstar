#include"Prereq.h"
#include"CommandPoolManager.h"
#include"RenderContext.h"
#include"Device.h"
#include"Log.h"
namespace Voidstar
{

   

    void CommandPoolManager::Release() {

        if (m_InUsePools.size() > 0)
        {
            Log::GetLog()->error("Not all pools are deallocated!");
        }
        for (const auto& pool : m_FreePools)
        {
            RenderContext::GetDevice()->GetDevice().destroyCommandPool(pool);
        }

    }

    vk::CommandPool CommandPoolManager::GetFreePool() {
        if (!m_FreePools.empty()) {
            const auto pool = m_FreePools.back();
            m_FreePools.pop_back();
            return pool;
        }
        auto pool = CreatePool();
        m_InUsePools.emplace_back(pool);
        return pool;
    }

    void CommandPoolManager::FreePool(vk::CommandPool pool) {
        for (uint32_t i = 0; i < (uint32_t)m_InUsePools.size(); ++i) {
            if (m_InUsePools[i] == pool) {
                RenderContext::GetDevice()->GetDevice().resetCommandPool(m_InUsePools[i]);
                m_InUsePools.erase(m_InUsePools.begin() + i);
                m_FreePools.push_back(pool);
                break;
            }
        }
    }

    vk::CommandPool CommandPoolManager::CreatePool() const {
        vk::CommandPoolCreateInfo cmdPoolInfo;
        cmdPoolInfo.queueFamilyIndex = RenderContext::GetDevice()->GetGraphicsIndex();
         cmdPoolInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
        const auto res = RenderContext::GetDevice()->GetDevice().createCommandPool(cmdPoolInfo);
        return res;
    }
}