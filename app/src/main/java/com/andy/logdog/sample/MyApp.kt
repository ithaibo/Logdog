package com.andy.logdog.sample

import android.app.Application
import android.content.Context
import com.andy.logdog.sample.benchmark.SyncLogger
import com.andy.logdog.sample.benchmark.XLogger
import com.andy.logdog.sample.logdog.Logdog
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
        initSyncLog()
        initXLog()
    }


    private fun initLogdog() {
        val file = File(this.getExternalFilesDir(null), "logdog")
        val path = file.absolutePath
        Logdog.getInstance().init(path)
    }

    private fun initSyncLog() {
        val fileSync = File(this.getExternalFilesDir(null), "synclog")
        val pathSync = fileSync.absolutePath
        SyncLogger.getInstance().init(pathSync)
        SyncLogger.getInstance().d("Logdog start...")
    }

    private fun initXLog() {
        XLogger.getInstance().init(this.getExternalFilesDir(null)?.absolutePath!!)
    }

    override fun onTrimMemory(level: Int) {
        super.onTrimMemory(level)
    }

    override fun onTerminate() {
        super.onTerminate()
        Mmap.getInstance().release()
    }
}