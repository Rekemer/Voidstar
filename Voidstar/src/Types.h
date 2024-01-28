#pragma once
#include<memory>
#include<functional>
namespace Voidstar
{
    template<typename T>
    using UPtr = std::unique_ptr<T>;
    template<typename T>
    using SPtr = std::shared_ptr<T>;

    template<typename T, typename... Args>
    UPtr <T> CreateUPtr(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    template<typename T, typename... Args>
    SPtr <T> CreateSPtr(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    class CommandBuffer;
    using Func = std::function<void(CommandBuffer& cmd, size_t frameIndex)>;
}
