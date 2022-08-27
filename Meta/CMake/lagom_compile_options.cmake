include(${CMAKE_CURRENT_LIST_DIR}/common_compile_options.cmake)

add_compile_options(-Wno-implicit-const-int-float-conversion)
add_compile_options(-Wno-literal-suffix)
add_compile_options(-Wno-maybe-uninitialized)
add_compile_options(-Wno-unknown-warning-option)
# MSVC check is for clang-cl.exe
if (MSVC)
    add_compile_options(-Wno-expansion-to-defined)  # FIXME: Complain to thakis about this one
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS) # Go away with your _s replacements
    add_compile_definitions(_CRT_NONSTDC_NO_WARNINGS) # POSIX names are just fine, thanks
    add_compile_definitions(NOMINMAX) # ...really?
    add_compile_definitions(NAME_MAX=255)
else()
    add_compile_options(-fdiagnostics-color=always)
    add_compile_options(-fsigned-char)
    add_compile_options(-fno-exceptions)
    add_compile_options(-fPIC -g)
    add_compile_options(-O2)
endif()
if (NOT ENABLE_FUZZERS AND NOT APPLE AND NOT MSVC)
    add_compile_options(-fno-semantic-interposition)
endif()
