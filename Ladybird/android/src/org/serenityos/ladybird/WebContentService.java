/**
 * Copyright (c) 2022, Andrew Kaster <akaster@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

package org.serenityos.ladybird;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Parcel;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import org.qtproject.qt.android.bindings.QtService;

public class WebContentService extends QtService
{
    private static final String LOG_TAG = "WebContentService";

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(LOG_TAG, "Creating Service");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Log.i(LOG_TAG, "Destroying Service");
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        int ret = super.onStartCommand(intent, flags, startId);

        // Note: QtService will interact with our QAndroidService object in WebContent main
        return ret;
    }

    public static Bundle bundleFileDescriptors(int nativeIpcFd, int nativeFdPassingFd)
    {
        Bundle initialParamsBundle = new Bundle();

        Log.d(LOG_TAG, "parcelling " + nativeIpcFd);
        ParcelFileDescriptor ipc = ParcelFileDescriptor.adoptFd(nativeIpcFd);
        Log.d(LOG_TAG, "parcelling " + nativeFdPassingFd);
        ParcelFileDescriptor fd_pass = ParcelFileDescriptor.adoptFd(nativeFdPassingFd);
        Log.d(LOG_TAG, "Sending....");

        initialParamsBundle.putParcelable("IPC_SOCKET", ipc);
        initialParamsBundle.putParcelable("FD_PASSING_SOCKET", fd_pass);

        return initialParamsBundle;
    }
}
