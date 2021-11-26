/*
 * Copyright (c) 2021, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "AK/Assertions.h"
#include <AK/Error.h>
#include <AK/Optional.h>
#include <AK/StdLibExtras.h>
#include <AK/Types.h>

#ifndef __cpp_impl_coroutine
#    error "Coroutines must be enabled in the compiler options to use AK::CoroutineHandle!"
#endif

#ifndef AK_NO_REPLACE_STD
namespace std { // NOLINT(cert-dcl58-cpp) coroutine_traits required to be in ::std

// [dcl.fct.def.coroutine]
// The promise type of a coroutine is std​::​coroutine_­traits<R, P1, …, Pn>​::​promise_­type, where R is the return type of the function, and
// P1 … Pn are the sequence of types of the non-object function parameters, preceded by the type of the object parameter ([dcl.fct])
// if the coroutine is a non-static member function. The promise type shall be a class type. template<typename R, typename... Args>

template<typename R, typename = void>
struct __CoroutineTraits {
};

template<typename R>
struct __CoroutineTraits<R, AK::Detail::Void<typename R::promise_type>> {
    using promise_type = typename R::promise_type;
};

template<typename R, typename... Args>
struct coroutine_traits : __CoroutineTraits<R> {
};

template<typename PromiseT = void>
class coroutine_handle;

template<>
class coroutine_handle<void> {
public:
    constexpr coroutine_handle() noexcept = default;
    constexpr coroutine_handle(std::nullptr_t) noexcept { }
    coroutine_handle& operator=(std::nullptr_t) noexcept
    {
        m_handle = nullptr;
        return *this;
    }

    constexpr void* address() const noexcept { return m_handle; }
    static constexpr coroutine_handle from_address(void* address)
    {
        coroutine_handle tmp;
        tmp.m_handle = address;
        return tmp;
    }

    constexpr explicit operator bool() const noexcept { return m_handle != nullptr; }
    bool done() const { return __builtin_coro_done(m_handle); }

    void operator()() const { return resume(); }
    void resume() const { __builtin_coro_resume(m_handle); }
    void destroy() const { __builtin_coro_destroy(m_handle); }

private:
    void* m_handle { nullptr };
};

template<typename PromiseT>
class coroutine_handle {
public:
    constexpr coroutine_handle() noexcept = default;
    constexpr coroutine_handle(std::nullptr_t) noexcept { }
    coroutine_handle& operator=(std::nullptr_t) noexcept
    {
        m_handle = nullptr;
        return *this;
    }

    constexpr void* address() const noexcept { return m_handle; }
    static constexpr coroutine_handle from_address(void* address) noexcept
    {
        coroutine_handle tmp;
        tmp.m_handle = address;
        return tmp;
    }

    static constexpr coroutine_handle from_promise(PromiseT& promise) noexcept
    {
        coroutine_handle tmp;
        tmp.m_handle = __builtin_coro_promise(&promise, alignof(PromiseT), true);
        return tmp;
    }

    constexpr operator coroutine_handle<>() const noexcept { return coroutine_handle<>::from_address(address()); }
    constexpr explicit operator bool() const noexcept { return m_handle != nullptr; }
    bool done() const { return __builtin_coro_done(m_handle); }

    void operator()() const { return resume(); }
    void resume() const { __builtin_coro_resume(m_handle); }
    void destroy() const { __builtin_coro_destroy(m_handle); }

    PromiseT& promise() const { return *static_cast<PromiseT*>(__builtin_coro_promise(m_handle, alignof(PromiseT), false)); }

private:
    void* m_handle { nullptr };
};

struct noop_coroutine_promise {
};

template<>
struct coroutine_handle<noop_coroutine_promise> {

    constexpr void* address() const noexcept { return m_handle; }
    constexpr operator coroutine_handle<>() const noexcept { return coroutine_handle<>::from_address(address()); }
    constexpr explicit operator bool() const noexcept { return true; }
    constexpr bool done() const noexcept { return false; }

    constexpr void operator()() const noexcept { }
    constexpr void resume() const noexcept { }
    constexpr void destroy() const noexcept { }

    noop_coroutine_promise& promise() const noexcept { return *static_cast<noop_coroutine_promise*>(__builtin_coro_promise(m_handle, alignof(noop_coroutine_promise), false)); }

private:
    friend coroutine_handle<noop_coroutine_promise> noop_coroutine() noexcept;

// Spooky compiler internals alert! GCC doesn't use the builtin, so we need to basically copy the impl from libstdc++ :/
#    if __has_builtin(__builtin_coro_noop)
    // constructor
    coroutine_handle() noexcept
    {
        m_handle = __builtin_coro_noop();
    }

    void* m_handle = nullptr;
#    else
    struct __frame {
        static void __noop() { }
        void (*m_resume)() = __noop;
        void (*m_destroy)() = __noop;
        struct noop_coroutine_promise m_promise;
    };
    static __frame s_frame;
    void* m_handle = &s_frame;
#    endif
};

using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;

#    if !__has_builtin(__builtin_coro_noop)
inline noop_coroutine_handle::__frame noop_coroutine_handle::s_frame {};
#    endif

inline noop_coroutine_handle noop_coroutine() noexcept
{
    return noop_coroutine_handle {};
}

}
#endif // AK_NO_REPLACE_STD

namespace AK {

template<typename T = void>
using CoroutineHandle = ::std::coroutine_handle<T>;
using NoopCoroutineHandle = ::std::noop_coroutine_handle;
inline NoopCoroutineHandle noop_coroutine() noexcept { return ::std::noop_coroutine(); }

struct SuspendAlways {
    constexpr bool await_ready() const noexcept { return true; }
    constexpr void await_suspend(CoroutineHandle<>) const noexcept { }
    constexpr void await_resume() const noexcept {};
};
struct SuspendNever {
    constexpr bool await_ready() const noexcept { return false; }
    constexpr void await_suspend(CoroutineHandle<>) const noexcept { }
    constexpr void await_resume() const noexcept {};
};

// ^^^ ::std classes vvv custom classes

template<typename Value>
class Promise {
public:
    struct FinalAwaitable {
        bool await_ready() const noexcept { return false; }
        template<typename OtherPromiseT>
        CoroutineHandle<> await_suspend(CoroutineHandle<OtherPromiseT> coroutine) noexcept
        {
            auto next_coroutine = coroutine.promise().m_continuation; // symmetric transfer online :robot:
            return next_coroutine ? next_coroutine : noop_coroutine();
        }
        void await_resume() noexcept { }
    };

    auto initial_suspend() noexcept { return SuspendAlways {}; }
    auto final_suspend() noexcept { return FinalAwaitable {}; }
    void unhandled_exception() noexcept { VERIFY_NOT_REACHED(); } // clang requires this even with -fno-exceptions

    void set_continuation(CoroutineHandle<> continuation) noexcept
    {
        m_continuation = continuation;
    }

    // These two will return Task<Value> but we didn't define Task yet
    static auto get_return_object_on_allocation_failure();
    auto get_return_object();

    template<typename ReturnT>
    requires(IsConvertible<ReturnT&&, Value>) void return_value(ReturnT&& value)
    {
        m_value = forward<ReturnT>(value);
    }

    // FIXME: return_void?

    bool has_value() const { return m_value.has_value(); }
    Value& value() { return m_value.value(); }
    Value&& release_value() { return m_value.release_value(); }

private:
    CoroutineHandle<> m_continuation;
    Optional<Value> m_value;
};

template<typename Value>
class Task {
public:
    // snake_case type required per C++ spec
    using PromiseType = Promise<Value>;
    using promise_type = PromiseType;

    Task() noexcept = default;

    explicit Task(CoroutineHandle<PromiseType> coroutine)
        : m_coroutine { coroutine }
    {
    }

    Task(Task&& other) noexcept
        : m_coroutine(other.m_coroutine)
    {
        other.m_coroutine = nullptr;
    }

    Task& operator=(Task&& other) noexcept
    {
        if (&other != this) {
            if (m_coroutine) {
                m_coroutine.destroy();
                m_coroutine = nullptr;
            }
            exchange(this->m_coroutine, other.m_coroutine);
        }
        return *this;
    }

    ~Task()
    {
        if (m_coroutine) {
            m_coroutine.destroy();
        }
    }

    bool done() const noexcept { return !m_coroutine || m_coroutine.done(); }

    // Awaitable contract
    bool await_ready() const noexcept { return done(); }

    CoroutineHandle<> await_suspend(CoroutineHandle<> suspender) const noexcept
    {
        m_coroutine.promise().set_continuation(suspender);
        return m_coroutine;
    }

    decltype(auto) await_resume() const& noexcept
    {
        // FIXME: What if !promise().has_value() or !m_coroutine (e.g. on allocation failure) ?
        return m_coroutine.promise().value();
    };

    decltype(auto) await_resume() const&& noexcept
    {
        return m_coroutine.promise().release_value();
    };

    // FIXME: What if not done? How are you *actually* supposed to get the value out of your task?
    PromiseType& promise() const
    {
        return m_coroutine.promise();
    }

private:
    CoroutineHandle<PromiseType> m_coroutine = nullptr;
};

template<typename Value>
auto Promise<Value>::get_return_object_on_allocation_failure()
{
    return Task<Value> {};
}

template<typename Value>
auto Promise<Value>::get_return_object()
{
    return Task<Value> { CoroutineHandle<Promise<Value>>::from_promise(*this) };
}

} // namespace AK

using AK::CoroutineHandle;
using AK::Promise;
using AK::SuspendAlways;
using AK::SuspendNever;
using AK::Task;
