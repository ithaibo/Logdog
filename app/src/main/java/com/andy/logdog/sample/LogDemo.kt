package com.andy.logdog.sample

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.text.Editable
import android.text.TextWatcher
import android.view.View
import androidx.core.widget.addTextChangedListener
import com.andy.logdog.Logdog
import com.andy.logdog.R
import com.andy.mmap.Utils
import kotlinx.android.synthetic.main.activity_log_demo.*
import java.io.File

class LogDemo : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_log_demo)
        //init log component
        Logdog.getInstance().init(generateLogPath())
        //init views
        initViews()
    }

    /**
     * 每天生成一个单独的日志文件
     * TODO 这里是否需要设计一下，以满足各种日志文件产生、分割的策略逻辑
     */
    fun generateLogPath():String {
        val file = File(getExternalFilesDir("log"), Utils.formatDateNow())
        return file.absolutePath
    }

    fun initViews() {
        tv_log_read.post{
            readLog()
        }
    }

    fun saveLog(view: View) {
        //do save
        val contentInput:String? = et_log.text.toString()
        contentInput?.let {
            saveLog(it)
        }

        //reload log saved
        readLog()
    }

    fun saveLog(log2Save: String) {
        Logdog.getInstance().i(log2Save)
    }

    /**
     * 读取最新的日志文件
     */
    fun readLog() {
        showLogRead(Logdog.getInstance().read())
    }

    fun showLogRead(logContent: String?) {
        tv_log_read.text = logContent
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
