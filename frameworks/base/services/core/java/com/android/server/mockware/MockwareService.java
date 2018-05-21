/*
 * Copyright (C) 2018 Bill Bao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.server.mockware;

import android.util.Log;
import android.util.Slog;

import android.os.IBinder;
import android.content.Context;

import android.mockware.IMockwareService;
import com.android.server.SystemService;

public class MockwareService extends SystemService {
    private final Context mContext;
    private static final String TAG = "MockwareService";

    public MockwareService(Context context) {
        super(context);

        Slog.d(TAG, "Build service");
        mContext = context;
        publishBinderService(context.MOCKWARE_SERVICE, mService);
    }
    
    /**
     * Called when service is started by the main system service
     */
    @Override
    public void onStart() {
        Slog.d(TAG, "Start service");
        init_native();  
    }
    
    /**
     * Implementation of AIDL service interface
     */
    private final IBinder mService = new IMockwareService.Stub() {
        /**
         * Implementation of the setVal() described in AIDL interface
         */
        @Override
        public void setVal(int val) {
            Slog.d(TAG, "Call setVal native service with val=" + val);
            setVal_native(val);
        }
		
        /**
         * Implementation of the getVal() described in AIDL interface
         */
        @Override
        public int getVal() {
            Slog.d(TAG, "Call getVal native service");
            int val = getVal_native();
            Slog.d(TAG, "Call getVal native service with value=" + val);
			return val;
        }
    };
    
    /* Native functions declarations */
    private static native boolean init_native();
	private static native void setVal_native(int val);
    private static native int getVal_native();

}