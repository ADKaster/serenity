/*
 * Copyright (c) 2018-2021, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <AK/ScopeGuard.h>
#include <AK/String.h>
#include <LibCore/MappedFile.h>
#include <LibCore/System.h>
#include <fcntl.h>

#ifndef AK_OS_WINDOWS
#include <sys/mman.h>
#include <unistd.h>

namespace Core {

ErrorOr<NonnullRefPtr<MappedFile>> MappedFile::map(StringView path)
{
    auto fd = TRY(Core::System::open(path, O_RDONLY | O_CLOEXEC, 0));
    return map_from_fd_and_close(fd, path);
}

ErrorOr<NonnullRefPtr<MappedFile>> MappedFile::map_from_fd_and_close(int fd, StringView path)
{
    TRY(Core::System::fcntl(fd, F_SETFD, FD_CLOEXEC));

    ScopeGuard fd_close_guard = [fd] {
        close(fd);
    };

    auto stat = TRY(Core::System::fstat(fd));
    auto size = stat.st_size;

    auto* ptr = TRY(Core::System::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0, 0, path));

    return adopt_ref(*new MappedFile(ptr, size));
}

MappedFile::MappedFile(void* ptr, size_t size)
    : m_data(ptr)
    , m_size(size)
{
}

MappedFile::~MappedFile()
{
    MUST(Core::System::munmap(m_data, m_size));
}

#else // ^^^ Unix vvv Windows
#include <windows.h>
#include <io.h>

namespace Core {

ErrorOr<NonnullRefPtr<MappedFile>> MappedFile::map(StringView path)
{
    auto fd = TRY(Core::System::open(path, O_RDONLY, 0)); 
    return map_from_fd_and_close(fd, path);
}

ErrorOr<NonnullRefPtr<MappedFile>> MappedFile::map_from_fd_and_close(int fd, StringView path)
{
    ScopeGuard fd_close_guard = [fd] {
        close(fd);
    };

    HANDLE file_handle = reinterpret_cast<HANDLE>(_get_osfhandle(fd));

    VERIFY(file_handle != INVALID_HANDLE_VALUE);

    LARGE_INTEGER file_size = {};
    GetFileSizeEx(file_handle, &file_size);
    size_t size = static_cast<size_t>(file_size.QuadPart);

    HANDLE mapping_handle = CreateFileMapping(file_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (mapping_handle == nullptr) {
        return Error::from_errno(GetLastError());
    }
    ScopeGuard mapping_close_guard = [mapping_handle] {
        CloseHandle(mapping_handle);
    };

    void* ptr = MapViewOfFile(mapping_handle, FILE_MAP_READ, 0, 0, 0);
    if (ptr == nullptr) {
        return Error::from_errno(GetLastError());
    }

    return adopt_ref(*new MappedFile(ptr, size));
}

MappedFile::MappedFile(void* ptr, size_t size)
    : m_data(ptr)
    , m_size(size)
{
}

MappedFile::~MappedFile()
{
    BOOL did_unmap = UnmapViewOfFile(m_data);
    VERIFY(did_unmap != 0);
}

#endif

}


