
#include <LibJS/Heap/Heap.h>
#include <LibJS/Runtime/AbstractOperations.h>
#include <LibJS/Runtime/Realm.h>
#include <LibJS/Runtime/VM.h>
#include <LibWeb/Bindings/Intrinsics.h>
#include <LibWeb/WebAssembly/Error.h>

namespace Web::Bindings {
template<>
void Intrinsics::create_web_prototype_and_constructor<WebAssembly::CompileErrorPrototype>(JS::Realm& realm)
{
    auto& vm = realm.vm();

    auto prototype = heap().allocate<WebAssembly::CompileErrorPrototype>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_prototypes.set("CompileError"sv, prototype);

    auto constructor = heap().allocate<WebAssembly::CompileErrorConstructor>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_constructors.set("CompileError"sv, constructor);

    prototype->define_direct_property(vm.names.constructor, constructor.ptr(), JS::Attribute::Writable | JS::Attribute::Configurable);
    constructor->define_direct_property(vm.names.name, JS::PrimitiveString::create(vm, "CompileError"sv).release_allocated_value_but_fixme_should_propagate_errors(), JS::Attribute::Configurable);
}

template<>
void Intrinsics::create_web_prototype_and_constructor<WebAssembly::LinkErrorPrototype>(JS::Realm& realm)
{
    auto& vm = realm.vm();

    auto prototype = heap().allocate<WebAssembly::LinkErrorPrototype>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_prototypes.set("LinkError"sv, prototype);

    auto constructor = heap().allocate<WebAssembly::LinkErrorConstructor>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_constructors.set("LinkError"sv, constructor);

    prototype->define_direct_property(vm.names.constructor, constructor.ptr(), JS::Attribute::Writable | JS::Attribute::Configurable);
    constructor->define_direct_property(vm.names.name, JS::PrimitiveString::create(vm, "LinkError"sv).release_allocated_value_but_fixme_should_propagate_errors(), JS::Attribute::Configurable);
}

template<>
void Intrinsics::create_web_prototype_and_constructor<WebAssembly::RuntimeErrorPrototype>(JS::Realm& realm)
{
    auto& vm = realm.vm();

    auto prototype = heap().allocate<WebAssembly::RuntimeErrorPrototype>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_prototypes.set("RuntimeError"sv, prototype);

    auto constructor = heap().allocate<WebAssembly::RuntimeErrorConstructor>(realm, realm).release_allocated_value_but_fixme_should_propagate_errors();
    m_constructors.set("RuntimeError"sv, constructor);

    prototype->define_direct_property(vm.names.constructor, constructor.ptr(), JS::Attribute::Writable | JS::Attribute::Configurable);
    constructor->define_direct_property(vm.names.name, JS::PrimitiveString::create(vm, "RuntimeError"sv).release_allocated_value_but_fixme_should_propagate_errors(), JS::Attribute::Configurable);
}
}

namespace Web::WebAssembly {
#define __JS_ENUMERATE_WITH_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeFromRealm, DefaultPrototypeGetter) \
    JS_IMPLEMENT_NATIVE_ERROR(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeFromRealm)
WEBASSEMBLY_ENUMERATE_ERRORS_WITH_PROTOTYPE
#undef __JS_ENUMERATE_WITH_PROTOTYPE

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_IMPLEMENT_NATIVE_ERROR_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType)
WEBASSEMBLY_ENUMERATE_ERRORS
#undef __JS_ENUMERATE

#define __JS_ENUMERATE_WITH_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeFromRealm, DefaultPrototypeGetter) \
    JS_IMPLEMENT_NATIVE_ERROR_CONSTRUCTOR(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType, PrototypeFromRealm, DefaultPrototypeGetter)

WEBASSEMBLY_ENUMERATE_ERRORS_WITH_PROTOTYPE
#undef __JS_ENUMERATE

}
