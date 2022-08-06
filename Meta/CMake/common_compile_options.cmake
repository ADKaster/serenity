set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    # -Wall == -Weverything for clang-cl, see https://reviews.llvm.org/D40603
    add_compile_options(/W4)
    add_compile_options(/permissive-)
    add_compile_options(/utf-8)
    add_compile_options(-Wno-unknown-attributes) # [[no_unique_address]] is completely broken on windows
else()
    add_compile_options(-Wall)
    add_compile_options(-Wextra)
endif()

if (NOT CMAKE_HOST_SYSTEM_NAME MATCHES SerenityOS)
    # FIXME: Something makes this go crazy and flag unused variables that aren't flagged as such when building with the toolchain.
    #        Disable -Werror for now.
    add_compile_options(-Werror)
endif()
