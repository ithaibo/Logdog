package com.andy.logdog;

import android.os.Bundle;
import android.util.Log;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    EditText editText;
    private TextView tv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);


        File file = new File(getApplicationContext()
                .getExternalFilesDir(null), "logdog");
        Logdog.path = file.getAbsolutePath();

        editText = findViewById(R.id.et);
        tv = findViewById(R.id.tv);
        // Example of a call to a native method
        findViewById(R.id.btn_read).setOnClickListener(v-> readFile());
        findViewById(R.id.btn_write_mmap).setOnClickListener(v-> writeInputWithMMAP());
        findViewById(R.id.btn_init).setOnClickListener(v-> Logdog.getInstance().init(getApplication()));
    }

    private void writeInputWithMMAP() {
        Logdog.getInstance().w(getInput());
        editText.setText("");
    }

    private String getInput() {
        String input;
        if (editText.getText().length() <= 0){
           input = "";
        } else {
           input = editText.getText().toString();
        }
        return input;
    }

    private void readFile() {
        String read = Logdog.getInstance().readFile(Logdog.path);
        Log.i("Main", "read from file: " + read);
        tv.setText(read);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Logdog.getInstance().onExit();
    }
}
