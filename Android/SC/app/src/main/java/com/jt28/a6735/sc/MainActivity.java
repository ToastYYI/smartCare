package com.jt28.a6735.sc;

import android.Manifest;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothDevice;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.LocationManager;
import android.os.Bundle;
import android.provider.Settings;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ListView;
import android.widget.Toast;

import com.jt28.a6735.h_ble.BleController;
import com.jt28.a6735.h_ble.callback.ConnectCallback;
import com.jt28.a6735.h_ble.callback.ScanCallback;
import com.jt28.a6735.sc.adapter.DeviceListAdapter;

import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity implements AdapterView.OnItemClickListener {

    private ProgressDialog progressDialog;
    private BleController mBleController;
    private static String LOGTAG = "AppCompatActivity";
    //搜索结果列表
    private List<BluetoothDevice> bluetoothDevices = new ArrayList<BluetoothDevice>();
    private ListView mDeviceList;

    // gps是否可用
    public static final boolean isGpsEnable(final Context context) {
        LocationManager locationManager
                = (LocationManager) context.getSystemService(Context.LOCATION_SERVICE);
        boolean gps = locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER);
        boolean network = locationManager.isProviderEnabled(LocationManager.NETWORK_PROVIDER);
        if (gps || network) {
            return true;
        }
        return false;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mDeviceList = (ListView) findViewById(R.id.mDeviceList);

        //Android6.0需要动态申请权限
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {
            //请求权限
            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION,
                            Manifest.permission.ACCESS_FINE_LOCATION},
                    3);
            if (ActivityCompat.shouldShowRequestPermissionRationale(this,
                    Manifest.permission.ACCESS_COARSE_LOCATION)) {
                //判断是否跟用户做一个说明
               // DialogUtils.shortT(getApplicationContext(), "需要蓝牙权限");
            }
        }

        if(isGpsEnable(this)) {
            // TODO  第一步：初始化
            mBleController = BleController.getInstance().init(this);
            // TODO  第二步：搜索设备，获取列表后进行展示
            scanDevices();
        } else {
            //跳转到gps设置页
            Intent intent = new Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS);
            startActivityForResult(intent, 0);
        }
    }

    private void scanDevices() {

        showProgressDialog("请稍后", "正在搜索设备");

        mBleController.scanBle(0, new ScanCallback() {
            @Override
            public void onSuccess() {
                hideProgressDialog();
                if (bluetoothDevices.size() > 0) {
                    mDeviceList.setAdapter(new DeviceListAdapter(MainActivity.this, bluetoothDevices));
                    mDeviceList.setOnItemClickListener(MainActivity.this);
                } else {
                    Toast.makeText(MainActivity.this, "未搜索到Ble设备", Toast.LENGTH_SHORT).show();
                }
            }

            @Override
            public void onScanning(BluetoothDevice device, int rssi, byte[] scanRecord) {
                if (!bluetoothDevices.contains(device)) {
                    bluetoothDevices.add(device);
                }
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        hideProgressDialog();
    }

    public void showProgressDialog(String title, String message) {
        if (progressDialog == null) {
            progressDialog = ProgressDialog.show(this, title, message, true, false);
        } else if (progressDialog.isShowing()) {
            progressDialog.setTitle(title);
            progressDialog.setMessage(message);
        }
        progressDialog.show();
    }

    public void hideProgressDialog() {
        if (progressDialog != null && progressDialog.isShowing()) {
            progressDialog.dismiss();
            progressDialog = null;
        }
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
        showProgressDialog("请稍后", "正在连接设备");

        // TODO 第三步：点击条目后,获取地址，根据地址连接设备
        String address = bluetoothDevices.get(i).getAddress();
        mBleController.connect(0, address, new ConnectCallback() {
            @Override
            public void onConnSuccess() {
                hideProgressDialog();
                Toast.makeText(MainActivity.this, "连接成功", Toast.LENGTH_SHORT).show();
                startActivity(new Intent(MainActivity.this,SendAndReciveActivity.class));
            }

            @Override
            public void onConnFailed() {
                hideProgressDialog();
                Toast.makeText(MainActivity.this, "连接超时，请重试", Toast.LENGTH_SHORT).show();
            }
        });
    }
}
