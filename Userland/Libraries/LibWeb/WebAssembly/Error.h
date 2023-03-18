#pragma once

#include <LibJS/Runtime/Error.h>
#include <LibJS/Runtime/ErrorConstructor.h>
#include <LibJS/Runtime/ErrorPrototype.h>

namespace Web::WebAssembly {

#define WEBASSEMBLY_ENUMERATE_ERRORS                                                                  \
    /* 4.7. Error Objects https://webassembly.github.io/spec/js-api/#error-objects */                 \
    __JS_ENUMERATE(CompileError, compile_error, CompileErrorPrototype, CompileErrorConstructor, void) \
    __JS_ENUMERATE(RuntimeError, runtime_error, RuntimeErrorPrototype, RuntimeErrorConstructor, void) \
    __JS_ENUMERATE(LinkError, link_error, LinkErrorPrototype, LinkErrorConstructor, void)

#define WEBASSEMBLY_ENUMERATE_ERRORS_WITH_PROTOTYPE                                                                                                                                                                                                                                                                                \
    __JS_ENUMERATE_WITH_PROTOTYPE(CompileError, compile_error, CompileErrorPrototype, CompileErrorConstructor, void, (Web::Bindings::ensure_web_prototype<CompileErrorPrototype>(realm, "CompileError")), ([](JS::Realm& realm) { return (&Web::Bindings::ensure_web_prototype<CompileErrorPrototype>(realm, "CompileError")); })) \
    __JS_ENUMERATE_WITH_PROTOTYPE(RuntimeError, runtime_error, RuntimeErrorPrototype, RuntimeErrorConstructor, void, (Web::Bindings::ensure_web_prototype<RuntimeErrorPrototype>(realm, "RuntimeError")), ([](JS::Realm& realm) { return (&Web::Bindings::ensure_web_prototype<RuntimeErrorPrototype>(realm, "RuntimeError")); })) \
    __JS_ENUMERATE_WITH_PROTOTYPE(LinkError, link_error, LinkErrorPrototype, LinkErrorConstructor, void, (Web::Bindings::ensure_web_prototype<LinkErrorPrototype>(realm, "LinkError")), ([](JS::Realm& realm) { return (&Web::Bindings::ensure_web_prototype<LinkErrorPrototype>(realm, "LinkError")); }))

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR(ClassName, snake_name, PrototypeName, ConstructorName)
WEBASSEMBLY_ENUMERATE_ERRORS
#undef __JS_ENUMERATE

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR_PROTOTYPE(ClassName, snake_name, PrototypeName, ConstructorName)
WEBASSEMBLY_ENUMERATE_ERRORS
#undef __JS_ENUMERATE

#define __JS_ENUMERATE(ClassName, snake_name, PrototypeName, ConstructorName, ArrayType) \
    JS_DECLARE_NATIVE_ERROR_CONSTRUCTOR(ClassName, snake_name, PrototypeName, ConstructorName)
WEBASSEMBLY_ENUMERATE_ERRORS
#undef __JS_ENUMERATE

}
