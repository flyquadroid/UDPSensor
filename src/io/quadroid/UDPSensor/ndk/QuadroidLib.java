package io.quadroid.UDPSensor.ndk;

import java.lang.System;

/**
 * UDPSensor
 * User: Matthias Nagel
 * Date: 26.11.13
 * http://www.quadroid.io
 */
public class QuadroidLib {

    public native static void init();

    static {
        System.loadLibrary("io_quadroid_UDPSensor_ndk_QuadroidLib");
    }
}