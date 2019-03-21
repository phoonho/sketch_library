#include <jni.h>
#include <string>
#include <android_utils.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <map>
#include<iostream>
#include<ctime>
//com.art.sketch
static const char *const sketchImageNative = "com/art/sketch/SketchImageNative";

static struct {
    jclass jClassPoint;
    jmethodID jMethodInit;
    jfieldID jFieldIDX;
    jfieldID jFieldIDY;
} gPointInfo;

static void initClassInfo(JNIEnv *env) {
    gPointInfo.jClassPoint = reinterpret_cast<jclass>(env->NewGlobalRef(
            env->FindClass("android/graphics/Point")));
    gPointInfo.jMethodInit = env->GetMethodID(gPointInfo.jClassPoint, "<init>", "(II)V");
    gPointInfo.jFieldIDX = env->GetFieldID(gPointInfo.jClassPoint, "x", "I");
    gPointInfo.jFieldIDY = env->GetFieldID(gPointInfo.jClassPoint, "y", "I");
}

static jobject createJavaPoint(JNIEnv *env, Point point_) {
    return env->NewObject(gPointInfo.jClassPoint, gPointInfo.jMethodInit, point_.x, point_.y);
}


static vector<Point2f> pointsToNative(JNIEnv *env, jobjectArray points_) {
    int arrayLength = env->GetArrayLength(points_);
    vector<Point2f> result;
    for (int i = 0; i < arrayLength; i++) {
        jobject point_ = env->GetObjectArrayElement(points_, i);
        int pX = env->GetIntField(point_, gPointInfo.jFieldIDX);
        int pY = env->GetIntField(point_, gPointInfo.jFieldIDY);
        result.push_back(Point2f(pX, pY));
    }
    return result;
}


static jobject createJBitmap(JNIEnv *env, int imgWidth, int imgHeight) {
    jobject bitmap;
    jclass bmpCfgCls = env->FindClass("android/graphics/Bitmap$Config");
    jmethodID bmpClsValueOfMid = env->GetStaticMethodID(bmpCfgCls,
                                                        "valueOf",
                                                        "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
    jobject jBmpCfg = env->CallStaticObjectMethod(bmpCfgCls, bmpClsValueOfMid,
                                                  env->NewStringUTF("ARGB_8888"));
    jclass bmpCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMid = env->GetStaticMethodID(bmpCls,
                                                       "createBitmap",
                                                       "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    bitmap = env->CallStaticObjectMethod(bmpCls, createBitmapMid, imgWidth, imgHeight,
                                         jBmpCfg);
    return bitmap;
}

//预扫描图像信息
static jint
native_threshold_image(JNIEnv *env, jclass type, jobject srcBitmap, jobjectArray previewBitmaps) {
    Mat sourceBitmapMat;
    bitmap_to_mat(env, srcBitmap, sourceBitmapMat);
    Mat grayBitmapMat;
    cvtColor(sourceBitmapMat, grayBitmapMat, CV_BGR2GRAY);

    Mat blurImage;
    blur(grayBitmapMat,blurImage,Size(3,3));

    long step2StartTime = clock();//计时开始
    Mat targetBitmapMat;
    int blockSize = 25;
    int constValue = 10;
    adaptiveThreshold(blurImage, targetBitmapMat, 255, CV_ADAPTIVE_THRESH_MEAN_C,
                      CV_THRESH_BINARY,
                      blockSize, constValue);
    double step2Time = (double) (clock() - step2StartTime) / CLOCKS_PER_SEC;

    //将当前的结果输出到预览图
    jobject jBmpObj = createJBitmap(env, targetBitmapMat.cols,
                                    targetBitmapMat.rows);
    mat_to_bitmap(env, targetBitmapMat, jBmpObj);
    env->SetObjectArrayElement(previewBitmaps, 0, jBmpObj);
    return 0;
}

static JNINativeMethod gMethods[] = {
        {
                "nativeThresholdImage",
                "(Landroid/graphics/Bitmap;[Landroid/graphics/Bitmap;)I",
                (void *) native_threshold_image
        }
};

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env = NULL;
    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        return JNI_FALSE;
    }
    jclass classDocScanner = env->FindClass(sketchImageNative);
    if (env->RegisterNatives(classDocScanner, gMethods, sizeof(gMethods) / sizeof(gMethods[0])) <
        0) {
        return JNI_FALSE;
    }
    initClassInfo(env);
    return JNI_VERSION_1_4;
}
