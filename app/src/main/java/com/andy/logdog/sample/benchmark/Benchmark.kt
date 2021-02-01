package com.andy.logdog.sample.benchmark

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import com.andy.logdog.sample.logdog.Logdog
import com.andy.note.R
import com.dianping.logan.Logan
import kotlinx.android.synthetic.main.activity_benchmark.*
import java.math.BigDecimal

class Benchmark : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_benchmark)

        btn_mmap.setOnClickListener {testMmap()}
        btn_sync_io.setOnClickListener {testSyncIO()}
        btn_xlog.setOnClickListener {testXLog()}
        btn_logan.setOnClickListener { testLogan() }
    }

    private fun testLogan() {
        val logdogThread = Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()
            for (i in 0 until  result.timesWrite) {
                Logan.w(LocalMockLogRepository.apiResponse, 2)
                result.successTimes++
            }
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "logan"
            tv_mmap.post { tv_logan.text = result.toString() }
        }
        logdogThread.name = "Logan"
        logdogThread.start()
    }

    fun testMmap() {
        val logdogThread = Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()
            for (i in 0 until  result.timesWrite) {
                val success = Logdog.getInstance().d(LocalMockLogRepository.apiResponse)
                if (success) result.successTimes++
            }
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "mmap"

            tv_mmap.post { tv_mmap.text = result.toString() }
        }
        logdogThread.name = "Logdog"
        logdogThread.start()
    }

    fun testSyncIO() {
        Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()

            for (i in 0 until  result.timesWrite) {
                val success =SyncLogger.getInstance().d(LocalMockLogRepository.apiResponse)
                if (success) result.successTimes++
            }
            SyncLogger.getInstance().exit()
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "SyncIO"

            tv_sync_io.post { tv_sync_io.text = result.toString() }
        }.start()
    }

    fun testXLog() {
        Thread{
            val result = Result()
            result.timesWrite = readTimes()
            val start = System.currentTimeMillis()
            val lengthContent = LocalMockLogRepository.apiResponse.length.toLong()
            for (i in 0 until  result.timesWrite) {
                val success =XLogger.getInstance().d(LocalMockLogRepository.apiResponse)
                if (success) result.successTimes++
            }
            val end = System.currentTimeMillis()
            result.timeCost = end - start
            result.contentLength = lengthContent
            result.writeType = "XLog"

            tv_sync_io.post { tv_xlog.text = result.toString() }
        }.start()
    }

    fun readTimes():Int {
        return if(et_times.text.isEmpty()) 1 else BigDecimal(et_times.text.toString()).toInt()
    }
}
