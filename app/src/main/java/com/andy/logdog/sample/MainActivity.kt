package com.andy.logdog.sample

import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.andy.logdog.sample.benchmark.Benchmark
import com.andy.logdog.sample.logdog.Logdog
import com.andy.mmap.Mmap
import com.andy.note.R
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Example of a call to a native method
        btn_read.setOnClickListener { readFile() }
        btn_benchmark.setOnClickListener { startActivity(Intent(this, Benchmark::class.java)) }
        findViewById<View>(R.id.btn_write_mmap).setOnClickListener { writeInputWithMMAP() }
    }

    private fun getEditInput(): String {
        if(et.text.isEmpty()) return ""
        return et.text.toString()
    }

    private fun writeInputWithMMAP() {
        Logdog.getInstance().d(getEditInput())
        et.setText("")
    }

    private fun readFile() {
        val read = Mmap.getInstance().read()
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
//        Logdog.getInstance().d("MainActivity.onResume invoked")
    }

    override fun onPause() {
        super.onPause()
//        Logdog.getInstance().d("MainActivity.onPause invoked")
    }

    override fun onStop() {
        super.onStop()
//        Logdog.getInstance().d("MainActivity.onStop invoked")
    }

    override fun onDestroy() {
        super.onDestroy()
//        Logdog.getInstance().i("MainActivity.onDestroy invoked")
    }
}
