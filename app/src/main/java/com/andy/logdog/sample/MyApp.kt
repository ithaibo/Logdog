package com.andy.logdog.sample

import android.app.Application
import android.content.Context
import com.andy.logdog.Logdog
import com.andy.logdog.sample.benchmark.SyncLogger
import com.andy.logdog.sample.benchmark.XLogger
import com.andy.mmap.Mmap
import java.io.File

class MyApp : Application() {
    companion object {
        private var app: MyApp? = null
        fun getApp() : MyApp? {
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
//        Logdog.getInstance().i("App is create")
    }


    private fun initLogdog() {
        val file = File(this.getExternalFilesDir(null), "logdog")
        val path = file.absolutePath
        Logdog.getInstance().init(path)
        Logdog.getInstance().d("Logdog start...")

        val fileSync = File(this.getExternalFilesDir(null), "synclog")
        val pathSync = fileSync.absolutePath
        SyncLogger.getInstance().init(pathSync)
        SyncLogger.getInstance().d("Logdog start...")

        XLogger.getInstance().init(this.getExternalFilesDir(null)?.absolutePath!!)
        XLogger.getInstance().d("Logdog start...")
    }

    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
    }

    override fun onTerminate() {
        super.onTerminate()
        Mmap.getInstance().release()
    }
}