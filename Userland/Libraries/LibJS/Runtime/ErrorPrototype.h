/*
 * Copyright (c) 2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021-2022, Linus Groh <linusg@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/PrototypeObject.h>

namespace JS {

class ErrorPrototype final : public PrototypeObject<ErrorPrototype, Error> {
    JS_PROTOTYPE_OBJECT(ErrorPrototype, Error, Error);

public:
    virtual ThrowCompletionOr<void> initialize(Realm&) override;
    virtual ~ErrorPrototype() override = default;

private:
    explicit ErrorPrototype(Realm&);

    JS_DECLARE_NATIVE_FUNCTION(to_string);
    JS_DECLARE_NATIVE_FUNCTION(stack_getter);
    JS_DECLARE_NATIVE_FUNCTION(stack_setter);
};

#define JS_DECLARE_NATIVE_ERROR_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName) \
    class PrototypeName final : public JS::PrototypeObject<PrototypeName, ClassName> {           \
        JS_PROTOTYPE_OBJECT(PrototypeName, ClassName, ClassName);                                \
                                                                                                 \
    public:                                                                                      \
        virtual JS::ThrowCompletionOr<void> initialize(JS::Realm&) override;                     \
        virtual ~PrototypeName() override = default;                                             \
                                                                                                 \
    private:                                                                                     \
        explicit PrototypeName(JS::Realm&);                                                      \
    };

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName)
JS_ENUMERATE_NATIVE_ERRORS
#undef __JS_ENUMERATE

}

// NOTE: This is in the header so that extension specs can define their own native errors
#define JS_IMPLEMENT_NATIVE_ERROR_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType)            \
    PrototypeName::PrototypeName(JS::Realm& realm)                                                                       \
        : PrototypeObject(*realm.intrinsics().error_prototype())                                                         \
    {                                                                                                                    \
    }                                                                                                                    \
                                                                                                                         \
    JS::ThrowCompletionOr<void> PrototypeName::initialize(JS::Realm& realm)                                              \
    {                                                                                                                    \
        auto& vm = this->vm();                                                                                           \
        MUST_OR_THROW_OOM(Base::initialize(realm));                                                                      \
        u8 attr = JS::Attribute::Writable | JS::Attribute::Configurable;                                                 \
        define_direct_property(vm.names.name, MUST_OR_THROW_OOM(JS::PrimitiveString::create(vm, #ClassName##sv)), attr); \
        define_direct_property(vm.names.message, JS::PrimitiveString::create(vm, String {}), attr);                      \
                                                                                                                         \
        return {};                                                                                                       \
    }
