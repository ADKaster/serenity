serenity_install_sources("Userland/Libraries/LibTest")

set(SOURCES
    TestSuite.cpp
    CrashTest.cpp
)

serenity_lib(LibTest test)
target_link_libraries(LibTest LibC)
