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

package android.mockware;

import android.mockware.IMockwareService;
import android.util.AndroidException;
import android.util.Log;
import android.util.Slog;
import android.content.Context;
import android.os.RemoteException;

/**
 * The Manager class;
 * The public methods of this class will be part of the new system API
 */
public class MockwareManager {
    private static final String TAG = "MockwareManager";
    private final Context mContext;
    private final IMockwareService mService;
    
    /**
     * Construct the remote service and execution context
     * 
     * @param ctx
     * @param service
     */
    public MockwareManager(Context ctx, IMockwareService service) {
        mContext = ctx;
        mService = service;
    }
    
    /**
     * Set value to the remote service
     */
    public void setVal(int val) {
        Slog.d(TAG, "mService.setVal(" + val + ")");
        try {
            mService.setVal(val);
        } catch (RemoteException ex){
            Slog.e(TAG, "Unable to set value to the remote Mockware Service");
        }
    }
        
    /**
     * Get value from the remote service
     */
    public int getVal() {
        int ret = 0;
        try {
            ret = mService.getVal();
        } catch (RemoteException ex){
            Slog.e(TAG, "Unable to get value from the remote Mockware Service");
        } finally {
            Slog.d(TAG, "mService.geVal() with val=" + ret);
            return ret;
        }
    }
}