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

#include <hardware/mockware.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/atomic.h>
#include <stdio.h>

#define DEVICE_NAME "/proc/mockware"

#define MODULE_NAME "Mockware"
#define MODULE_AUTHOR "baoganfeng@gmail.com"
#define BUFFER_LENGTH 256 

/* mockware device open / close */
static int mockware_device_open(const struct hw_module_t* module, const char* name, struct hw_device_t** device);
static int mockware_device_close(struct hw_device_t* device);

/* mockware device access */
static int mockware_device_set(struct mockware_device_t* dev, int val);
static int mockware_device_get(struct mockware_device_t* dev, int* val);

/* If define USE_FAKE_VALUE as 1, the framework will use a fake value instead of really call read/write to 
 * mockware device. This is introduced for debug purpose.
 * Define USE_FAKE_VALUE as 0 to really contact mockware device.
 */
#define USE_FAKE_VALUE 0

static int fake_value;
    
/* module methods */
static struct hw_module_methods_t mockware_module_methods = {
    .open = mockware_device_open
};

/* HAL_MODULE_INFO_SYM */
struct mockware_module_t HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .version_major = 1,
        .version_minor = 0,
        .id = MOCKWARE_HARDWARE_MODULE_ID,
        .name = MODULE_NAME,
        .author = MODULE_AUTHOR,
        .methods = &mockware_module_methods,
    }
};

/**
 * A pointer to this method is stored in hw_module_method_t.open.
 *
 * Once JNI loads the hw_module_method_t symbol, it
 * can call this function and "open" the HAL layer
 * receiving pointers to this module's additional methods.
 * This is mandatory, and part of hw_device_t 
 */
static int mockware_device_open(const struct hw_module_t* module, 
                                const char* name, 
                                struct hw_device_t** device) {
    struct mockware_device_t* dev;
    dev = (struct mockware_device_t*)malloc(sizeof(struct mockware_device_t));
    if (!dev) {
        ALOGE("Mockware-HAL: failed to alloc space");
        return -EFAULT;
    }
    memset(dev, 0, sizeof(struct mockware_device_t));
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (hw_module_t*)module;
    dev->common.close = mockware_device_close;
    dev->set_val = mockware_device_set;
    dev->get_val = mockware_device_get;
    *device = &(dev->common);
    
    if (USE_FAKE_VALUE) {
        ALOGD("Mockware-HAL: open fake device successfully.");    
        return 0;
    }        
    if ((dev->fd = open(DEVICE_NAME, O_RDWR)) == -1) {
        ALOGD("Mockware-HAL: failed to open %s -- %s.", DEVICE_NAME, strerror(errno));
    } else {
        ALOGD("Mockware-HAL: open %s successfully.", DEVICE_NAME);        
    }
    return 0;
}

/** This is mandatory, and part of hw_device_t */
static int mockware_device_close(struct hw_device_t* device) {
    struct mockware_device_t* mockware_device = (struct mockware_device_t*)device;
    if (mockware_device) {
        if (mockware_device->fd) {
            close(mockware_device->fd);            
        }
        free(mockware_device);
    }
    return 0;
}

/** Mockware device internal set value implementation */
static int mockware_device_set(struct mockware_device_t* dev, int val) {
    if (USE_FAKE_VALUE) {
        ALOGD("Mockware-HAL: set with fake device successfully, val=%d", val);    
        fake_value = val;
        return 0;
    }
    
    if (dev->fd == -1) {
        ALOGE("Mockware-HAL: error device %s is not opened", DEVICE_NAME);
        return -EFAULT;
    }

    char valStr[BUFFER_LENGTH];
    sprintf(valStr, "%d\n", val);      
    write(dev->fd, valStr, strlen(valStr));

    ALOGD("Mockware-HAL: set value %d to device fd=%d", val, dev->fd);
        
    return 0;
}

/** Mockware device internal get value implementation */
static int mockware_device_get(struct mockware_device_t* dev, int* val) {
    if (!val) {
        ALOGE("Mockware-HAL: error val pointer");
        return -EFAULT;
    }

    if (USE_FAKE_VALUE) {
        ALOGD("Mockware-HAL: get with fake device successfully, val=%d", fake_value);    
        *val = fake_value;
        return 0;
    }

    if (dev->fd == -1) {
        ALOGE("Mockware-HAL: error device %s is not opened", DEVICE_NAME);
        return -EFAULT;    
    }

    char valStr[BUFFER_LENGTH];
    int ret;
    ret = read(dev->fd, valStr, BUFFER_LENGTH);     // Read the response from the mockware device
    if (ret <= 0) {
        // Read again after failed. If you read the kernel mockware implementation procfs_read(),
        // you will understand it's necessary to read double time to ensure really get something
        // which has been ever written.
        ret = read(dev->fd, valStr, BUFFER_LENGTH);    
        if (ret <= 0 ) {
            ALOGE("Mockware-HAL: Failed to read the data from %s", DEVICE_NAME);
            return -EFAULT;            
        }
    }
    sscanf(valStr, "%d", val);
    
    ALOGD("Mockware-HAL: get value %d from device fd=%d", *val, dev->fd);
    return 0;
}

