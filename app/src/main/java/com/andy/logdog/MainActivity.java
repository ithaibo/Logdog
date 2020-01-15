package com.andy.logdog;

import android.os.Bundle;
import android.util.Log;
import android.widget.EditText;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

public class MainActivity extends AppCompatActivity {
    EditText editText;
    private TextView tv;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        editText = findViewById(R.id.et);
        tv = findViewById(R.id.tv);
        // Example of a call to a native method
        findViewById(R.id.btn_write).setOnClickListener(v-> writeInputToFile());
        findViewById(R.id.btn_read).setOnClickListener(v-> readFile());

        Logdog.getInstance().init(this);
    }

    private void readFile() {
        String read = Logdog.getInstance().read_file(Logdog.path);
        Log.i("Main", "read from file: " + read);
        tv.setText(read);
    }

    private void writeInputToFile() {
        if (editText.getText().length() <= 0) return;
        String input = editText.getText().toString();
        Logdog.getInstance().w(Logdog.path, input);
    }
}
