package com.andy.logdog.sample

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import com.andy.logdog.R
import com.andy.mmap.Mmap
import kotlinx.android.synthetic.main.activity_note.*
import java.io.File

class Note : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_note)

        //init note
        initNote()

        initViews()
    }

    private fun initViews() {
        btn_read.setOnClickListener { readFile() }
        btn_write_mmap.setOnClickListener {
            writeInputWithMMAP()
            it.post { readFile() }
        }
    }

    private fun initNote() {
        val file = File(this
                .getExternalFilesDir(null), "note")
        val path = file.absolutePath
        //TODO 改单利为工厂
        Mmap.getInstance().init(path)
    }

    private fun getEditInput(): String {
        if(et.text.isEmpty()) return ""
        return et.text.toString()
    }

    private fun writeInputWithMMAP() {
        Mmap.getInstance().save(getEditInput())
        et.setText("")
    }

    private fun readFile() {
        val read = Mmap.getInstance().read()
        Log.i("Main", "read from file: $read")
        tv.text = read
    }
}