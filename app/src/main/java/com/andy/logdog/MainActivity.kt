package com.andy.logdog

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*
import java.io.File

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)


        val file = File(applicationContext
                .getExternalFilesDir(null), "logdog")
        Logdog.path = file.absolutePath


        // Example of a call to a native method
        btn_read.setOnClickListener { readFile() }
        btn_write_mmap.setOnClickListener { writeInputWithMMAP() }
        btn_init.setOnClickListener { Logdog.getInstance().init(application) }
    }

    private fun getEditInput(): String {
        if(et.text.isEmpty()) return ""
        return et.text.toString()
    }

    private fun writeInputWithMMAP() {
        Logdog.getInstance().w(getEditInput())
        et.setText("")
    }

    private fun readFile() {
        val read = Logdog.getInstance().read()
        Log.i("Main", "read from file: $read")
        tv.text = read
    }

    override fun onDestroy() {
        super.onDestroy()
        Logdog.getInstance().release()
    }
}
