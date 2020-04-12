package com.andy.logdog.sample

import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.andy.logdog.R
import com.andy.mmap.Mmap
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

    }

    fun toLog(view: View) {
        startActivity(Intent(this, LogDemo::class.java))
    }

    fun toNote(view: View) {
        startActivity(Intent(this, Note::class.java))
    }
}
