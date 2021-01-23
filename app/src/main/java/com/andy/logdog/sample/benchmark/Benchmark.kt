package com.andy.logdog.sample.benchmark

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.andy.logdog.Logdog
import com.andy.logdog.R
import kotlinx.android.synthetic.main.activity_benchmark.*
import java.math.BigDecimal

class Benchmark : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_benchmark)

        btn_mmap.setOnClickListener {testMmap()}
        btn_sync_io.setOnClickListener {testSyncIO()}
        btn_okio.setOnClickListener {testOKIO()}
    }

    fun testMmap() {
        Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()
            for (i in 0 until  result.timesWrite) {
                Logdog.getInstance().d(LocalMockLogRepository.apiResponse)
            }
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "mmap"

            tv_mmap.post { tv_mmap.text = result.toString() }
        }.start()
    }

    fun testSyncIO() {
        Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()

            for (i in 0 until  result.timesWrite) {
                SyncLogger.getInstance().d(LocalMockLogRepository.apiResponse)
            }
            SyncLogger.getInstance().exit()
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "SyncIO"

            tv_sync_io.post { tv_sync_io.text = result.toString() }
        }.start()
    }

    fun testOKIO() {
    }

    fun readTimes():Int {
        return if(et_times.text.isEmpty()) 1 else BigDecimal(et_times.text.toString()).toInt()
    }
}
