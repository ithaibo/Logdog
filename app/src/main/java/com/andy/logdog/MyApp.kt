package com.andy.logdog

import android.app.Application
import android.content.Context
import java.io.File

class MyApp : Application() {
    companion object {
        private var app:MyApp? = null
        fun getApp() :MyApp? {
            return app
        }
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        app = this
        initLogdog()
    }

    override fun onCreate() {
        super.onCreate()
//        initLogdog()
    }


    private fun initLogdog() {
        val file = File(this
                .getExternalFilesDir(null), "logdog")
        val path = file.absolutePath
        Logdog.getInstance().init(path)
    }

    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
        Logdog.getInstance().w("Memory is not enough")
    }

    override fun onTerminate() {
        super.onTerminate()
        Logdog.getInstance().release()
    }
}