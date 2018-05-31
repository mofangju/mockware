/*
 * Copyright (C) 2018 Bill Bao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mockware;

import android.content.Context;
import com.android.mockware.R;
import android.app.Activity;
import android.os.ServiceManager;
import android.os.Bundle;
import android.mockware.MockwareManager;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;
import android.widget.EditText;

/**
 * NOTE: The implementation may hold UI thread, it is just for demo purpose. 
 *       Please do not use it for production purpose!
 */
public class MockwareActivity extends Activity implements OnClickListener {
    private final static String LOG_TAG = "MockwareActivity";
    
    private MockwareManager mockwareManager;
    private Button writeButton = null;
    private EditText writeEditText = null;
    private Button readButton = null;
    private TextView readTextView = null;
    private Button clearButton = null;
    
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);    
        
        writeButton = (Button)findViewById(R.id.button_write);
        writeEditText = (EditText)findViewById(R.id.editview_write);
        readButton = (Button)findViewById(R.id.button_read);
        readTextView = (TextView)findViewById(R.id.textview_read);
        clearButton = (Button)findViewById(R.id.button_clear);

        readButton.setOnClickListener(this);
        writeButton.setOnClickListener(this);
        clearButton.setOnClickListener(this);
        
        mockwareManager = (MockwareManager) getSystemService(Context.MOCKWARE_SERVICE);
        
        Log.i(LOG_TAG, "MockwareActivity Created");
    }
    
    @Override
    public void onClick(View v) {
        if(v.equals(readButton)) {
            int val = mockwareManager.getVal();
            String text = String.valueOf(val);
            readTextView.setText(text);
            Log.d(LOG_TAG, "call mockwareManager.getVal() with result:" + val);            
        } else if(v.equals(writeButton)) {
            String text = writeEditText.getText().toString();
            int val = Integer.parseInt(text);
            mockwareManager.setVal(val);
            writeEditText.setText("");
            Log.d(LOG_TAG, "call mockwareManager.set(" + text + ") with result:" + val);
        } else if(v.equals(clearButton)) {
            readTextView.setText("");
            writeEditText.setText("");
        }
    }
}