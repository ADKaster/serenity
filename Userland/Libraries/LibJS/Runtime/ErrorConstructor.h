/*
 * Copyright (c) 2020-2022, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/NativeFunction.h>

namespace JS {

class ErrorConstructor final : public NativeFunction {
    JS_OBJECT(ErrorConstructor, NativeFunction);

public:
    virtual ThrowCompletionOr<void> initialize(Realm&) override;
    virtual ~ErrorConstructor() override = default;

    virtual ThrowCompletionOr<Value> call() override;
    virtual ThrowCompletionOr<NonnullGCPtr<Object>> construct(FunctionObject& new_target) override;

private:
    explicit ErrorConstructor(Realm&);

    virtual bool has_constructor() const override { return true; }
};

#define JS_DECLARE_NATIVE_ERROR_CONSTRUCTOR(ClassName, snake_name, PrototypeName, ConstructorName)                      \
    class ConstructorName final : public JS::NativeFunction {                                                           \
        JS_OBJECT(ConstructorName, JS::NativeFunction);                                                                 \
                                                                                                                        \
    public:                                                                                                             \
        virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;                                            \
        virtual ~ConstructorName() override;                                                                            \
        virtual JS::ThrowCompletionOr<JS::Value> call() override;                                                       \
        virtual JS::ThrowCompletionOr<JS::NonnullGCPtr<JS::Object>> construct(JS::FunctionObject& new_target) override; \
                                                                                                                        \
    private:                                                                                                            \
        explicit ConstructorName(JS::Realm&);                                                                           \
                                                                                                                        \
        virtual bool has_constructor() const override                                                                   \
        {                                                                                                               \
            return true;                                                                                                \
        }                                                                                                               \
    };

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR_CONSTRUCTOR(ClassName, snake_name, PrototypeName, ConstructorName)
JS_ENUMERATE_NATIVE_ERRORS
#undef __JS_ENUMERATE

// NOTE: This is in the header so that extension specs can define their own native errors
#define JS_IMPLEMENT_NATIVE_ERROR_CONSTRUCTOR(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeFromRealm, DefaultPrototypeGetter) \
    ConstructorName::ConstructorName(JS::Realm& realm)                                                                                                      \
        : NativeFunction(realm.vm().names.ClassName.as_string(), *static_cast<JS::Object*>(realm.intrinsics().error_constructor()))                         \
    {                                                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    JS::ThrowCompletionOr<void> ConstructorName::initialize(JS::Realm& realm)                                                                               \
    {                                                                                                                                                       \
        auto& vm = this->vm();                                                                                                                              \
        MUST_OR_THROW_OOM(NativeFunction::initialize(realm));                                                                                               \
                                                                                                                                                            \
        /* 20.5.6.2.1 NativeError.prototype, https://tc39.es/ecma262/#sec-nativeerror.prototype */                                                          \
        define_direct_property(vm.names.prototype, &PrototypeFromRealm, 0);                                                                                 \
                                                                                                                                                            \
        define_direct_property(vm.names.length, JS::Value(1), JS::Attribute::Configurable);                                                                 \
                                                                                                                                                            \
        return {};                                                                                                                                          \
    }                                                                                                                                                       \
                                                                                                                                                            \
    ConstructorName::~ConstructorName() = default;                                                                                                          \
                                                                                                                                                            \
    /* 20.5.6.1.1 NativeError ( message [ , options ] ), https://tc39.es/ecma262/#sec-nativeerror */                                                        \
    JS::ThrowCompletionOr<JS::Value> ConstructorName::call()                                                                                                \
    {                                                                                                                                                       \
        /* 1. If NewTarget is undefined, let newTarget be the active function object; else let newTarget be NewTarget. */                                   \
        return TRY(construct(*this));                                                                                                                       \
    }                                                                                                                                                       \
                                                                                                                                                            \
    /* 20.5.6.1.1 NativeError ( message [ , options ] ), https://tc39.es/ecma262/#sec-nativeerror */                                                        \
    JS::ThrowCompletionOr<JS::NonnullGCPtr<JS::Object>> ConstructorName::construct(JS::FunctionObject& new_target)                                          \
    {                                                                                                                                                       \
        auto& vm = this->vm();                                                                                                                              \
                                                                                                                                                            \
        auto message = vm.argument(0);                                                                                                                      \
        auto options = vm.argument(1);                                                                                                                      \
                                                                                                                                                            \
        /* 2. Let O be ? OrdinaryCreateFromConstructor(newTarget, "%NativeError.prototype%", « [[ErrorData]] »). */                                       \
        auto error = TRY(JS::ordinary_create_from_constructor<ClassName>(vm, new_target, DefaultPrototypeGetter));                                          \
                                                                                                                                                            \
        /* 3. If message is not undefined, then */                                                                                                          \
        if (!message.is_undefined()) {                                                                                                                      \
            /* a. Let msg be ? ToString(message). */                                                                                                        \
            auto msg = TRY(message.to_string(vm));                                                                                                          \
                                                                                                                                                            \
            /* b. Perform CreateNonEnumerableDataPropertyOrThrow(O, "message", msg). */                                                                     \
            error->create_non_enumerable_data_property_or_throw(vm.names.message, JS::PrimitiveString::create(vm, move(msg)));                              \
        }                                                                                                                                                   \
                                                                                                                                                            \
        /* 4. Perform ? InstallErrorCause(O, options). */                                                                                                   \
        TRY(error->install_error_cause(options));                                                                                                           \
                                                                                                                                                            \
        /* 5. Return O. */                                                                                                                                  \
        return error;                                                                                                                                       \
    }

}
