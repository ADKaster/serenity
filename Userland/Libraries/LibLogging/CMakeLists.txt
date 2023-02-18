set(SOURCES
    Logger.cpp
    SyslogClient.cpp
)

serenity_lib(LibLogging logging)
target_link_libraries(LibLogging PRIVATE LibConfig LibCore LibIPC LibTimeZone)
