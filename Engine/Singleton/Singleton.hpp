#pragma once
#include <cassert>
#include <utility>
#include "../Utility1/Log/Log.hpp"

#define DECLARE_SINGLETON(Type)               \
    friend class Singleton<Type>;            \
    static constexpr const char* Name = #Type;

template <typename T>
class Singleton {
protected:
    static T* s_instance;

    Singleton() = default;
    virtual ~Singleton() = default;

public:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    // 生成
    template <typename... Args>
    static void Create(Args&&... args) {
        if (s_instance == nullptr) {
            LOG_INFO("Instanceの生成:{}", T::Name);
            s_instance = new T(std::forward<Args>(args)...);
        }
    }
    // 削除
    static void Delete() {
        if (s_instance != nullptr) {
            LOG_INFO("Instanceの削除:{}", T::Name);
            delete s_instance;
            s_instance = nullptr;
        }
    }
    // 取得
    static T& Get() {
        if (!s_instance) {
            LOG_CRITICAL("Instanceが存在しません");
            assert(s_instance != nullptr && "Instanceが存在しません");
        }
        return *s_instance;
    }
    // nullチェック用
    static T* GetPtr() {
        return s_instance;
    }
};

template <typename T> T* Singleton<T>::s_instance = nullptr;