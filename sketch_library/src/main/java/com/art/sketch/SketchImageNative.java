package com.art.sketch;

import android.graphics.Bitmap;

public class SketchImageNative {

    public static native int nativeThresholdImage(Bitmap srcBitmap, Bitmap[] previewBitmaps);


    static {
        System.loadLibrary("sketch_image");
    }
}
