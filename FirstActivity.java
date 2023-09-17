package com.car.Activity;

import android.animation.ObjectAnimator;
import android.annotation.SuppressLint;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import com.bkrcl.control_car_video.camerautil.CameraCommandUtil;
import com.car.DataProcessingModule.ConnectTransport;
import com.car.Fragment.CtrlCameraFragment;
import com.car.MessageBean.DataRefreshBean;
import com.car.R;
import com.car.Utils.CameraUtile.XcApplication;
import com.car.Utils.OtherUtil.CameraConnectUtil;
import com.car.Utils.OtherUtil.RadiusUtil;
import com.car.Utils.OtherUtil.ToastUtil;
import com.car.Utils.OtherUtil.Transparent;
import com.car.Utils.OtherUtil.WiFiStateUtil;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.HexDump;
import com.hoho.android.usbserial.util.SerialInputOutputManager;
import com.task.imageUtils.ImageUtils;
import com.task.tasks.QR;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.opencv.android.Utils;
import org.opencv.core.Mat;
import org.opencv.imgproc.Imgproc;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;


public class FirstActivity extends AppCompatActivity implements CtrlCameraFragment.CameraCtrl, QR.CameraCtrl{
    public static ToastUtil toastUtil;
    private TextView psStatusTV, codedDiskTV, lightTV, ultraSonicTV;
    public static ConnectTransport Connect_Transport;
    // 设备ip
    public static String IPCar;
    // 摄像头工具
    public static CameraCommandUtil cameraCommandUtil;
    // 摄像头IP
    public static String IPCamera = null;
    public static String purecameraip = null;
    public static Handler recvhandler = null;
    private CameraConnectUtil cameraConnectUtil;
    private ImageView image_show = null, image_canny = null;
    private ImageButton refershImageButton;
    private Button reference_Btn;
    private static TextView showip = null;
    String Camera_show_ip = null;
    private float x1 = 0;
    private float y1 = 0;
    private boolean dateGetState = true; // 主从车接收状态切换
    // 图片
    public static Bitmap bitmap;



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
                WindowManager.LayoutParams.FLAG_FULLSCREEN);
        setContentView(R.layout.activity_first);
        EventBus.getDefault().register(this);
        initView();
        initAll();
        cameraCommandUtil = new CameraCommandUtil();
        cameraConnectUtil = new CameraConnectUtil(this);
        CtrlCameraFragment.setCameraCtrlInterface(this);
        QR.setCameraCtrlInterface(this);

        getCameraPic();
        if (XcApplication.isserial == XcApplication.Mode.SOCKET && !IPCamera.equals("null:81")) {
            setCameraConnectState(true);
            showip.setText("WiFi-IP：" + FirstActivity.IPCar + "\n" + "Camera-IP:" + FirstActivity.purecameraip);
        } else if (XcApplication.isserial == XcApplication.Mode.SOCKET && IPCamera.equals("null:81")) {
            showip.setText("WiFi-IP：" + FirstActivity.IPCar + "\n" + "请重启您的平台设备！");
        }

    }

    /**
     * 接收平台连接相关的Eventbus消息
     * @param refresh
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThreadx(DataRefreshBean refresh) {
        if (refresh.getRefreshState() == 1 && new WiFiStateUtil(this).wifiInit()) {
            // connect_Open();  //官方链接代码
        } else if (refresh.getRefreshState() == 3) {
            toastUtil.ShowToast("平台已连接");
        } else if (refresh.getRefreshState() == 4) {
            toastUtil.ShowToast("平台连接失败！");
        } else toastUtil.ShowToast("请检查WiFi连接状态！");

    }

    private void getCameraPic() {
        XcApplication.executorServicetor.execute(new Runnable() {
            @Override
            public void run() {
                if (IPCamera.equals("null:81")) return;
                while (true) {
                    getBitmap();
                }
            }
        });
    }

    // 得到当前摄像头的图片信息
    public void getBitmap() {
        setBitmap(cameraCommandUtil.httpForImage(IPCamera));
        if (bitmap != null) {
            //处理
            setBitmap(RadiusUtil.roundBitmapByXfermode(bitmap, bitmap.getWidth(), bitmap.getHeight(), 8));
            if (cannySwitch) { // 是否打开Canny开关
                Mat mMat_src, mMat_edge, mMat_canny;
                Bitmap mBm_canny;

                Mat tmp = new Mat();
                Utils.bitmapToMat(bitmap, tmp);

                mMat_src = new Mat();
                Imgproc.cvtColor(tmp, mMat_src, Imgproc.COLOR_RGBA2BGR); // RGBA转BGR

                Mat gray = new Mat();
                Imgproc.cvtColor(mMat_src, gray, Imgproc.COLOR_BGR2GRAY); // 灰度化

                mMat_edge = ImageUtils.laplace(gray); // 边缘增强

                mMat_canny = new Mat();
                Imgproc.Canny(mMat_edge, mMat_canny, canny1, canny2); //　Canny检测

                mBm_canny = Bitmap.createBitmap(mMat_edge.width(), mMat_edge.height(), Bitmap.Config.ARGB_8888);
                Utils.matToBitmap(mMat_canny, mBm_canny);
                cannyBitmap = mBm_canny; // 赋值给cannyBitmap，等待phHandler更新界面，见本文件第180行
            }
        } else {
            setCameraConnectState(false);
        }
        phHandler.sendEmptyMessage(10);
    }
    Boolean cannySwitch = false;
    Bitmap cannyBitmap;
    int canny1 = 10;
    int canny2 = 10;
    // 显示图片
    @SuppressLint("HandlerLeak")
    public Handler phHandler = new Handler() { // pHandler设置为公有，方便TaskFragment访问
        public void handleMessage(Message msg) {
            if (msg.what == 10) {
                image_show.setImageBitmap(bitmap);
                if (cannySwitch) // 显示Canny结果
                    image_canny.setImageBitmap(cannyBitmap);
            } else if (msg.what == 11) {

            } else if (msg.what == 12) { // 12, 13, 14, 15 来自TaskFragment控件回调
                image_canny.setVisibility(View.VISIBLE);
                cannySwitch = true;
            } else if (msg.what == 13) {
                image_canny.setVisibility(View.INVISIBLE);
                cannySwitch = false;
            } else if (msg.what == 14) {
                canny1 = (int)msg.obj;
            } else if (msg.what == 15) {
                canny2 = (int)msg.obj;
            }
        }
    };
    // 摄像头连接状态，默认为true
    private boolean cameraConnectState = true;

    public boolean isCameraConnectState() {
        return cameraConnectState;
    }

    public void setCameraConnectState(boolean cameraConnectState) {
        this.cameraConnectState = cameraConnectState;
    }
    public static void setBitmap(Bitmap bitmap) {
        FirstActivity.bitmap = bitmap;
    }


    private void initView() {
        image_show = findViewById(R.id.img);
        image_canny = findViewById(R.id.canny_imageView);
        refershImageButton = findViewById(R.id.refresh_img_btn);
        reference_Btn = findViewById(R.id.refresh_btn);

        reference_Btn.setOnClickListener(v -> ObjectrotationAnim(refershImageButton));
        refershImageButton.setOnClickListener(v -> ObjectrotationAnim(refershImageButton));
        showip = (TextView) findViewById(R.id.showip);
        image_show.setOnTouchListener(new ontouchlistener1());
    }

    private void initAll(){
        toastUtil = new ToastUtil(this);
        if (XcApplication.isserial == XcApplication.Mode.USB_SERIAL) {  //竞赛平台和a72通过usb转串口通信
            mHandler.sendEmptyMessageDelayed(MESSAGE_REFRESH, REFRESH_TIMEOUT_MILLIS); //启动usb的识别和获取
            Transparent.showLoadingMessage(this, "正在拼命追赶串口……", false);//启动旋转效果的对话框，实现usb的识别和获取
        }

        Connect_Transport = new ConnectTransport();    //实例化连接类
        cameraConnectUtil = new CameraConnectUtil(this);
    }

    /* 可尝试用线程发送，这样就不用写回调了 */
    @Override
    public void callBackCameraCtrl(int pos) {
        if (pos == 1) {                // up

            XcApplication.executorServicetor.execute(new Runnable() {
                @Override
                public void run() {
                    cameraCommandUtil.postHttp(IPCamera, 0, 1);  //上
                }
            });
        } else if (pos == 2) {        // down
            toastUtil.ShowToast("向下微调");
            XcApplication.executorServicetor.execute(new Runnable() {
                @Override
                public void run() {
                    cameraCommandUtil.postHttp(IPCamera, 2, 1);  //下
                }
            });
        } else if (pos == 3) {        // left
            toastUtil.ShowToast("向左微调");
            XcApplication.executorServicetor.execute(new Runnable() {
                @Override
                public void run() {
                    cameraCommandUtil.postHttp(IPCamera, 4, 1);  //左
                }
            });
        } else if (pos == 4) {       // right
            toastUtil.ShowToast("向右微调");
            XcApplication.executorServicetor.execute(new Runnable() {
                @Override
                public void run() {
                    cameraCommandUtil.postHttp(IPCamera, 6, 1);  //右
                }
            });
        }
    }

    private class ontouchlistener1 implements View.OnTouchListener {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            // TODO 自动生成的方法存根
            switch (event.getAction() & MotionEvent.ACTION_MASK) {
                // 点击位置坐标
                case MotionEvent.ACTION_DOWN:
                    x1 = event.getX();
                    y1 = event.getY();
                    break;
                // 弹起坐标
                case MotionEvent.ACTION_UP:
                    float x2 = event.getX();
                    float y2 = event.getY();
                    float xx = x1 > x2 ? x1 - x2 : x2 - x1;
                    float yy = y1 > y2 ? y1 - y2 : y2 - y1;
                    // 判断滑屏趋势
                    int MINLEN = 30;
                    if (xx > yy) {
                        if ((x1 > x2) && (xx > MINLEN)) {        // left
                            toastUtil.ShowToast("向左微调");
                            XcApplication.executorServicetor.execute(new Runnable() {
                                @Override
                                public void run() {
                                    cameraCommandUtil.postHttp(IPCamera, 4, 1);  //左
                                }
                            });

                        } else if ((x1 < x2) && (xx > MINLEN)) { // right
                            toastUtil.ShowToast("向右微调");
                            XcApplication.executorServicetor.execute(new Runnable() {
                                @Override
                                public void run() {
                                    cameraCommandUtil.postHttp(IPCamera, 6, 1);  //右
                                }
                            });
                        }
                    } else {
                        if ((y1 > y2) && (yy > MINLEN)) {        // up
                            toastUtil.ShowToast("向上微调");
                            XcApplication.executorServicetor.execute(new Runnable() {
                                @Override
                                public void run() {
                                    cameraCommandUtil.postHttp(IPCamera, 0, 1);  //上
                                }
                            });
                        } else if ((y1 < y2) && (yy > MINLEN)) { // down
                            toastUtil.ShowToast("向下微调");
                            XcApplication.executorServicetor.execute(new Runnable() {
                                @Override
                                public void run() {
                                    cameraCommandUtil.postHttp(IPCamera, 2, 1);  //下
                                }
                            });
                        }
                    }
                    x1 = 0;
                    x2 = 0;
                    y1 = 0;
                    y2 = 0;

                    break;
            }
            return true;
        }
    }

    /**
     * 刷新按钮实现顺时针360度
     *
     * @param view
     */
    private void ObjectrotationAnim(View view) {
        //构造ObjectAnimator对象的方法
        EventBus.getDefault().post(new DataRefreshBean(1));
        ObjectAnimator animator = ObjectAnimator.ofFloat(view, "rotation", 0.0F, 360.0F);// 设置顺时针360度旋转
        animator.setDuration(1500);//设置旋转时间
        animator.start();//开始执行动画（顺时针旋转动画）
    }
    /**
     * 接收Eventbus消息
     *
     * @param refresh
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEventMainThread(DataRefreshBean refresh) {
         if (refresh.getRefreshState() == 4) {
        }
    }


    //------------------------------------------------------------------------------------------
    //获取和实现usb转串口的通信，实现A72和竞赛平台的串口通信 Android72
    public static UsbSerialPort sPort = null;

    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();

    private SerialInputOutputManager mSerialIoManager;

    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {

                @Override
                public void onRunError(Exception e) {
                    Log.e(TAG, "Runner stopped.");
                }

                @Override
                public void onNewData(final byte[] data) {   //新的数据
                    FirstActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            Message msg = recvhandler.obtainMessage(1, data);
                            msg.sendToTarget();
                            FirstActivity.this.updateReceivedData(data);
                        }
                    });
                }
            };

    protected void controlusb() {
        Log.e(TAG, "Resumed, port=" + sPort);
        if (sPort == null) {
            toastUtil.ShowToast("没有串口驱动！");
        } else {
            openUsbDevice();
            if (connection == null) {
                mHandler.sendEmptyMessageDelayed(MESSAGE_REFRESH, REFRESH_TIMEOUT_MILLIS);
                toastUtil.ShowToast("串口驱动失败！");
                return;
            }
            try {
                sPort.open(connection);
                sPort.setParameters(115200, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);
            } catch (IOException e) {
                toastUtil.ShowToast("串口驱动错误！");
                try {
                    sPort.close();
                } catch (IOException e2) {
                }
                sPort = null;
                return;
            }
        }
        onDeviceStateChange();
        Transparent.dismiss();//关闭加载对话框
    }

    // 在打开usb设备前，弹出选择对话框，尝试获取usb权限
    private void openUsbDevice() {
        tryGetUsbPermission();
    }

    private static final String ACTION_USB_PERMISSION = "com.android.example.USB_PERMISSION";
    private UsbDeviceConnection connection;

    private void tryGetUsbPermission() {

        IntentFilter filter = new IntentFilter(ACTION_USB_PERMISSION);
        registerReceiver(mUsbPermissionActionReceiver, filter);
        PendingIntent mPermissionIntent = PendingIntent.getBroadcast(this, 0, new Intent(ACTION_USB_PERMISSION), 0);

        //here do emulation to ask all connected usb device for permission
        for (final UsbDevice usbDevice : mUsbManager.getDeviceList().values()) {
            //add some conditional check if necessary
            if (mUsbManager.hasPermission(usbDevice)) {
                //if has already got permission, just goto connect it
                //that means: user has choose yes for your previously popup window asking for grant perssion for this usb device
                //and also choose option: not ask again
                afterGetUsbPermission(usbDevice);
            } else {
                //this line will let android popup window, ask user whether to allow this app to have permission to operate this usb device
                mUsbManager.requestPermission(usbDevice, mPermissionIntent);
            }
        }
    }

    private void afterGetUsbPermission(UsbDevice usbDevice) {

        toastUtil.ShowToast("Found USB device: VID=" + usbDevice.getVendorId() + " PID=" + usbDevice.getProductId());
        doYourOpenUsbDevice(usbDevice);
    }

    private void doYourOpenUsbDevice(UsbDevice usbDevice) {
        connection = mUsbManager.openDevice(usbDevice);
    }

    private final BroadcastReceiver mUsbPermissionActionReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (ACTION_USB_PERMISSION.equals(action)) {
                synchronized (this) {
                    UsbDevice usbDevice = (UsbDevice) intent.getParcelableExtra(UsbManager.EXTRA_DEVICE);
                    if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                        //user choose YES for your previously popup window asking for grant perssion for this usb device
                        if (null != usbDevice) {
                            afterGetUsbPermission(usbDevice);
                        }
                    } else {
                        //user choose NO for your previously popup window asking for grant perssion for this usb device
                        toastUtil.ShowToast("Permission denied for device" + usbDevice);
                    }
                }
            }
        }
    };

    private void stopIoManager() {
        if (mSerialIoManager != null) {
            Log.e(TAG, "Stopping io manager ..");
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if (sPort != null) {
            Log.e(TAG, "Starting io manager ..");
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener); //添加监听
            mExecutor.submit(mSerialIoManager); //在新的线程中监听串口的数据变化
        }
    }

    private void onDeviceStateChange() {
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {
        final String message = "Read " + data.length + " bytes: \n"
                + HexDump.dumpHexString(data) + "\n\n";
        //  Log.e("read data is ：：","   "+message);

    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        EventBus.getDefault().unregister(this);
        cameraConnectUtil.destroy();
        if (XcApplication.isserial == XcApplication.Mode.USB_SERIAL) {
            try {
                unregisterReceiver(mUsbPermissionActionReceiver);
                sPort.close();
            } catch (IOException e) {
                e.printStackTrace();
            } catch (IllegalArgumentException ignored) {

            }
            sPort = null;
        } else if (XcApplication.isserial == XcApplication.Mode.SOCKET) {
            Connect_Transport.destory();
        }
    }

    private static final int MESSAGE_REFRESH = 101;
    private static final long REFRESH_TIMEOUT_MILLIS = 5000;
    private UsbManager mUsbManager;
    private List<UsbSerialPort> mEntries = new ArrayList<UsbSerialPort>();
    private final String TAG = FirstActivity.class.getSimpleName();

    @SuppressLint("HandlerLeak")
    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MESSAGE_REFRESH:
                    refreshDeviceList();
                    break;
                default:
                    super.handleMessage(msg);
                    break;
            }
        }
    };

    @SuppressLint("HandlerLeak")
    private Handler usbHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == 2) {
                try {
                    useUsbtoserial();
                } catch (IndexOutOfBoundsException e) {
                    Transparent.dismiss();//关闭加载对话框
                    toastUtil.ShowToast("串口通信失败，请检查设备连接状态！");
                }
            }
        }
    };
    @SuppressLint("HandlerLeak")
    public static Handler showidHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            if (msg.what == 22) {
                showip.setText(msg.obj + "\n" + "Camera-IP：" + IPCamera);
            }
        }
    };
    private void useUsbtoserial() {
        final UsbSerialPort port = mEntries.get(0);  //A72上只有一个 usb转串口，用position =0即可
        final UsbSerialDriver driver = port.getDriver();
        final UsbDevice device = driver.getDevice();
        final String usbid = String.format("Vendor %s  ，Product %s",
                HexDump.toHexString((short) device.getVendorId()),
                HexDump.toHexString((short) device.getProductId()));
        Message msg = showidHandler.obtainMessage(22, usbid);
        msg.sendToTarget();
        FirstActivity.sPort = port;
        if (sPort != null) {
            controlusb();  //使用usb功能
        }
    }

    @SuppressLint("StaticFieldLeak")
    private void refreshDeviceList() {
        mUsbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
        new AsyncTask<Void, Void, List<UsbSerialPort>>() {
            @Override
            protected List<UsbSerialPort> doInBackground(Void... params) {
                Log.e(TAG, "Refreshing device list ...");
                Log.e("mUsbManager is :", "  " + mUsbManager);
                final List<UsbSerialDriver> drivers =
                        UsbSerialProber.getDefaultProber().findAllDrivers(mUsbManager);

                final List<UsbSerialPort> result = new ArrayList<UsbSerialPort>();
                for (final UsbSerialDriver driver : drivers) {
                    final List<UsbSerialPort> ports = driver.getPorts();
                    Log.e(TAG, String.format("+ %s: %s port%s",
                            driver, Integer.valueOf(ports.size()), ports.size() == 1 ? "" : "s"));
                    result.addAll(ports);
                }
                return result;
            }

            @Override
            protected void onPostExecute(List<UsbSerialPort> result) {
                mEntries.clear();
                mEntries.addAll(result);
                usbHandler.sendEmptyMessage(2);
                Log.e(TAG, "Done refreshing, " + mEntries.size() + " entries found.");
            }
        }.execute((Void) null);
    }

}
