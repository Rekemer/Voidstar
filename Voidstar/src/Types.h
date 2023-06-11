#pragma once
#include<memory>
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