/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021-2022, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/DeprecatedFlyString.h>
#include <AK/String.h>
#include <LibJS/Runtime/Completion.h>
#include <LibJS/Runtime/Object.h>
#include <LibJS/SourceRange.h>

namespace JS {

struct TracebackFrame {
    DeprecatedFlyString function_name;
    SourceRange source_range;
};

class Error : public Object {
    JS_OBJECT(Error, Object);

public:
    static NonnullGCPtr<Error> create(Realm&);
    static NonnullGCPtr<Error> create(Realm&, String message);
    static ThrowCompletionOr<NonnullGCPtr<Error>> create(Realm&, StringView message);

    virtual ~Error() override = default;

    [[nodiscard]] ThrowCompletionOr<String> stack_string(VM&) const;

    ThrowCompletionOr<void> install_error_cause(Value options);

    Vector<TracebackFrame, 32> const& traceback() const { return m_traceback; }

protected:
    explicit Error(Object& prototype);

private:
    void populate_stack();
    Vector<TracebackFrame, 32> m_traceback;
};

// NOTE: Making these inherit from Error is not required by the spec but
//       our way of implementing the [[ErrorData]] internal slot, which is
//       used in Object.prototype.toString().
#define JS_DECLARE_NATIVE_ERROR(ClassName, snake_name, PrototypeName, ConstructorName)                    \
    class ClassName final : public JS::Error {                                                            \
        JS_OBJECT(ClassName, JS::Error);                                                                  \
                                                                                                          \
    public:                                                                                               \
        static JS::NonnullGCPtr<ClassName> create(JS::Realm&);                                            \
        static JS::NonnullGCPtr<ClassName> create(JS::Realm&, String message);                            \
        static JS::ThrowCompletionOr<JS::NonnullGCPtr<ClassName>> create(JS::Realm&, StringView message); \
                                                                                                          \
        explicit ClassName(JS::Object& prototype);                                                        \
        virtual ~ClassName() override = default;                                                          \
    };

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR(ClassName, snake_name, PrototypeName, ConstructorName)
JS_ENUMERATE_NATIVE_ERRORS
#undef __JS_ENUMERATE

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    , ClassName
template<typename T>
inline constexpr bool IsECMA262NativeError = IsOneOf<T
        JS_ENUMERATE_NATIVE_ERRORS>;
#undef __JS_ENUMERATE

namespace Web::WebAssembly {
class CompileError;
}

// NOTE: This is in the header so that extension specs can define their own native errors
#define JS_IMPLEMENT_NATIVE_ERROR(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeExpression)                 \
    JS::NonnullGCPtr<ClassName> ClassName::create(JS::Realm& realm)                                                                      \
    {                                                                                                                                    \
        return realm.heap().allocate<ClassName>(realm, PrototypeExpression).release_allocated_value_but_fixme_should_propagate_errors(); \
    }                                                                                                                                    \
                                                                                                                                         \
    JS::NonnullGCPtr<ClassName> ClassName::create(JS::Realm& realm, String message)                                                      \
    {                                                                                                                                    \
        auto& vm = realm.vm();                                                                                                           \
        auto error = ClassName::create(realm);                                                                                           \
        u8 attr = JS::Attribute::Writable | JS::Attribute::Configurable;                                                                 \
        error->define_direct_property(vm.names.message, JS::PrimitiveString::create(vm, move(message)), attr);                           \
        return error;                                                                                                                    \
    }                                                                                                                                    \
                                                                                                                                         \
    JS::ThrowCompletionOr<JS::NonnullGCPtr<ClassName>> ClassName::create(JS::Realm& realm, StringView message)                           \
    {                                                                                                                                    \
        return create(realm, TRY_OR_THROW_OOM(realm.vm(), String::from_utf8(message)));                                                  \
    }                                                                                                                                    \
                                                                                                                                         \
    ClassName::ClassName(JS::Object& prototype)                                                                                          \
        : JS::Error(prototype)                                                                                                           \
    {                                                                                                                                    \
    }

}
