#include "io_quadroid_UDPSensor_ndk_QuadroidLib.h"

#include <jni.h>
#include <string.h>
#include <math.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android/looper.h>
#include <time.h>

#define DEBUG 0
#define LOOPER_ID 1
#define SAMP_PER_SEC 2
#define LOG 0
#define LOG_SIZE 202

#define NDK2SDK_DEBUG 1

#define USE_ACCELEROMETER 1
#define USE_GYROSCOPE 1
#define USE_MAGNETIC 1
#define USE_DISTANCE 0
#define USE_PRESSURE 1

#define TAG "io_quadroid_UDPSensor_ndk_QuadroidLib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)

// Local Position ----------------------------

float acce[3] = { 0.0f, 0.0f, 0.0f };			// Accelerometer
float acceRaw[3] = { 0.0f, 0.0f, 0.0f };
float gyro[3] = { 0.0f, 0.0f, 0.0f };			// Gyroscope
float gyroRaw[3] = { 0.0f, 0.0f, 0.0f };
float magn[3] = { 0.0f, 0.0f, 0.0f };			// Magnetic field
float pressure = 0.0f;							// Pressure
float magnRaw[3] = { 0.0f, 0.0f, 0.0f };
float altitude = 0.0f;							// Altitude
float distance = 0.0f;							// Distance
double quat[4] = { 0.0d, 0.0d, 0.0d, 0.0d };	// Quaternion
float motor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };	// Motors

double q[4] = {1.0f, 0.0f, 0.0f, 0.0f};
double In[3] = {0.0f, 0.0f, 0.0f};

long long timestamp = 0;

// Client/Remote -----------------------------

// float cTime = 0.0f;								// Timestamp
double cQuat[4] = { 0.0d, 0.0d, 0.0d, 0.0d };	// Quaternion
float cAltitude = 0.0f;								// Altitude

// Logging -----------------------------------

long loog[LOG_SIZE];
int log_i = 0;
int log_j = 1;
int log_output = 0;
int log_acce = 0;
int log_gyro = 0;
int log_magn = 0;


// Runtime variables -------------------------

ASensorEventQueue* queue;
ASensorManager* sensorManager;
ALooper* looper;
ASensorRef acceSensor;
ASensorRef gyroSensor;
ASensorRef magnSensor;
ASensorRef proxSensor;
ASensorRef pressureSensor;
int events;
long long currentTime = 0;

JavaVM* mJvm;
JNIEnv* mEnv;
jclass mClass;
jmethodID mMethod;

jint JNI_OnLoad(JavaVM* vm, void* reserved){
	mJvm = vm;

	if(NDK2SDK_DEBUG==1){
		LOGI("JNIOnLoad");
	}

    if((*vm)->GetEnv(vm, (void**)&mEnv, JNI_VERSION_1_6)){
    	if(NDK2SDK_DEBUG==1){
    		LOGI("JNIOnLoad: Could not get JNIEnv*");
    	}
        return JNI_ERR;
    }

    jclass temp = mEnv->FindClass("io/quadroid/UDPSensor/main/MainActivity");
    mClass = (jclass)mEnv->NewGlobalRef(temp);
    if(mClass == NULL){
    	if(NDK2SDK_DEBUG==1){
    		LOGI("JNIOnLoad: Could not get java class");
    	}
        return JNI_ERR;
    } else {
    	if(NDK2SDK_DEBUG==1){
    		LOGI("JNIOnLoad: Could get java class");
    	}
    }

    mMethod = (*mEnv)->GetStaticMethodID(mEnv, mClass, "onValuesEventChanged", "(JFFFFFFFFFF)V");
    if(mMethod == NULL){
    	if(NDK2SDK_DEBUG==1){
    		LOGI("JNIOnLoad: Could not get method identifier");
    	}
        return JNI_ERR;
    } else {
    	if(NDK2SDK_DEBUG==1){
    		LOGI("JNIOnLoad: Could get method identifier");
    	}
    }

    if(NDK2SDK_DEBUG==1){
    	LOGI("JNIOnLoad: JNI Version: %d", JNI_VERSION_1_6);
    }

	return JNI_VERSION_1_6;
}

void sendValues(){
	(*mEnv)->CallStaticVoidMethod(mEnv, mClass, mMethod, timestamp, acceRaw[0], acceRaw[1], acceRaw[2], gyroRaw[0], gyroRaw[1], gyroRaw[2], magnRaw[0], magnRaw[1], magnRaw[2], pressure);
}

/*
 * Class:     io_quadroid_ndk_QuadroidLib
 * Method:    get_sensor_events
 * Description: callback function for the sensor manager
 */
static int get_sensor_events(int fd, int events, void* data) {

	/*
	typedef struct ASensorVector {
		union {
			float v[3];
			struct {
				float x;
				float y;
				float z;
			}
			struct {
				float azimuth;
				float pitch;
				float roll;
			};
		};
		int8_t status;
		uint8_t reserved[3];
	} ASensorVector;

	typedef struct ASensorEvent {
		int32_t version;  sizeof(struct ASensorEvent)
		int32_t sensor;
		int32_t type;
		int32_t reserved0;
		int64_t timestamp;
		union {
			float           data[16];
			ASensorVector   vector;
			ASensorVector   acceleration;
			ASensorVector   magnetic;
			float           temperature;
			float           distance;
			float           light;
			float           pressure;
		};
		int32_t reserved1[4];
	} ASensorEvent;
	*/
	ASensorEvent event;

	while (ASensorEventQueue_getEvents(queue, &event, 1) > 0) {

		switch (event.type) {
		    case 6: // PRESSURE
        				pressure = event.pressure;

        				if(DEBUG==1){
        					LOGI("pressure: %f", pressure);
        				}
        				break;
			case ASENSOR_TYPE_ACCELEROMETER:
				acce[0] = event.acceleration.roll;
				acce[1] = event.acceleration.pitch;
				acce[2] = event.acceleration.azimuth;
				acceRaw[0] = event.acceleration.x;
				acceRaw[1] = event.acceleration.y;
				acceRaw[2] = event.acceleration.z;

				acceRaw[1] *= -1.0f;

				if(DEBUG==1){
					LOGI(
						"accl(roll,pitch,azimuth,x,y,z,t): %f %f %f %f %f %f %lld",
						acce[0], acce[1], acce[2], acceRaw[0], acceRaw[1], acceRaw[2], event.timestamp
					);
				}
				break;
			case ASENSOR_TYPE_GYROSCOPE:
				gyro[0] = event.vector.roll;
				gyro[1] = event.vector.pitch;
				gyro[2] = event.vector.azimuth;
				gyroRaw[0] = event.vector.x;
				gyroRaw[1] = event.vector.y;
				gyroRaw[2] = event.vector.z;

				if(DEBUG==1){
					LOGI(
						"gyro(roll,pitch,azimuth,x,y,z,t): %f %f %f %f %f %f %lld",
						gyro[0], gyro[1], gyro[2], gyroRaw[0], gyroRaw[1], gyroRaw[2], event.timestamp
					);
				}
				break;
			case ASENSOR_TYPE_MAGNETIC_FIELD:
				magn[0] = event.magnetic.roll;
				magn[1] = event.magnetic.pitch;
				magn[2] = event.magnetic.azimuth;
				magnRaw[0] = event.magnetic.x;
				magnRaw[1] = event.magnetic.y;
				magnRaw[2] = event.magnetic.z;
				if(DEBUG==1){
					LOGI(
						"magn(roll,pitch,azimuth,x,y,z,t): %f %f %f %f %f %f %lld",
						magn[0], magn[1], magn[2], magnRaw[0], magnRaw[1], magnRaw[2], event.timestamp
					);
				}
				break;
			case ASENSOR_TYPE_PROXIMITY:
				altitude = event.distance;
				break;
		}

		// Logging
		if(LOG==1){

			// ASENSOR_TYPE_ACCELEROMETER
			if(event.type==ASENSOR_TYPE_ACCELEROMETER && USE_ACCELEROMETER==1) {

				if(log_i<(LOG_SIZE-1)){
					loog[log_i] = event.timestamp;
					log_i++;
				} else {

					if(log_output==0){
						for(log_j=1; log_j<LOG_SIZE; log_j++){
							LOGI("#id: %d: delta acce(t): %f", log_j, (loog[log_j]-loog[log_j-1])/1000000.0f);
						}
						LOGI("SOLL: acce: %d, acce: %d, magn: %d", log_acce, log_gyro, log_magn);
						log_output = 1;
					}
					break;
				}
			}

			// ASENSOR_TYPE_GYROSCOPE
			if(event.type==ASENSOR_TYPE_GYROSCOPE && USE_GYROSCOPE==1) {

				if(log_i<(LOG_SIZE-1)){
					loog[log_i] = event.timestamp;
					log_i++;
				} else {

					if(log_output==0){
						for(log_j=1; log_j<LOG_SIZE; log_j++){
							LOGI("#id: %d: delta gyro(t): %f", log_j, (loog[log_j]-loog[log_j-1])/1000000.0f);
						}
						LOGI("SOLL: acce: %d, gyro: %d, magn: %d", log_acce, log_gyro, log_magn);
						log_output = 1;
					}
					break;
				}
			}

			// ASENSOR_TYPE_MAGNETIC_FIELD
			if(event.type==ASENSOR_TYPE_MAGNETIC_FIELD && USE_MAGNETIC==1) {

				if(log_i<(LOG_SIZE-1)){
					loog[log_i] = event.timestamp;
					log_i++;
				} else {

					if(log_output==0){
						for(log_j=1; log_j<LOG_SIZE; log_j++){
							LOGI("#id: %d: delta magn(t): %f", log_j, (loog[log_j]-loog[log_j-1])/1000000.0f);
						}
						LOGI("SOLL: acce: %d, gyro: %d, magn: %d", log_acce, log_gyro, log_magn);
						log_output = 1;
					}
					break;
				}
			}

		}
	}

	timestamp = event.timestamp;

    sendValues();

	return 1;
}


/*
 * Class:     io_quadroid_ndk_QuadroidLib
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_io_quadroid_UDPSensor_ndk_QuadroidLib_init(JNIEnv *env, jclass clazz){

    sensorManager = ASensorManager_getInstance();
    looper = ALooper_forThread();

    if(looper == NULL){
    	looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    }

    queue = ASensorManager_createEventQueue(sensorManager, looper, LOOPER_ID, get_sensor_events, NULL);

    if(USE_PRESSURE==1){
    		pressureSensor = ASensorManager_getDefaultSensor(sensorManager, 6);
    		ASensorEventQueue_enableSensor(queue, pressureSensor);
    		ASensorEventQueue_setEventRate(queue, pressureSensor, SAMP_PER_SEC);
        }

    if(USE_ACCELEROMETER==1){
    	acceSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_ACCELEROMETER);
		ASensorEventQueue_enableSensor(queue, acceSensor);
		ASensorEventQueue_setEventRate(queue, acceSensor, SAMP_PER_SEC);
		log_acce = ASensor_getMinDelay(acceSensor);
    }

    if(USE_GYROSCOPE==1){
    	gyroSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_GYROSCOPE);
		ASensorEventQueue_enableSensor(queue, gyroSensor);
		ASensorEventQueue_setEventRate(queue, gyroSensor, SAMP_PER_SEC);
		log_gyro = ASensor_getMinDelay(gyroSensor);
    }

    if(USE_MAGNETIC==1){
    	magnSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_MAGNETIC_FIELD);
		ASensorEventQueue_enableSensor(queue, magnSensor);
		ASensorEventQueue_setEventRate(queue, magnSensor, SAMP_PER_SEC);
		log_magn = ASensor_getMinDelay(magnSensor);
    }

    if(USE_DISTANCE==1){
    	proxSensor = ASensorManager_getDefaultSensor(sensorManager, ASENSOR_TYPE_PROXIMITY);
        ASensorEventQueue_enableSensor(queue, proxSensor);
        ASensorEventQueue_setEventRate(queue, proxSensor, SAMP_PER_SEC);
    }

    currentTime = time(0);
    ALooper_pollAll(-1, NULL, &events, NULL);
}