/*
 * Copyright (c) 2023, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include <AK/Error.h>
#include <AK/OwnPtr.h>
#include <AK/Span.h>
#include <AK/StringView.h>

#include <LibWebView/ViewImplementation.h>
#include <LibWebView/WebContentClient.h>

struct WebContentResult {
    NonnullRefPtr<WebView::WebContentClient> client;
    OwnPtr<WebView::ViewImplementation::OSPrivateState> os_private;
};

ErrorOr<WebContentResult> launch_web_content_process(WebView::ViewImplementation& view,
    ReadonlySpan<String> candidate_web_content_paths,
    WebView::EnableCallgrindProfiling,
    WebView::IsLayoutTestMode,
    WebView::UseJavaScriptBytecode);
