package com.example.sketchlibrarydemo

import android.content.Intent
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.os.Bundle
import android.support.v7.app.AppCompatActivity;
import android.view.View
import com.art.sketch.SketchImageNative

import kotlinx.android.synthetic.main.activity_main.*
import java.io.FileNotFoundException

class MainActivity : AppCompatActivity() {
    private val IMAGE_CODE = 100
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        initView()
    }
    private fun initView() {
        loadImageBtn.setOnClickListener {
            val getAlbum = Intent(Intent.ACTION_GET_CONTENT)
            getAlbum.type = "image/*"
            startActivityForResult(getAlbum, IMAGE_CODE)
        }
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (resultCode == RESULT_OK && requestCode == IMAGE_CODE) {
            try {
                val uri = data!!.data
                val bitmap = BitmapFactory.decodeStream(contentResolver.openInputStream(uri!!))
                val previewBitmap = arrayOfNulls<Bitmap>(1)
                SketchImageNative.nativeThresholdImage(bitmap, previewBitmap)
                previewImage.visibility = View.VISIBLE
                previewImage.setImageBitmap(previewBitmap[0])
            } catch (e: FileNotFoundException) {
                e.printStackTrace()
            }

        }
    }
}
