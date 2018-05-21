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

#ifndef __ANDROID_MOCKWARE_HW_INTERFACE__
#define __ANDROID_MOCKWARE_HW_INTERFACE__

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/hardware.h>

__BEGIN_DECLS
#define MOCKWARE_HARDWARE_MODULE_ID "mockware"

/**
 * The mockware module structure.
 */	
struct mockware_module_t {
	struct hw_module_t common;
};

/**
 * The mockware device description structure;
 * First field must be the hw_device_t field;
 * Other fields can be function pointers and othe exported fields
 */
struct mockware_device_t {
    /* Will be used in HAL open method */
    struct hw_device_t common;

    int fd;
    /* Pointers to your HAL functions */
    int (*set_val)(struct mockware_device_t* dev, int val);
    int (*get_val)(struct mockware_device_t* dev, int* val);
};
__END_DECLS
#endif //__ANDROID_MOCKWARE_HW_INTERFACE__
