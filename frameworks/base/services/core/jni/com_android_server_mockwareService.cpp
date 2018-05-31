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

#define LOG_TAG "MockwareJNI"

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"
#include <utils/misc.h>
#include <utils/Log.h>
#include <hardware/mockware.h>
#include <stdio.h>

namespace android
{

/**
 * mockware device defined in <hardware/mockware.h> 
 */
struct mockware_device_t *mockware_device;

/**
 * set value to mackware hardware
 */
static void mockware_setVal(JNIEnv* env, jobject clazz, jint value) {
    int val = value;
    ALOGI("Mockware JNI: set value %d to device.", val);
    if (!mockware_device) {
        ALOGI("Mockeare JNI: device is not open.");
        return;
    }
    mockware_device->set_val(mockware_device, val);
}

/**
 * get value from mackware hardware 
 */
static jint mockware_getVal(JNIEnv* env, jobject clazz) {
    int val = 0;
    if (!mockware_device) {
        ALOGI("Mockware JNI: device is not open.");
        return val;
    }
    mockware_device->get_val(mockware_device, &val);
    ALOGI("Mockware JNI: get value %d from device.", val);
    return val;
}

static inline int mockware_device_open(const hw_module_t* module, struct mockware_device_t** device) {
    return module->methods->open(module, MOCKWARE_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
}

/**
 * JNI Layer init function;
 */
static jboolean mockware_init(JNIEnv* env, jclass clazz) {
    mockware_module_t* module;
    
    ALOGI("Mockware JNI: initializing......");
    if (hw_get_module(MOCKWARE_HARDWARE_MODULE_ID, (const struct hw_module_t**)&module) == 0) {
        ALOGI("Mockware JNI: mockware Stub has found.");
        if (mockware_device_open(&(module->common), &mockware_device) == 0) {
            ALOGI("Mockware JNI: mockware device is opened.");
            return 1;
        }
        ALOGE("Mockware JNI: failed to open mockware device.");
        return 0;
    }
    ALOGE("Mockware JNI: failed to get mockware stub module.");
    return 0;        
}

/**
 * JNI method table
 */
static JNINativeMethod method_table[] = {
    {"init_native", "()Z", (void*)mockware_init},
    {"setVal_native", "(I)V", (void*)mockware_setVal},
    {"getVal_native", "()I", (void*)mockware_getVal},
};

/* register JNI methods */
int register_android_server_MockwareService(JNIEnv *env)
{
    return jniRegisterNativeMethods(env, "com/android/server/mockware/MockwareService",
            method_table, NELEM(method_table));
}

};  // end of android name space
