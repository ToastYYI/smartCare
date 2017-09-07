package kong.qingwei.kqwfacedetectiondemo;

import android.Manifest;
import android.bluetooth.BluetoothGatt;
import android.bluetooth.BluetoothGattCharacteristic;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Bitmap;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.SeekBar;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

import com.clj.fastble.BleManager;
import com.clj.fastble.conn.BleCharacterCallback;
import com.clj.fastble.conn.BleGattCallback;
import com.clj.fastble.exception.BleException;
import com.clj.fastble.scan.ListScanCallback;
import com.kongqw.interfaces.OnFaceDetectorListener;
import com.kongqw.interfaces.OnOpenCVInitListener;
import com.kongqw.view.CameraFaceDetectionView;

import org.opencv.core.Mat;
import org.opencv.core.Rect;

import java.util.Arrays;

public class MainActivity extends AppCompatActivity implements OnFaceDetectorListener,SeekBar.OnSeekBarChangeListener,CompoundButton.OnCheckedChangeListener,SensorEventListener {

    private static final String TAG = "MainActivity";
    private static final String FACE1 = "face1";
    private static final String FACE2 = "face2";
    private static boolean isGettingFace = false;
    private Bitmap mBitmapFace1;
    private Bitmap mBitmapFace2;
    private ImageView mImageViewFace1;
    private ImageView mImageViewFace2;
    private TextView mCmpPic;
    private double cmp;
    private CameraFaceDetectionView mCameraFaceDetectionView;
    private PermissionsManager mPermissionsManager;

    private SeekBar mSeekBarDef;
    private SeekBar mSeekBarDef2;
    private TextView ble_tex;

    // 下面的所有UUID及指令请根据实际设备替换
    private static final String UUID_SERVICE = "0000ffe5-0000-1000-8000-00805f9b34fb";
    private static final String UUID_SERVICE2 = "0000ffe0-0000-1000-8000-00805f9b34fb";
    private static final String UUID_INDICATE = "0000ffe4-0000-1000-8000-00805f9b34fb";
    private static final String UUID_NOTIFY = "00000000-0000-0000-8000-00805f9b0000";
    private static final String UUID_WRITE = "0000ffe9-0000-1000-8000-00805f9b34fb";
    private static final String UUID_READ = "0000ffe4-0000-1000-8000-00805f9b34fb";
    private static String SAMPLE_WRITE_DATA = "567890";                  // 要写入设备某一个character的指令
    private byte[] send_data = new byte[11];//byte数组;

    private static final long TIME_OUT = 10000;                                          // 扫描超时时间
    private static final String DEVICE_NAME = "LHB";                         // 符合连接规则的蓝牙设备名
    private static final String[] DEVICE_NAMES = new String[]{};                        // 符合连接规则的蓝牙设备名
    private static final String DEVICE_MAC = "这里写你的设备地址";                        // 符合连接规则的蓝牙设备地址

    private SensorManager sensorManager;
    private Sensor magneticSensor;
    private Sensor accelerometerSensor;
    private Sensor gyroscopeSensor;
    // 将纳秒转化为秒
    private static final float NS2S = 1.0f / 1000000000.0f;
    private float timestamp;
    private float angle[] =new float[3];


    private BleManager bleManager;// Ble核心管理类
    private com.clj.fastble.data.ScanResult scanResult;

    public static byte[] int2byte(int res) {
        byte[] targets = new byte[4];
        targets[0] = (byte) (res & 0xff);
        targets[1] = (byte) ((res >> 8) & 0xff);
        targets[2] = (byte) ((res >> 16) & 0xff);
        targets[3] = (byte) (res >>> 24);
        return targets;
    }

    /* int -> byte[] */
    public static byte[] getBytes(int s, boolean asc) {
        byte[] buf = new byte[4];
        if (asc)
            for (int i = buf.length - 1; i >= 0; i--) {
                buf[i] = (byte) (s & 0x000000ff);
                s >>= 8;
            }
        else
            for (int i = 0; i < buf.length; i++) {
                buf[i] = (byte) (s & 0x000000ff);
                s >>= 8;
            }
        return buf;
    }

    private int face_flag = 0;
    Handler handler2=new Handler();
    Runnable runnable=new Runnable() {
        @Override
        public void run() {
            switch(face_flag) {
                case 0:
                    mImageViewFace1.setImageResource(R.mipmap.adore);
                    face_flag = 1;
                    break;
                case 1:
                    mImageViewFace1.setImageResource(R.mipmap.cry);
                    face_flag = 2;
                    break;
                case 2:
                    mImageViewFace1.setImageResource(R.mipmap.lol);
                    face_flag = 3;
                    break;
                case 3:
                    mImageViewFace1.setImageResource(R.mipmap.rose);
                    face_flag = 4;
                    break;
                case 4:
                    mImageViewFace1.setImageResource(R.mipmap.thumbsup);
                    face_flag = 0;
                    break;
            }
            handler.postDelayed(this, 2000);//work_interval_time
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        bleManager = new BleManager(this);
        bleManager.enableBluetooth();
        scanAndConnect1();

        Button ble = (Button) findViewById(R.id.ble);
        ble.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                scanAndConnect1();
            }
        });

        Button ble_send = (Button) findViewById(R.id.ble_send);
        ble_send.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                write();
            }
        });

        ble_tex = (TextView) findViewById(R.id.text);

        // 设置Switch开关
        Switch mSwitch = (Switch)findViewById(R.id.switch_face);
        mSwitch.setOnCheckedChangeListener(this);
//        Button ble_read = (Button) findViewById(R.id.ble_read);
//        ble_read.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                listen_notify();
//            }
//        });

        // “系统默认SeekBar”
        mSeekBarDef = (SeekBar) findViewById(R.id.seekBar);
        mSeekBarDef.setOnSeekBarChangeListener(this);

        mSeekBarDef2 = (SeekBar) findViewById(R.id.seekBar2);
        mSeekBarDef2.setOnSeekBarChangeListener(this);

        // 检测人脸的View
        mCameraFaceDetectionView = (CameraFaceDetectionView) findViewById(R.id.cameraFaceDetectionView);
        if (mCameraFaceDetectionView != null) {
            mCameraFaceDetectionView.setOnFaceDetectorListener(this);
            mCameraFaceDetectionView.setClickCallback(new CameraFaceDetectionView.ClickCallback(){
                @Override
                public void Find_face(int l_x,int l_y,int x_x,int x_y) {
                    if(face_find) {
                        send_data[0] = 0x3a;
                        send_data[1] = 0x01;
                        send_data[2] = (byte) (l_x & 0xFF);
                        send_data[3] = (byte) ((l_x >> 8) & 0xFF);
                        send_data[4] = (byte) (l_y & 0xFF);
                        send_data[5] = (byte) ((l_y >> 8) & 0xFF);
                        send_data[6] = (byte) (x_x & 0xFF);
                        send_data[7] = (byte) ((x_x >> 8) & 0xFF);
                        send_data[8] = (byte) (x_y & 0xFF);
                        send_data[9] = (byte) ((x_y >> 8) & 0xFF);
                        send_data[10] = 0x0a;
                        Message message = new Message();
                        message.what = 0;
                        handler.sendMessage(message);
                    }
                }
            });

            mCameraFaceDetectionView.setOnOpenCVInitListener(new OnOpenCVInitListener() {
                @Override
                public void onLoadSuccess() {
                    Log.i(TAG, "onLoadSuccess: ");
                    boolean isSwitched = mCameraFaceDetectionView.switchCamera();
                    //mCameraFaceDetectionView.disableView();
                }

                @Override
                public void onLoadFail() {
                    Log.i(TAG, "onLoadFail: ");
                }

                @Override
                public void onMarketError() {
                    Log.i(TAG, "onMarketError: ");
                }

                @Override
                public void onInstallCanceled() {
                    Log.i(TAG, "onInstallCanceled: ");
                }

                @Override
                public void onIncompatibleManagerVersion() {
                    Log.i(TAG, "onIncompatibleManagerVersion: ");
                }

                @Override
                public void onOtherError() {
                    Log.i(TAG, "onOtherError: ");
                }
            });
            mCameraFaceDetectionView.loadOpenCV(getApplicationContext());
        }
        //mCameraFaceDetectionView.disableView();
        // 显示的View
        mImageViewFace1 = (ImageView) findViewById(R.id.face1);
//        mImageViewFace2 = (ImageView) findViewById(R.id.face2);

        handler2.postDelayed(runnable, 2000);//每两秒执行一次runnable.
//        mCmpPic = (TextView) findViewById(R.id.text_view);
//        Button bn_get_face = (Button) findViewById(R.id.bn_get_face);
//
//        mImageViewFace1.setImageResource(R.mipmap.timg);
//
//        // 抓取一张人脸
//        bn_get_face.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View v) {
//                isGettingFace = true;
//            }
//        });

        Button switch_camera = (Button) findViewById(R.id.switch_camera);
        // 切换摄像头（如果有多个）
        switch_camera.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // 切换摄像头
                boolean isSwitched = mCameraFaceDetectionView.switchCamera();
                Toast.makeText(getApplicationContext(), isSwitched ? "摄像头切换成功" : "摄像头切换失败", Toast.LENGTH_SHORT).show();
            }
        });

        // 动态权限检查器
        mPermissionsManager = new PermissionsManager(this) {
            @Override
            public void authorized(int requestCode) {
                Toast.makeText(getApplicationContext(), "权限通过！", Toast.LENGTH_SHORT).show();
            }

            @Override
            public void noAuthorization(int requestCode, String[] lacksPermissions) {
                AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
                builder.setTitle("提示");
                builder.setMessage("缺少相机权限！");
                builder.setPositiveButton("设置权限", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        PermissionsManager.startAppSettings(getApplicationContext());
                    }
                });
                builder.create().show();
            }

            @Override
            public void ignore() {
                AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
                builder.setTitle("提示");
                builder.setMessage("Android 6.0 以下系统不做权限的动态检查\n如果运行异常\n请优先检查是否安装了 OpenCV Manager\n并且打开了 CAMERA 权限");
                builder.setPositiveButton("确认", null);
                builder.setNeutralButton("设置权限", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        PermissionsManager.startAppSettings(getApplicationContext());
                    }
                });
                //builder.create().show();
            }
        };


        sensorManager = (SensorManager) getSystemService(Context.SENSOR_SERVICE);
        magneticSensor =sensorManager
                .getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        accelerometerSensor =sensorManager
                .getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        gyroscopeSensor =sensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE);
        //注册陀螺仪传感器，并设定传感器向应用中输出的时间间隔类型是SensorManager.SENSOR_DELAY_GAME(20000微秒)
        //SensorManager.SENSOR_DELAY_FASTEST(0微秒)：最快。最低延迟，一般不是特别敏感的处理不推荐使用，该模式可能在成手机电力大量消耗，由于传递的为原始数据，诉法不处理好会影响游戏逻辑和UI的性能
        //SensorManager.SENSOR_DELAY_GAME(20000微秒)：游戏。游戏延迟，一般绝大多数的实时性较高的游戏都是用该级别
        //SensorManager.SENSOR_DELAY_NORMAL(200000微秒):普通。标准延时，对于一般的益智类或EASY级别的游戏可以使用，但过低的采样率可能对一些赛车类游戏有跳帧现象
        //SensorManager.SENSOR_DELAY_UI(60000微秒):用户界面。一般对于屏幕方向自动旋转使用，相对节省电能和逻辑处理，一般游戏开发中不使用

        sensorManager.registerListener(this,gyroscopeSensor,
                SensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(this,magneticSensor,
                SensorManager.SENSOR_DELAY_UI);
        sensorManager.registerListener(this,accelerometerSensor,
                SensorManager.SENSOR_DELAY_UI);
    }

    @Override
    protected void onResume() {
        super.onResume();
        // 要校验的权限
        String[] PERMISSIONS = new String[]{Manifest.permission.CAMERA};
        // 检查权限
        mPermissionsManager.checkPermissions(0, PERMISSIONS);
    }

    /**
     * 设置应用权限
     *
     * @param view view
     */
    public void setPermissions(View view) {
        PermissionsManager.startAppSettings(getApplicationContext());
    }

    /**
     * 检测到人脸
     *
     * @param mat  Mat
     * @param rect Rect
     */
    @Override
    public void onFace(Mat mat, Rect rect) {
//        if (isGettingFace) {
//            if (null == mBitmapFace1 || null != mBitmapFace2) {
//                mBitmapFace1 = null;
//                mBitmapFace2 = null;
//
//                // 保存人脸信息并显示
//                FaceUtil.saveImage(this, mat, rect, FACE1);
//                mBitmapFace1 = FaceUtil.getImage(this, FACE1);
//                cmp = 0.0d;
//            } else {
//                FaceUtil.saveImage(this, mat, rect, FACE2);
//                mBitmapFace2 = FaceUtil.getImage(this, FACE2);
//
//                // 计算相似度
//                cmp = FaceUtil.compare(this, FACE1, FACE2);
//                Log.i(TAG, "onFace: 相似度 : " + cmp);
//            }
//
//            runOnUiThread(new Runnable() {
//                @Override
//                public void run() {
//                    if (null == mBitmapFace1) {
//                        mImageViewFace1.setImageResource(R.mipmap.ic_contact_picture);
//                    } else {
//                        mImageViewFace1.setImageBitmap(mBitmapFace1);
//                    }
//                    if (null == mBitmapFace2) {
//                        mImageViewFace2.setImageResource(R.mipmap.ic_contact_picture);
//                    } else {
//                        mImageViewFace2.setImageBitmap(mBitmapFace2);
//                    }
//                    mCmpPic.setText(String.format("相似度 :  %.2f", cmp) + "%");
//                }
//            });
//
//            isGettingFace = false;
//        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        mPermissionsManager.recheckPermissions(requestCode, permissions, grantResults);
    }


    /**
     * 判断是否支持ble
     */
    private boolean isSupportBle() {
        return bleManager.isSupportBle();
    }

    /**
     * 手动开启蓝牙
     */
    private void enableBlue() {
        bleManager.enableBluetooth();
    }

    /**
     * 手动关闭蓝牙
     */
    private void disableBlue() {
        bleManager.disableBluetooth();
    }

    /**
     * 刷新缓存操作
     */
    private void refersh() {
        bleManager.refreshDeviceCache();
    }

    /**
     * 关闭操作
     */
    private void close() {
        bleManager.closeBluetoothGatt();
    }

    /**
     * 扫描出周围所有设备
     */
    private void scanDevice() {
        bleManager.scanDevice(new ListScanCallback(TIME_OUT) {
            @Override
            public void onScanning(com.clj.fastble.data.ScanResult result) {
                scanResult = result;
            }

            @Override
            public void onScanComplete(com.clj.fastble.data.ScanResult[] results) {

            }
        });
    }

    /**
     * 当搜索到周围有设备之后，可以选择直接连某一个设备
     */
    private void connectDevice() {
        bleManager.connectDevice(scanResult, true, new BleGattCallback() {
            @Override
            public void onNotFoundDevice() {
                Log.i(TAG, "未发现设备");
            }

            @Override
            public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
            }

            @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
            @Override
            public void onConnectSuccess(BluetoothGatt gatt, int status) {
                Log.i(TAG, "连接成功");
                gatt.discoverServices();
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                Log.i(TAG, "发现服务");
                bleManager.getBluetoothState();
            }

            @Override
            public void onConnectFailure(BleException exception) {
                Log.i(TAG, "连接断开：" + exception.toString());
                bleManager.handleException(exception);
            }
        });
    }

    /**
     * 扫描指定广播名的设备，并连接（唯一广播名）
     */
    private boolean s_flag = false;
    private void scanAndConnect1() {
        bleManager.scanNameAndConnect(
        DEVICE_NAME,
        TIME_OUT,
        false,
        new BleGattCallback() {
            @Override
            public void onNotFoundDevice() {
                Log.i(TAG, "未发现设备");
                text_msg = "未发现设备";
                Message message=new Message();
                message.what=1;
                handler.sendMessage(message);
            }

            @Override
            public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
                text_msg = "发现设备";
                Message message=new Message();
                message.what=1;
                handler.sendMessage(message);
            }

            @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
            @Override
            public void onConnectSuccess(BluetoothGatt gatt, int status) {
                gatt.discoverServices();
                Log.i(TAG, "连接成功");
                text_msg = "连接成功";
                Message message=new Message();
                message.what=1;
                handler.sendMessage(message);
            }

            @Override
            public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                Log.i(TAG, "发现服务");
                s_flag = true;
                text_msg = "发现服务";
                Message message=new Message();
                message.what=1;
                handler.sendMessage(message);
            }

            @Override
            public void onConnectFailure(BleException exception) {
                Log.i(TAG, "连接中断：" + exception.toString());
                s_flag = false;
                text_msg = "连接中断";
                Message message=new Message();
                message.what=1;
                handler.sendMessage(message);
            }

        });
    }

    /**
     * 扫描指定广播名的设备，并连接（模糊广播名）
     */
    private void scanAndConnect2() {
        bleManager.scanfuzzyNameAndConnect(
            DEVICE_NAME,
            TIME_OUT,
            false,
            new BleGattCallback() {
                @Override
                public void onNotFoundDevice() {
                    Log.i(TAG, "未发现设备");
                }

                @Override
                public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                    Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
                }

                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onConnectSuccess(BluetoothGatt gatt, int status) {
                    gatt.discoverServices();
                    Log.i(TAG, "连接成功");
                }

                @Override
                public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                    Log.i(TAG, "发现服务");
                }

                @Override
                public void onConnectFailure(BleException exception) {
                    Log.i(TAG, "连接中断：" + exception.toString());
                }
            });
    }

    /**
     * 扫描指定广播名的设备，并连接（多个广播名）
     */
    private void scanAndConnect3() {
        bleManager.scanNamesAndConnect(
            DEVICE_NAMES,
            TIME_OUT,
            false,
            new BleGattCallback() {
                @Override
                public void onNotFoundDevice() {
                    Log.i(TAG, "未发现设备");
                }

                @Override
                public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                    Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
                }

                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onConnectSuccess(BluetoothGatt gatt, int status) {
                    gatt.discoverServices();
                    Log.i(TAG, "连接成功");
                }

                @Override
                public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                    Log.i(TAG, "发现服务");
                }

                @Override
                public void onConnectFailure(BleException exception) {
                    Log.i(TAG, "连接中断：" + exception.toString());
                }
            });
    }

    /**
     * 扫描指定广播名的设备，并连接（模糊、多个广播名）
     */
    private void scanAndConnect4() {
        bleManager.scanfuzzyNamesAndConnect(
            DEVICE_NAMES,
            TIME_OUT,
            false,
            new BleGattCallback() {
                @Override
                public void onNotFoundDevice() {
                    Log.i(TAG, "未发现设备");
                }

                @Override
                public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                    Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
                }

                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onConnectSuccess(BluetoothGatt gatt, int status) {
                    gatt.discoverServices();
                    Log.i(TAG, "连接成功");
                }

                @Override
                public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                    Log.i(TAG, "发现服务");
                }

                @Override
                public void onConnectFailure(BleException exception) {
                    Log.i(TAG, "连接中断：" + exception.toString());
                }
            });
    }

    /**
     * 扫描指定物理地址的设备，并连接
     */
    private void scanAndConnect5() {
        bleManager.scanMacAndConnect(
            DEVICE_MAC,
            TIME_OUT,
            false,
            new BleGattCallback() {
                @Override
                public void onNotFoundDevice() {
                    Log.i(TAG, "未发现设备");
                }

                @Override
                public void onFoundDevice(com.clj.fastble.data.ScanResult scanResult) {
                    Log.i(TAG, "发现设备: " + scanResult.getDevice().getAddress());
                }

                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onConnectSuccess(BluetoothGatt gatt, int status) {
                    gatt.discoverServices();
                    Log.i(TAG, "连接成功");
                }

                @Override
                public void onServicesDiscovered(BluetoothGatt gatt, int status) {
                    Log.i(TAG, "发现服务");
                }

                @Override
                public void onConnectFailure(BleException exception) {
                    Log.i(TAG, "连接中断：" + exception.toString());
                }
            });
    }

    /**
     * 取消搜索
     */
    private void cancelScan() {
        bleManager.cancelScan();
    }

    /**
     * notify
     */
    private void listen_notify() {
        bleManager.notify(
            UUID_SERVICE2,
            UUID_READ,
            new BleCharacterCallback() {
                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onSuccess(BluetoothGattCharacteristic characteristic) {
                    Log.i(TAG, "接受到数据" + characteristic.getStringValue(0));
                    for(int i = 0;i < characteristic.getValue().length;i++) {
                        Log.i(TAG,"" + characteristic.getValue()[i]);
                    }
                    text_msg = "接受到数据" + Arrays.toString(characteristic.getValue());
                    Message message=new Message();
                    message.what=1;
                    handler.sendMessage(message);
                }

                @Override
                public void onFailure(BleException exception) {

                }
            });
    }

    /**
     * stop notify
     */
    private boolean stop_notify() {
        return bleManager.stopNotify(UUID_SERVICE, UUID_NOTIFY);
    }

    /**
     * indicate
     */
    private void listen_indicate() {
        bleManager.indicate(
            UUID_SERVICE2,
            UUID_READ,
            new BleCharacterCallback() {

                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onSuccess(BluetoothGattCharacteristic characteristic) {
                    Log.i(TAG, "接受到数据" + characteristic.getValue().toString());
                }

                @Override
                public void onFailure(BleException exception) {

                }
            });
    }

    /**
     * stop indicate
     */
    private boolean stop_indicate() {
        return bleManager.stopIndicate(UUID_SERVICE2, UUID_INDICATE);
    }

    /**
     * write
     */
    private void write() {
        bleManager.writeDevice(
            UUID_SERVICE,
            UUID_WRITE,
            send_data,
            new BleCharacterCallback() {
                @Override
                public void onSuccess(BluetoothGattCharacteristic characteristic) {
                    Log.i(TAG, "发送成功!");
                }

                @Override
                public void onFailure(BleException exception) {

                }
            });
    }

    /**
     * read
     */
    private void read() {
        bleManager.readDevice(
            UUID_SERVICE2,
            UUID_READ,
            new BleCharacterCallback() {
                @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
                @Override
                public void onSuccess(BluetoothGattCharacteristic characteristic) {
                    Log.i(TAG, "接受到数据" + characteristic.getValue().toString());
                }

                @Override
                public void onFailure(BleException exception) {

                }
            });
    }

    private String text_msg = "";
    //ui线程
    private android.os.Handler handler = new android.os.Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what) {
                case 0:
                    if(s_flag) {
                        write();
                    }
                    break;
                case 1:
                    ble_tex.setText(text_msg);
                    break;
                default:
                    break;
            }
        }
    };

    private  int sleep_right = 0;
    private  int sleep_left = 0;
    /*
     * SeekBar滚动时的回调函数
     */
    @Override
    public void onProgressChanged(SeekBar seekBar, int i, boolean b) {
        switch(seekBar.getId()) {
            case R.id.seekBar:
                sleep_right = i;
                break;
            case R.id.seekBar2:
                sleep_left = i;
                break;
            default:
                break;
        }
    }

    /*
    * SeekBar开始滚动的回调函数
    */
    @Override
    public void onStartTrackingTouch(SeekBar seekBar) {

    }

    /*
    * SeekBar停止滚动的回调函数
    */
    @Override
    public void onStopTrackingTouch(SeekBar seekBar) {
        send_data[0] = 0x3a;
        send_data[1] = 0x02;
        send_data[2] = (byte) (sleep_right & 0xFF);
        send_data[3] =  (byte) ((sleep_right >> 8) & 0xFF);
        send_data[4] = (byte) (sleep_left & 0xFF);
        send_data[5] =  (byte) ((sleep_left >> 8) & 0xFF);
        send_data[6] = 0x00;
        send_data[7] =  0x00;
        send_data[8] = 0x00;
        send_data[9] =  0x00;
        send_data[10] = 0x0a;
        Message message=new Message();
        message.what=0;
        handler.sendMessage(message);
    }

    private boolean face_find = false;
    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
        face_find = b;
        if(b){
            mCameraFaceDetectionView.enableView();
        } else {
            mCameraFaceDetectionView.disableView();
        }
    }

    @Override
    public void onSensorChanged(SensorEvent event) {

        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
// x,y,z分别存储坐标轴x,y,z上的加速度
            float x = event.values[0];
            float y = event.values[1];
            float z = event.values[2];
// 根据三个方向上的加速度值得到总的加速度值a
            float a = (float) Math.sqrt(x * x + y * y + z * z);
            System.out.println("a---------->" + a);
// 传感器从外界采集数据的时间间隔为10000微秒
            System.out.println("magneticSensor.getMinDelay()-------->"
                    +magneticSensor.getMinDelay());
// 加速度传感器的最大量程
            System.out.println("event.sensor.getMaximumRange()-------->"
                    + event.sensor.getMaximumRange());

            System.out.println("x------------->" + x);
            System.out.println("y------------->" + y*10);
            System.out.println("z------------->" + z);

            Log.d("jarlen","x------------->" + (int)(x*10));
//            Log.d("jarlen","y------------>" + (int)(y*10));
//            Log.d("jarlen","z----------->" + z);
            int right = 0;
            int lrft = 0;

            y = y*20;
            x = x*20;
            if(y > 25) {
                Log.d("jt128","左转" + (y-15) );
                lrft = (int)(0-y-15);
            } else if(y < -25) {
                Log.d("jt128","右转" + (0-y-15));
                right = (int)(0-y-15);
            } else {
                if(x < -25) {
                    Log.d("jt128", "前景" + (0-x-15));
                    right = (int)(0-x-15);
                    lrft = (int)(0-x-15);
                } else {

                }
            }


//            send_data[0] = 0x3a;
//            send_data[1] = 0x02;
//            send_data[2] = (byte) ((int)right & 0xFF);
//            send_data[3] =  (byte) (((int)right >> 8) & 0xFF);
//            send_data[4] = (byte) ((int)lrft & 0xFF);
//            send_data[5] =  (byte) (((int)lrft >> 8) & 0xFF);
//            send_data[6] = 0x00;
//            send_data[7] =  0x00;
//            send_data[8] = 0x00;
//            send_data[9] =  0x00;
//            send_data[10] = 0x0a;
//            Message message=new Message();
//            message.what=0;
//            handler.sendMessage(message);

// showTextView.setText("x---------->" + x + "\ny-------------->" +
// y + "\nz----------->" + z);
        }else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
// 三个坐标轴方向上的电磁强度，单位是微特拉斯(micro-Tesla)，用uT表示，也可以是高斯(Gauss),1Tesla=10000Gauss
//            float x = event.values[0];
//            float y = event.values[1];
//            float z = event.values[2];
//// 手机的磁场感应器从外部采集数据的时间间隔是10000微秒
//            System.out.println("magneticSensor.getMinDelay()-------->"
//                    +magneticSensor.getMinDelay());
//// 磁场感应器的最大量程
//            System.out.println("event.sensor.getMaximumRange()----------->"
//                    + event.sensor.getMaximumRange());
//            System.out.println("lhbx------------->" + x);
//            System.out.println("lhby------------->" + y);
//            System.out.println("lhbz------------->" + z);
            //
            // Log.d("TAG","x------------->" + x);
            // Log.d("TAG", "y------------>" + y);
            // Log.d("TAG", "z----------->" + z);
            //
            // showTextView.setText("x---------->" + x + "\ny-------------->" +
            // y + "\nz----------->" + z);
        }else if (event.sensor.getType() == Sensor.TYPE_GYROSCOPE) {
            //从 x、y、z 轴的正向位置观看处于原始方位的设备，如果设备逆时针旋转，将会收到正值；否则，为负值
            if(timestamp != 0){
                // 得到两次检测到手机旋转的时间差（纳秒），并将其转化为秒
//                final float dT = (event.timestamp -timestamp) * NS2S;
//                // 将手机在各个轴上的旋转角度相加，即可得到当前位置相对于初始位置的旋转弧度
//                angle[0] += event.values[0] * dT;
//                angle[1] += event.values[1] * dT;
//                angle[2] += event.values[2] * dT;
//                // 将弧度转化为角度
//                float anglex = (float) Math.toDegrees(angle[0]);
//                float angley = (float) Math.toDegrees(angle[1]);
//                float anglez = (float) Math.toDegrees(angle[2]);
//
//                System.out.println("anglex------------>" + anglex);
//                System.out.println("angley------------>" + angley);
//                System.out.println("anglez------------>" + anglez);
//                System.out.println("gyroscopeSensor.getMinDelay()----------->" +
//                        gyroscopeSensor.getMinDelay());
            }
            //将当前时间赋值给timestamp
            timestamp = event.timestamp;

        }
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }
}
