package com.jt28.a6735.sc;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.view.animation.Animation;
import android.view.animation.AnimationUtils;
import android.view.animation.LinearInterpolator;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.jt28.a6735.h_ble.BleController;
import com.jt28.a6735.h_ble.callback.OnReceiverCallback;
import com.jt28.a6735.h_ble.callback.OnWriteCallback;
import com.jt28.a6735.sc.Until.Crc16;
import com.jt28.a6735.sc.view.RockerView;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

import static com.jt28.a6735.sc.view.RockerView.DirectionMode.DIRECTION_8;

public class SendAndReciveActivity extends AppCompatActivity implements SensorEventListener {
    private BleController mBleController;
    private TextView mReciveText;
    private StringBuffer mReciveString = new StringBuffer();
    public static final String REQUESTKEY_SENDANDRECIVEACTIVITY = "SendAndReciveActivity";

    private RockerView mRockerView;

    // 加速(重力)传感器API
    private SensorManager mSensorManager = null;
    private Sensor mSensor = null;

    // 加速(重力)传感器开关
    private ToggleButton mToggle = null;

    // 加载动画
    private ImageView mWheelView = null;
    private Animation mAnimation = null;

    private int run_flag = 0;

    private String TAG = "lhb";

    public static String byteArrayToStr(byte[] byteArray) {
        if (byteArray == null) {
            return null;
        }
        String str = null;
        try {
            str = new String(byteArray,"GBK");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return str;
    }

    @Override
    protected void onResume() {
        super.onResume();
        // 初始化传感器
        mSensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        mSensor = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
    }
    @Override
    protected void onPause() {
        // 注销传感器
        mSensorManager.unregisterListener(this);
        super.onPause();
    }

    @Override
    protected void onStop() {
        //close();
        super.onStop();
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_send_and_recive);

        initView();
        // TODO 在新的界面要获取实例，无需init
        mBleController = BleController.getInstance();
        // TODO 接收数据的监听
        mBleController.registReciveListener(REQUESTKEY_SENDANDRECIVEACTIVITY, new OnReceiverCallback() {
            @Override
            public void onRecive(byte[] value) {
                mReciveString.append(byteArrayToStr(value) + "\r\n");
                if(mReciveString.length() > 120) {
                    mReciveString.delete(0,mReciveString.length());
                }
                mReciveText.setText(mReciveString.toString());
            }
        });

        Button send = (Button) findViewById(R.id.send);
        send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SendStructData(0,0,-2000,2000);
            }
        });

        mRockerView = (RockerView) findViewById(R.id.my_rocker);
        mRockerView.setOnShakeListener(DIRECTION_8, new RockerView.OnShakeListener() {
            @Override
            public void onStart() {

            }

            @Override
            public void direction(RockerView.Direction direction) {
                if (direction == RockerView.Direction.DIRECTION_CENTER){
                    //mTvShake.setText("当前方向：中心");
                    if(run_flag != 6) {
                        run_flag = 6;
                        Log.d(TAG, "当前方向：中心");
                        SendStructData(0, 0, 0,0);
                        onAnimated(false);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_DOWN){
                    //mTvShake.setText("当前方向：下");
                    if(run_flag != 7) {
                        run_flag = 7;
                        Log.d(TAG, "当前方向：下");
                        SendStructData(0, 0, -2000,-2000);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_LEFT){
                    //mTvShake.setText("当前方向：左");
                    if(run_flag != 8) {
                        run_flag = 8;
                        Log.d(TAG, "当前方向：左");
                        SendStructData(0, 0, -1500,1500);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_UP){
                    //mTvShake.setText("当前方向：上");
                    if(run_flag != 9) {
                        run_flag = 9;
                        Log.d(TAG, "当前方向：上");
                        SendStructData(0, 0, 2000,2000);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_RIGHT){
                    //mTvShake.setText("当前方向：右");
                    if(run_flag != 10) {
                        run_flag = 10;
                        Log.d(TAG, "当前方向：右");
                        int[] s_data = new int[10];
                        SendStructData(0, 0, 1500,-1500);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_DOWN_LEFT){
                    //mTvShake.setText("当前方向：左下");
                    if(run_flag != 11) {
                        run_flag = 11;
                        Log.d(TAG, "当前方向：左下");
                        SendStructData(0, 0, -2000,-2400);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_DOWN_RIGHT){
                    //mTvShake.setText("当前方向：右下");
                    if(run_flag != 12) {
                        run_flag = 12;
                        Log.d(TAG, "当前方向：右下");
                        SendStructData(0, 0, -2400,-2000);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_UP_LEFT){
                    //mTvShake.setText("当前方向：左上");
                    if(run_flag != 13) {
                        run_flag = 13;
                        Log.d(TAG, "当前方向：左上");
                        SendStructData(0, 0, 2000,2400);
                        onAnimated(true);
                    }
                }else if (direction == RockerView.Direction.DIRECTION_UP_RIGHT){
                    //mTvShake.setText("当前方向：右上");
                    if(run_flag != 14) {
                        run_flag = 14;
                        Log.d(TAG, "当前方向：右上");
                        SendStructData(0, 0, 2400,2000);
                        onAnimated(true);
                    }
                }
            }

            @Override
            public void onFinish() {

            }
        });
        mRockerView.setOnAngleChangeListener(new RockerView.OnAngleChangeListener() {
            @Override
            public void onStart() {

            }

            @Override
            public void angle(double angle) {
                //mTvAngle.setText("当前角度："+angle);
                //Log.d(TAG,"当前角度"+angle);
            }

            @Override
            public void onFinish() {

            }
        });
        mRockerView.setOnDistanceLevelListener(new RockerView.OnDistanceLevelListener() {
            @Override
            public void onDistanceLevel(int level) {
                //mTvLevel.setText("当前距离级别："+level);
               // Log.d(TAG,"当前距离级别"+level);
            }
        });
        mToggle = (ToggleButton) findViewById(R.id.toggle);
        mToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    // 注册传感器
                    mSensorManager.registerListener(SendAndReciveActivity.this, mSensor, SensorManager.SENSOR_DELAY_GAME);
                } else {
                    // 注销传感器
                    mSensorManager.unregisterListener(SendAndReciveActivity.this);
                }
            }
        });

        mWheelView = (ImageView) findViewById(R.id.wheel_view);
        // 加载动画
        mAnimation = AnimationUtils.loadAnimation(this, R.anim.rotate);
        // 设置匀速旋转速率
        mAnimation.setInterpolator(new LinearInterpolator());
        onAnimated(false);
    }

    public void onAnimated(boolean isAnim) {
        if (isAnim) {
            mWheelView.startAnimation(mAnimation);
        } else {
            mWheelView.clearAnimation();
        }
    }

    private void initView() {
        mReciveText = (TextView) findViewById(R.id.mReciveText);
    }

    public static int byteArrayToInt(byte[] b, int offset) {
        int value= 0;
        for (int i = 0; i < 4; i++) {
            int shift= (4 - 1 - i) * 8;
            value +=(b[i + offset] & 0x000000FF) << shift;
        }
        return value;
    }

    private byte[] intToByteArray(final int integer) {
        int byteNum = (40 - Integer.numberOfLeadingZeros (integer < 0 ? ~integer : integer))/ 8;
        byte[] byteArray = new byte[4];

        for (int n = 0; n < byteNum; n++)
            byteArray[3 - n] = (byte) (integer>>> (n * 8));
        return (byteArray);
    }

    public static byte[] intToByteArray1(int i) {
        byte[] result = new byte[4];
        result[0] = (byte)((i >> 24) & 0xFF);
        result[1] = (byte)((i >> 16) & 0xFF);
        result[2] = (byte)((i >> 8) & 0xFF);
        result[3] = (byte)(i & 0xFF);
        return result;
    }

    private byte[] bytes = new byte[7];//= sendText.getBytes();

    private void SendStructData(int id,int mt,int left,int right){
        this.bytes[0] = (byte) id;
        this.bytes[1] = (byte) mt;

        if(left > 0) {
            byte b_l =(byte) (left & 0xff);
            byte b_h =(byte) ((left >> 8) & 0xFF);
            b_h = (byte) (b_h | 0x80);
            this.bytes[2] = b_l;
            this.bytes[3] = b_h;
        } else {
            left = 0 - left;
            byte b_l =(byte) (left & 0xff);
            byte b_h =(byte) ((left >> 8) & 0xFF);
            b_h = (byte) (b_h & 0x7f);
            this.bytes[2] = b_l;
            this.bytes[3] = b_h;
        }

        if(right > 0) {
            byte b_l =(byte) (right & 0xff);
            byte b_h =(byte) ((right >> 8) & 0xFF);
            b_h = (byte) (b_h | 0x80);
            this.bytes[4] = b_l;
            this.bytes[5] = b_h;
        } else {
            right = 0 - right;
            byte b_l =(byte) (right & 0xff);
            byte b_h =(byte) ((right >> 8) & 0xFF);
            b_h = (byte) (b_h & 0x7f);
            this.bytes[4] = b_l;
            this.bytes[5] = b_h;
        }
        this.bytes[6] = Crc16.getadd(bytes,6);
        Log.d(TAG,"发送" + Arrays.toString(bytes));
        //this.bytes[6] = (byte) Crc16.getcrc(bytes,6);
//        for(int i = 0;(i < data1.length) & (i < 10);i++) {
//            byte[] b = intToByteArray1(data1[i]);
////            Log.d("lhb", Arrays.toString(b));
//            this.bytes[(i*4)+4] = (byte) b[3];
//            this.bytes[(i*4)+5] = (byte) b[2];
//            this.bytes[(i*4)+6] = (byte) b[1];
//            this.bytes[(i*4)+7] = (byte) b[0];
//        }
        mBleController.writeBuffer(bytes, new OnWriteCallback() {
            @Override
            public void onSuccess() {
                Toast.makeText(SendAndReciveActivity.this, "发送成功！", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void onFailed(int state) {
                Toast.makeText(SendAndReciveActivity.this, "发送失败！", Toast.LENGTH_SHORT).show();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        //移除接收数据的监听
        mBleController.unregistReciveListener(REQUESTKEY_SENDANDRECIVEACTIVITY);
        // TODO 断开连接
        mBleController.closeBleConn();
    }

    @Override
    public void onSensorChanged(SensorEvent event) {
        if (event.sensor == null) {
            return;
        }

        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
            float gx = event.values[0];
            float gy = event.values[1];
            float gz = event.values[2];
            if (gx < -3) {
                if(run_flag != 1) {
                    run_flag = 1;
                    Log.d(TAG, "右转");
                    SendStructData(0, 0, 1500,-1500);
                    onAnimated(true);
                }
            } else if (gx > 6) {
//                Logger.d(TAG, "[11] --> go backward...");
//                mTextView.setText("重力感应:后退");
//                data[0] = 0x02;
//                writeStream(data);
                if(run_flag != 2) {
                    run_flag = 2;
                    Log.d(TAG, "左转");
                    SendStructData(0, 0, -1500,1500);
                    onAnimated(true);
                }
            } else if (gy < -4) {
//                Logger.d(TAG, "[12] --> turn left...");
//                mTextView.setText("重力感应:左转");
//                data[0] = 0x03;
//                writeStream(data);
                if(run_flag != 3) {
                    run_flag = 3;
                    Log.d(TAG, "前进");
                    SendStructData(0, 0, 2000,2000);
                    onAnimated(true);
                }
            } else if (gy > 4) {
//                Logger.d(TAG, "[13] --> turn right...");
//                mTextView.setText("重力感应:右转");
//                data[0] = 0x04;
//                writeStream(data);
                if(run_flag != 4) {
                    run_flag = 4;
                    Log.d(TAG, "后退");
                    SendStructData(0, 0, -2000,-2000);
                    onAnimated(true);
                }
            } else {
//                Logger.d(TAG, "[14] --> gx:" + gx + "------gy:" + gy + "------gz:" + gz);
//                mTextView.setText("重力感应:暂停");
//                data[0] = 0x00;
//                writeStream(data);
                if(run_flag != 5) {
                    run_flag = 5;
                    Log.d(TAG, "暂停");
                    SendStructData(0, 0, 0,0);
                    onAnimated(false);
                }
            }
        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int i) {

    }
}
