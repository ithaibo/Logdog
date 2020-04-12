package com.andy.logdog.sample

import android.app.Application
import android.content.Context

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
    }

    override fun onCreate() {
        super.onCreate()
    }
}