set(AK_SOURCES
    Assertions.cpp
    Base64.cpp
    CircularBuffer.cpp
    ConstrainedStream.cpp
    CountingStream.cpp
    DOSPackedTime.cpp
    DeprecatedFlyString.cpp
    DeprecatedString.cpp
    Error.cpp
    FloatingPointStringConversions.cpp
    FlyString.cpp
    Format.cpp
    FuzzyMatch.cpp
    GenericLexer.cpp
    Hex.cpp
    JsonObject.cpp
    JsonParser.cpp
    JsonPath.cpp
    JsonValue.cpp
    LexicalPath.cpp
    MemoryStream.cpp
    NumberFormat.cpp
    OptionParser.cpp
    Random.cpp
    StackInfo.cpp
    Stream.cpp
    String.cpp
    StringBuilder.cpp
    StringFloatingPointConversions.cpp
    StringImpl.cpp
    StringUtils.cpp
    StringView.cpp
    Time.cpp
    URL.cpp
    URLParser.cpp
    UUID.cpp
    Utf16View.cpp
    Utf32View.cpp
    Utf8View.cpp
    kmalloc.cpp
)
set(RAW_AK_SOURCES ${AK_SOURCES})
# AK sources are included from many different places, such as the Kernel, LibC, and Loader
list(TRANSFORM AK_SOURCES PREPEND "${CMAKE_CURRENT_SOURCE_DIR}/")

set(AK_SOURCES ${AK_SOURCES} PARENT_SCOPE)

if (EXPERIMENTAL_MODULES)
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

    add_library(AK ${AK_SOURCES})
    target_sources(AK PUBLIC
      FILE_SET CXX_MODULES
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        ModuleInterface.cppm
    )
    target_compile_definitions(AK PRIVATE EXPORT_MODULE)
    target_link_directories(AK PRIVATE Threads::Threads)
endif()

serenity_install_headers(AK)
serenity_install_sources(AK)
