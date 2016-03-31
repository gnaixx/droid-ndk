//
// Created by 薛祥清 on 16/3/10.
//

#include "com_example_gnaix_ndk_NativeMethod.h"
#include <android/log.h>
#include <sys/system_properties.h>
#include <string>

using namespace std;

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG
#define TAG "NDK_NATIVE"
#define LOGD(fmt, args...)  __android_log_print(ANDROID_LOG_DEBUG,TAG, fmt, ##args)
#define DEBUG_PRINT(format, args...) \
    LOGD(format, ##args)
#else
#define  DEBUG_PRINT(format, args...)
#endif

JNIEXPORT jint JNICALL Java_com_example_gnaix_ndk_NativeMethod_getInt
        (JNIEnv *env, jclass object, jint num)
{
    int len;
    char buf[1024];
    __system_property_get("ro.serialno", buf);
    LOGD("serialno : %s", buf);
    len = strlen(buf);
    return num + len;
}

JNIEXPORT jstring JNICALL Java_com_example_gnaix_ndk_NativeMethod_getString
        (JNIEnv *env, jclass object, jstring str){

    //1. 将unicode编码的java字符串转换成C风格字符串
    const char *buf_name = env->GetStringUTFChars(str, 0);
    int len = strlen(buf_name);
    char n_name[len];
    strcpy(n_name, buf_name);

    //2. 释放内存
    env->ReleaseStringUTFChars(str, buf_name);

    //3. 处理
    char buf[1024];
    __system_property_get(n_name, buf);
    LOGD("serialno : %s", buf);

    //4. 去掉尾部"\0"
    int len_buf = strlen(buf);
    string result(buf, len_buf);

    return env->NewStringUTF(result.c_str());
}

JNIEXPORT jbyteArray JNICALL Java_com_example_gnaix_ndk_NativeMethod_getByteArray
        (JNIEnv *env, jclass object, jbyteArray array){
    //1. 获取数组指针和长度
    jbyte *array_body = env->GetByteArrayElements(array, 0);
    int len_body = env->GetArrayLength(array);

    //2. 具体处理
    jbyteArray result = env->NewByteArray(len_body);
    jbyte buf[len_body];
    for(int i=0; i<len_body; i++){
        buf[i] = array_body[i] + 1;
    }

    //3. 释放内存
    env->ReleaseByteArrayElements(array, array_body, 0);

    //4. 赋值
    env->SetByteArrayRegion(result, 0, len_body, buf);
    return result;
}

JNIEXPORT void JNICALL Java_com_example_gnaix_ndk_NativeMethod_invokeJobject
        (JNIEnv *env, jclass object, jstring name, jint age){
    jclass clazz = NULL;
    jobject jobj = NULL;
    jfieldID fid_name = NULL;
    jfieldID fid_age = NULL;
    jmethodID mid_construct = NULL;
    jmethodID mid_to_string = NULL;
    jstring j_new_str;
    jstring j_result;

    //1. 获取Person类Class引用
    clazz = env->FindClass("com/example/gnaix/ndk/Person");
    if(clazz == NULL){
        LOGD("clazz null");
        return;
    }

    //2. 获取类的默认构造函数ID
    mid_construct = env->GetMethodID(clazz, "<init>", "()V");
    if(mid_construct == NULL){
        LOGD("construct null");
        return;
    }

    //3. 获取实例方法ID和变量ID
    fid_name = env->GetFieldID(clazz, "name", "Ljava/lang/String;");
    fid_age = env->GetFieldID(clazz, "age", "I");
    mid_to_string = env->GetMethodID(clazz, "toString", "()Ljava/lang/String;");
    if(fid_age==NULL || fid_name==NULL || mid_to_string==NULL){
        LOGD("age|name|toString null");
        return;
    }

    //4. 创建该类的实例
    jobj = env->NewObject(clazz, mid_construct);
    if(jobj == NULL){
        LOGD("jobject null");
        return;
    }

    //5. 修改变量值
    env->SetIntField(jobj, fid_age, 23);
    j_new_str = env->NewStringUTF("gnaix");
    env->SetObjectField(jobj, fid_name, j_new_str);

    //6. 调用实例方法
    j_result = (jstring)env->CallObjectMethod(jobj, mid_to_string);
    const char *buf_result = env->GetStringUTFChars(j_result, 0);
    LOGD("object: %s", buf_result);
    env->ReleaseStringUTFChars(j_result, buf_result);

    //7. 释放局部引用
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jobj);
    env->DeleteLocalRef(j_new_str);
    env->DeleteLocalRef(j_result);

}


JNIEXPORT void JNICALL Java_com_example_gnaix_ndk_NativeMethod_invokeStaticFieldAndMethod
        (JNIEnv *env, jclass object, jint age, jstring name){

    jclass clazz = NULL;
    jfieldID fid = NULL;
    jmethodID mid = NULL;
    jstring j_result = NULL;

    //1. 获取Util类的Class引用
    clazz = env->FindClass("com/example/gnaix/ndk/Util");
    if(clazz == NULL){
        LOGD("clazz null");
        return;
    }

    //2. 获取静态变量属性ID和方法ID
    fid = env->GetStaticFieldID(clazz, "STATIC_FIELD", "I");
    if(fid == NULL){
        LOGD("fieldID null");
        return;
    }
    mid = env->GetStaticMethodID(clazz, "getStaticMethod", "(Ljava/lang/String;I)Ljava/lang/String;");
    if(mid == NULL){
        LOGD("method null");
        return;
    }

    //3. 获取静态变量值和调用静态方法
    jint num = env->GetStaticIntField(clazz, fid);
    LOGD("static field value: %d", num);
    j_result = (jstring)env->CallStaticObjectMethod(clazz, mid, name, age);
    const char *buf_result = env->GetStringUTFChars(j_result, 0);
    LOGD("static: %s", buf_result);
    env->ReleaseStringUTFChars(j_result, buf_result);

    //4. 修改静态变量值
    env->SetStaticIntField(clazz, fid, 100);

    //5. 释放局部引用
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(j_result);
}

JNIEXPORT jobjectArray JNICALL Java_com_example_gnaix_ndk_NativeMethod_getPersons
        (JNIEnv *env, jclass object){
    jclass clazz = NULL;
    jobject jobj = NULL;
    jmethodID mid_construct = NULL;
    jfieldID fid_age = NULL;
    jfieldID fid_name = NULL;
    jstring j_name;

    //1. 获取Person类的Class引用
    clazz = env->FindClass("com/example/gnaix/ndk/Person");
    if(clazz == NULL){
        LOGD("clazz null");
        return NULL;
    }

    //2. 获取类的默认构造函数ID
    mid_construct = env->GetMethodID(clazz, "<init>", "()V");
    if(mid_construct == NULL){
        LOGD("construct null");
        return NULL;
    }

    //3. 获取实例方法ID和变量ID
    fid_name = env->GetFieldID(clazz, "name", "Ljava/lang/String;");
    fid_age = env->GetFieldID(clazz, "age", "I");
    if(fid_age==NULL || fid_name==NULL){
        LOGD("age|name null");
        return NULL;
    }

    //4. 处理单个对象并添加到数组
    int size = 5;
    jobjectArray obj_array = env->NewObjectArray(size, clazz, 0);
    for(int i=0; i<size; i++){
        jobj = env->NewObject(clazz, mid_construct);
        if(jobj == NULL){
            LOGD("jobject null");
            return NULL;
        }
        env->SetIntField(jobj, fid_age, 23 + i);
        j_name = env->NewStringUTF("gnaix");
        env->SetObjectField(jobj, fid_name, j_name);
        env->SetObjectArrayElement(obj_array, i, jobj);
    }


    //5. 释放局部变量
    env->DeleteLocalRef(clazz);
    env->DeleteLocalRef(jobj);
    return obj_array;
}