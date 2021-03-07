package com.andy.logdog.sample

import android.content.Context
import android.util.Log
import androidx.multidex.MultiDex
import androidx.multidex.MultiDexApplication
import com.andy.logdog.sample.benchmark.SyncLogger
import com.andy.logdog.sample.benchmark.XLogger
import com.andy.logdog.sample.logdog.Logdog
import com.andy.mmap.Mmap
import com.dianping.logan.Logan
import com.dianping.logan.LoganConfig
import java.io.File

class MyApp : MultiDexApplication() {
    private val FILE_NAME = "logan_v1"

    companion object {
        private var app: MyApp? = null
        fun getApp(): MyApp? {
            return app
        }
    }

    override fun attachBaseContext(base: Context?) {
        super.attachBaseContext(base)
        MultiDex.install(this)
    }

    override fun onCreate() {
        super.onCreate()
        app = this
        initLogdog()
        initSyncLog()
//        initXLog()
        initLogan()
    }


    private fun initLogdog() {
        val file = File(this.getExternalFilesDir(null), "logdog")
        val path = file.absolutePath
        if (!file.exists()) {
            file.mkdirs()
        }
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
        initLogan()
    }

    private fun initLogan() {
        val config = LoganConfig.Builder()
                .setCachePath(applicationContext.filesDir.absolutePath)
                .setPath(applicationContext.getExternalFilesDir(null)!!.absolutePath
                        + File.separator + FILE_NAME)
                .setEncryptKey16("0123456789012345".toByteArray())
                .setEncryptIV16("0123456789012345".toByteArray())
                .build()
        Logan.init(config)
        Logan.setDebug(true)
        Logan.setOnLoganProtocolStatus { cmd, code -> Log.d("LogdogApp", "clogan > cmd : $cmd | code : $code") }
    }
}