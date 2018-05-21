###TL;DR

# Play with AOSP Mockware HAL and Customized Linux Kernel

Here we will play with Android Open Source Project (AOSP) and Linux Kernel (Goldfish) via a mock hardware device, 
named as "mockware" with a step by step guide.

### 0 Build AOSP and Kernel From Source Code

#### 0.0 Build AOSP
Make a partition with more than 150G. 
```
    cd ${ANDROID_BUILD_TOP}
    repo init -u https://android.googlesource.com/platform/manifest -b android-7.1.1_r9
    repo sync
    . build/envsetup.sh
    lunch aosp_arm-eng
    make -j4
```

Note: For those with slow network speed, please check highly compressed AOSP source code, e.g.,  https://forum.xda-developers.com/showpost.php?p=63435189&postcount=2

After a successful compliation, your will find the generated images as follows.
```
    out/target/product/generic/system.img
    out/target/product/generic/ramdisk.img
    out/target/product/generic/userdata.img
```

Test to execute the built emulator:
```		
	emulator -verbose -show-kernel -wipe-data 
```

Pay attention to the kernel message:
```
    Linux version 3.4.67-01650-gf1614b3 (jinqian@jinqian.mtv.corp.google.com) (gcc version 4.9 20150123 (prerelease) (GCC) )   #475 PREEMPT Fri Mar 11 15:59:17 PST 2016
    CPU: ARMv7 Processor [410fc080] revision 0 (ARMv7), cr=10c53c7d
```
This means we are using Linux kernel version 3.4.67.

#### 0.1 Build Linux Goldfish Kernel

Download Goldfish Linux Kernel, and check out the branch "android-goldfish-3.4" as mentioned above.
```
    git clone https://android.googlesource.com/kernel/goldfish.git
    git checkout -t origin/android-goldfish-3.4 -b goldfish-3.4
```

Now build the goldfish kernel.
```
    cd kernek/goldfish
    export ARCH=arm
    export CROSS_COMPILE=~/Documents/aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin/arm-eabi-
    export PATH=~/Documents/aosp/prebuilts/gcc/linux-x86/arm/arm-eabi-4.8/bin:$PATH
    make goldfish_armv7_defconfig
    make 
```

Try to test the new kernel in the emulator:
```
    emulator -verbose -show-kernel -kernel kernel/goldfish/arch/arm/boot/zImage
```

Now we finished the basic preparation.

### 1 Mockware Device Driver in Linux Kernel

The Mockware device creates a proc entry for reading/writing, implemented using the following code: 
```
    kernel/goldfish/drivers/mockware/mockware.c
```

To embedded the mockware, change drivers/Kconfig as follows:
```
    menu "Device Drivers"
    ...
    source "drivers/mockware/Kconfig"
    endmenu
```

Change drivers/Makefile, and add following line in the end of the file
```
    obj-$(CONFIG_MOCKWARE) += mockware/
```

Now build the kernel:
```
    cd kernel/goldfish
    make mrproper
    make goldfish_armv7_defconfig
    make menuconfig
    make 
```

Note: In "make menuconfig", Select "Device Drivers" => "Mockware Linux Driver".

The built kernel zImage should include the mockware. It can be verified as follows.
```	
    adb shell
	
	generic:/ # echo '333' > /proc/mockware                                        
	generic:/ # cat /proc/mockware                                                 
	333
    generic:/ #
```	

To verify programmatically, check external/mockware/mockware.c.
```
    croot
    mmm external/mockware
    make snod
    emulator -verbose -show-kernel -wipe-data -kernel kernel/goldfish/arch/arm/boot/zImage
```	

```
    adb shell
    generic:/ # /system/bin/mockware                                               
    Type in a short string to send to the mockware device:
    777
    Writing message to the device [777].
    Press ENTER to read back from the device...

    Reading from the device...
    The received message is: [777] readed=3
    generic:/ # 
```	
		  
### 2 Add Mockware Hardware Abstract Layer (HAL)

Check and copy the following implementation into ${ANDROID_BUILD_TOP}:
```
    hardware/libhardware/modules/mockware/Android.mk
    hardware/libhardware/modules/mockware/mockware.c
    hardware/libhardware/include/hardware/mockware.h
```

Need to define struct hw_module_t HAL_MODULE_INFO_SYM for system to open the mockware device:
```
    static struct hw_module_methods_t mockware_module_methods = {
	    .open = mockware_device_open
    };

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
```
  
Change hardware/libhardware/modules/Android.mk as follows.
```
    hardware_modules := gralloc hwcomposer audio nfc nfc-nci local_time \
      power usbaudio audio_remote_submix camera usbcamera consumerir sensors vibrator \
      tv_input fingerprint input vehicle thermal vr mockware
    include $(call all-named-subdir-makefiles,$(hardware_modules))
```

Compile and check the result:
```
    mmm hardware/libhardware/modules/mockware 
    ls -l out/target/product/generic/system/lib/hw/mockware.default.so
```

### 3 Add Mockware to Application Frameworks

#### 3.1 The “left” side of the Binder.

Create frameworks/base/core/java/android/mockware/ImockwareService.aidl:
```
    interface IMockwareService {
        void setVal(int val);
        int getVal();
    }
```
Register ImockwareService.aidl in build system, i.e., change frameworks/base/Android.mk as follows.
```
    LOCAL_SRC_FILES += \
	    ...
        core/java/android/mockware/ImockwareService.aidl \
```

Add Mockware Service Manager(frameworks/base/core/java/android/mockware/MockwareManager.java):
```
    
  public class MockwareManager {
      private final Context mContext;
      private final IMockwareService mService;
    
      public MockwareManager(Context ctx, IMockwareService service) {
          mContext = ctx;
          mService = service;
      }
	
      public void setVal(int val) {
          try {
              mService.setVal(val);
          } catch (RemoteException ex){
              Slog.e(TAG, "Unable to set value to the remote Mockware Service");
          }
      }
	    
      public int getVal() {
         ...
      }
  }
  
```

Add service name constant to the Context class (frameworks/base/core/java/android/content/Context.java) so clients can use it instead of hardcoded string:
```  

    /**
     * Use with {@link #getSystemService} to retrieve a {@link
     * android.mockware.mockwareManager} instance
     * @see #getSystemService
     */
    public static final String mockware_SERVICE = "mockware_service";
```

Register Mockware service manager instance in frameworks/base/core/java/android/app/SystemServiceRegistry.java, add the following lines:
```   

    ...
        registerService(Context.mockware_SERVICE, mockwareManager.class,
            new CachedServiceFetcher<mockwareManager>() {
                @Override   
                public mockwareManager createService(ContextImpl ctx) {
                    IBinder b = ServiceManager.getService(Context.mockware_SERVICE);
                    return new mockwareManager(ctx, ImockwareService.Stub.asInterface(b));
                }
            });
```

#### 3.2 The “right” side of the Binder.

Create Mockware service frameworks/base/services/core/java/com/android/server/mockware/MockwareService.java:
```
	public class MockwareService extends SystemService {
        ...
	    public MockwareService(Context context) {
			...
	        publishBinderService(context.MOCKWARE_SERVICE, mService);
	    }
        
    
	    /**
	     * Implementation of AIDL service interface
	     */
	    private final IBinder mService = new IMockwareService.Stub() {
	        @Override
	        public void setVal(int val) {
	            Slog.d(TAG, "Call setVal native service with val=" + val);
	            setVal_native(val);
	        }
		
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
```

Register service in System Server (frameworks/base/services/java/com/android/server/SystemServer.java), 
add two lines:
```
    import com.android.server.mockware.mockwareService;
    ...
	public final class SystemServer {
	    private void startOtherServices() {
		    ...	
            mSystemServerManager.startService(mockwareService.class);
			...
		}
	}		
```

Add JNI frameworks/base/services/core/jni/com_android_server_MockwareService.cpp to Access HAL:
```

	static inline int mockware_device_open(const hw_module_t* module, struct mockware_device_t** device) {
		return module->methods->open(module, MOCKWARE_HARDWARE_MODULE_ID, (struct hw_device_t**)device);
	}

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
```

Change: framwork/base/services/core/jni/onload.cpp, add two lines:
```

    register_android_server_MockwareService(JNIEnv *env);
    ...
    register_android_server_MockwareService(env);
```

Change: framwork/base/services/core/jni/Android.mk, add following line:
```

    (LOCAL_REL_DIR)/com_android_server_MockwareService.cpp \
```

### 4 Integrate All Together

As we have added mockware into Android frameworks, we need to update API:

```
    make update-api
```
Check frameworks/base/api/current.txt, which contains mockware specific definitions.

```

    adb remount
    mmm hardware/libhardware/modules/mockware
    adb push out/target/product/generic/system/lib/hw/mockware.default.so system/lib/hw/
   
    mmm frameworks/base/ 
    adb push out/target/product/generic/system/framework/framework.jar system/framework/

    mmm frameworks/base/services 
    adb push out/target/product/generic/system/framework/services.jar system/framework/ 
    adb push out/target/product/generic/system/lib/libandroid_servers.so system/lib/

    adb reboot

```

Note: It's also helpful to know the following commands:
```
    adb sync
    make snod
```

### 5 Test with a Sample Client.

Let's create a Mockware client application to test. The directory packages/experimental/mockware 
contains the implementataion.

```
    mmm packages/experimental/Mockware
    adb install -r out/target/product/generic/system/app/Mockware/Mockware.apk
```

Do not blame me, it does not work using 
```
    emulator -verbose -show-kernel -kernel kernel/goldfish/arch/arm/boot/zImage
```	
The log shows:
```
    05-19 04:23:31.922   224   939 V MockwareJNI: Mockware JNI: set value 5 to device.
    05-19 04:23:31.922   224   939 V MockwareJNI: Mockeare JNI: device is not open.
```

For the trouble-shooting, change the USE_FAKE_VALUE to 1 in hardware/libhardware/modules/mockware/mockware.c
Therefore we can test if it works above the HAL layer. Here we come the conclusion that there is interaction issue between customized kernel (mockware) and HAL. The root cause is SElinux is working in our AOSP android-7.1.1_r9 version. 

Before Android 5, the security policy is DAC (Discretionary Access Control): Access is provided based on user permission.

After Android 5 and above, SELinux Policy is adopted, i.e., MAC (Mandatory Access Control): Each program runs within a sandbox that limits its permissions.

To work around it, disable the SeLinux Ploicy as follows.
```
    emulator -verbose -show-kernel -selinux disabled -kernel kernel/goldfish/arch/arm/boot/zImage
```	
Now it works.

### 6 More about Android SELinux Policy

If you are still interested in SELinux, you need to search and add the cooresponeding policies. For example, if you execute:
```
    adb shell dmesg | grep avc
```
You may get following:
```
    ...
    type=1400 audit(1526868899.480:13): avc: denied { write } for pid=879 comm="system_server" name="mockware" dev="proc" ino=4026532483 scontext=u:r:system_server:s0 tcontext=u:object_r:proc:s0 tclass=file permissive=1
```

This means you need to and new entry to ${ANDROID_BUILD_TOP}/system/selinux/system_server.te
```
    allow system_server mockware_device:file { write };	
```	

In android 8.1 google added a new policy language called the Common Intermediate Language (CIL). I will not continue studying SELinux policy for Android 7 now.


Last but not least, play with AOSP and Linux Kernel and have fun!


### Reference

The following list some reference (most of them cannot be executed without update). 

1 https://www.linuxtopia.org/online_books/Linux_Kernel_Module_Programming_Guide/x814.html

2 http://devarea.com/aosp-adding-a-native-daemon/#.Wvc_aNMvzOQ

3 https://www.inovex.de/blog/migrating-an-embedded-android-setup-android-framework/

4 http://processors.wiki.ti.com/index.php/Android-Adding_SystemService

5 https://www.linaro.org/blog/adding-a-new-system-service-to-android-5-tips-and-how-to/

6 https://blog.lemberg.co.uk/introducing-new-service-android-7-navigation-bar-customization



### License
```
   Copyright (C) 2018 Bill Bao

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
```

