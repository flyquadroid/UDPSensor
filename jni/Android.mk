LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE 				:= io_quadroid_UDPSensor_ndk_QuadroidLib
LOCAL_SRC_FILES 			:= io_quadroid_UDPSensor_ndk_QuadroidLib.c
LOCAL_LDLIBS 				:= -L$(SYSROOT)/usr/lib -llog -landroid # NATIVE
LOCAL_EXPORT_CFLAGS += -g
# LOCAL_LDLIBS 				:= -llog -landroid -lEGL -lGLESv2 # NATIVE ACTIVITY
# LOCAL_STATIC_LIBRARIES 	:= android_native_app_glue # NATIVE ACTIVITY
include $(BUILD_SHARED_LIBRARY)
# $(call import-module,android/native_app_glue) # NATIVE ACTIVITY