package com.andy.logdog

import android.os.Bundle
import android.util.Log
import androidx.appcompat.app.AppCompatActivity
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        Logdog.getInstance().i("MainActivity.onCreate invoked")
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
        btn_read.setOnClickListener { readFile() }
        btn_write_mmap.setOnClickListener { writeInputWithMMAP() }
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

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        Logdog.getInstance().d("MainActivity.onAttachedToWindow invoked")
    }

    override fun onStart() {
        super.onStart()
        Logdog.getInstance().d("MainActivity.onStart invoked")
    }

    override fun onResume() {
        super.onResume()
        Logdog.getInstance().d("MainActivity.onResume invoked")
    }

    override fun onPause() {
        super.onPause()
        Logdog.getInstance().d("MainActivity.onPause invoked")
    }

    override fun onStop() {
        super.onStop()
        Logdog.getInstance().d("MainActivity.onStop invoked")
    }

    override fun onDestroy() {
        super.onDestroy()
        Logdog.getInstance().i("MainActivity.onDestroy invoked")
    }
}
