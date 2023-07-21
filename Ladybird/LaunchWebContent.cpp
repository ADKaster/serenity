/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "LaunchWebContent.h"

#ifdef AK_OS_ANDROID
#    include "WebContentServiceAndroid.h"
#endif

ErrorOr<WebContentResult> launch_web_content_process(WebView::ViewImplementation& view,
    ReadonlySpan<String> candidate_web_content_paths,
    WebView::EnableCallgrindProfiling enable_callgrind_profiling,
    WebView::IsLayoutTestMode is_layout_test_mode,
    WebView::UseJavaScriptBytecode use_javascript_bytecode)
{
    int socket_fds[2] {};
    TRY(Core::System::socketpair(AF_LOCAL, SOCK_STREAM, 0, socket_fds));

    int ui_fd = socket_fds[0];
    int wc_fd = socket_fds[1];

    int fd_passing_socket_fds[2] {};
    TRY(Core::System::socketpair(AF_LOCAL, SOCK_STREAM, 0, fd_passing_socket_fds));

    qDebug() << "WebContent sockets: " << socket_fds[0] << ":" << socket_fds[1];
    qDebug() << "    WebContent FD sockets: " << fd_passing_socket_fds[0] << ":" << fd_passing_socket_fds[1];

    int ui_fd_passing_fd = fd_passing_socket_fds[0];
    int wc_fd_passing_fd = fd_passing_socket_fds[1];

    OwnPtr<WebView::ViewImplementation::OSPrivateState> os_private;
#ifdef AK_OS_ANDROID
    (void)candidate_web_content_paths;
    (void)enable_callgrind_profiling;
    (void)is_layout_test_mode;
    (void)use_javascript_bytecode;
    // NOTE: Java will close the fds by wrapping them a ParcelFileDescriptors
    os_private = WebContentServiceAndroid::the().add_client(wc_fd, wc_fd_passing_fd);
    qDebug() << "Created WebContent Service client";
#else
    if (auto child_pid = TRY(Core::System::fork()); child_pid == 0) {
        TRY(Core::System::close(ui_fd_passing_fd));
        TRY(Core::System::close(ui_fd));

        auto takeover_string = TRY(String::formatted("WebContent:{}", wc_fd));
        TRY(Core::System::setenv("SOCKET_TAKEOVER"sv, takeover_string, true));

        auto webcontent_fd_passing_socket_string = TRY(String::number(wc_fd_passing_fd));

        ErrorOr<void> result;
        for (auto const& path : candidate_web_content_paths) {
            constexpr auto callgrind_prefix_length = 3;

            if (Core::System::access(path, X_OK).is_error())
                continue;

            auto arguments = Vector {
                "valgrind"sv,
                "--tool=callgrind"sv,
                "--instr-atstart=no"sv,
                path.bytes_as_string_view(),
                "--webcontent-fd-passing-socket"sv,
                webcontent_fd_passing_socket_string
            };
            if (enable_callgrind_profiling == WebView::EnableCallgrindProfiling::No)
                arguments.remove(0, callgrind_prefix_length);
            if (is_layout_test_mode == WebView::IsLayoutTestMode::Yes)
                arguments.append("--layout-test-mode"sv);
            if (use_javascript_bytecode == WebView::UseJavaScriptBytecode::Yes)
                arguments.append("--use-bytecode"sv);

            result = Core::System::exec(arguments[0], arguments.span(), Core::System::SearchInPath::Yes);
            if (!result.is_error())
                break;
        }

        if (result.is_error())
            warnln("Could not launch any of {}: {}", candidate_web_content_paths, result.error());
        VERIFY_NOT_REACHED();
    }

    TRY(Core::System::close(wc_fd_passing_fd));
    TRY(Core::System::close(wc_fd));
#endif

    auto socket = TRY(Core::LocalSocket::adopt_fd(ui_fd));
    TRY(socket->set_blocking(true));

    auto new_client = TRY(adopt_nonnull_ref_or_enomem(new (nothrow) WebView::WebContentClient(move(socket), view)));
    new_client->set_fd_passing_socket(TRY(Core::LocalSocket::adopt_fd(ui_fd_passing_fd)));

    if (enable_callgrind_profiling == WebView::EnableCallgrindProfiling::Yes) {
        dbgln();
        dbgln("\033[1;45mLaunched WebContent process under callgrind!\033[0m");
        dbgln("\033[100mRun `\033[4mcallgrind_control -i on\033[24m` to start instrumentation and `\033[4mcallgrind_control -i off\033[24m` stop it again.\033[0m");
        dbgln();
    }

    return WebContentResult { move(new_client), move(os_private) };
}
