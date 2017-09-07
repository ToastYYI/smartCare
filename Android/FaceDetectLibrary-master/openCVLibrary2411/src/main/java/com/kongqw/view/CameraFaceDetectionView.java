package com.kongqw.view;

import android.content.Context;
import android.hardware.Camera;
import android.util.AttributeSet;
import android.util.Log;

import com.kongqw.interfaces.OnFaceDetectorListener;
import com.kongqw.interfaces.OnOpenCVInitListener;

import org.opencv.R;
import org.opencv.android.BaseLoaderCallback;
import org.opencv.android.CameraBridgeViewBase;
import org.opencv.android.JavaCameraView;
import org.opencv.android.LoaderCallbackInterface;
import org.opencv.android.OpenCVLoader;
import org.opencv.core.Core;
import org.opencv.core.Mat;
import org.opencv.core.MatOfRect;
import org.opencv.core.Point;
import org.opencv.core.Rect;
import org.opencv.core.Scalar;
import org.opencv.core.Size;
import org.opencv.objdetect.CascadeClassifier;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

import static org.bytedeco.javacpp.opencv_objdetect.CV_HAAR_DO_CANNY_PRUNING;
import static org.bytedeco.javacpp.opencv_objdetect.CV_HAAR_DO_ROUGH_SEARCH;
import static org.bytedeco.javacpp.opencv_objdetect.CV_HAAR_FIND_BIGGEST_OBJECT;
import static org.bytedeco.javacpp.opencv_objdetect.CV_HAAR_SCALE_IMAGE;

/**
 * Created by kqw on 2016/9/9.
 * CameraFaceDetectionView
 */
public class CameraFaceDetectionView extends JavaCameraView implements CameraBridgeViewBase.CvCameraViewListener2 {

    private static final String TAG = "RobotCameraView";
    private OnFaceDetectorListener mOnFaceDetectorListener;
    private OnOpenCVInitListener mOnOpenCVInitListener;
    private static final Scalar FACE_RECT_COLOR = new Scalar(0, 255, 0, 255);
    private CascadeClassifier mJavaDetector;
    // 记录切换摄像头点击次数
    private int mCameraSwitchCount = 0;

    private Mat mRgba;
    private Mat mGray;

    private int mAbsoluteFaceSize = 0;
    // 脸部占屏幕多大面积的时候开始识别
    private static final float RELATIVE_FACE_SIZE = 0.2f;

    public CameraFaceDetectionView(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    /**
     * 加载OpenCV
     *
     * @param context context
     * @return 是否安装了OpenCV
     */
    public boolean loadOpenCV(Context context) {
        // 初始化OpenCV
        boolean isLoaded = OpenCVLoader.initAsync(OpenCVLoader.OPENCV_VERSION_2_4_5, context, mLoaderCallback);

        if (isLoaded) {
            // OpenCV加载成功
            setCvCameraViewListener(this);
        } else {
            // 加载失败
            Log.i(TAG, "loadOpenCV: ----------------------------");
            Log.i(TAG, "loadOpenCV: " + "请先安装OpenCV Manager！ https://github.com/kongqw/KqwFaceDetectionDemo/tree/master/OpenCVManager");
            Log.i(TAG, "loadOpenCV: ----------------------------");
        }

        return isLoaded;
    }

    private boolean isLoadSuccess = false;
    private BaseLoaderCallback mLoaderCallback = new BaseLoaderCallback(getContext().getApplicationContext()) {

        @Override
        public void onManagerConnected(int status) {
            switch (status) {
                case LoaderCallbackInterface.SUCCESS:
                    Log.i(TAG, "onManagerConnected: OpenCV加载成功");
                    if (null != mOnOpenCVInitListener) {
                        mOnOpenCVInitListener.onLoadSuccess();
                    }
                    isLoadSuccess = true;
                    try {
                        InputStream is = getResources().openRawResource(R.raw.lbpcascade_frontalface);
                        File cascadeDir = getContext().getApplicationContext().getDir("cascade", Context.MODE_PRIVATE);
                        File cascadeFile = new File(cascadeDir, "lbpcascade_frontalface.xml");
                        FileOutputStream os = new FileOutputStream(cascadeFile);

                        byte[] buffer = new byte[4096];
                        int bytesRead;
                        while ((bytesRead = is.read(buffer)) != -1) {
                            os.write(buffer, 0, bytesRead);
                        }
                        is.close();
                        os.close();

                        mJavaDetector = new CascadeClassifier(cascadeFile.getAbsolutePath());
                        if (mJavaDetector.empty()) {
                            Log.e(TAG, "级联分类器加载失败");
                            mJavaDetector = null;
                        }

                    } catch (IOException e) {
                        e.printStackTrace();
                        Log.e(TAG, "没有找到级联分类器");
                    }
                    enableView();

                    break;
                case LoaderCallbackInterface.MARKET_ERROR: // OpenCV loader can not start Google Play Market.
                    Log.i(TAG, "onManagerConnected: 打开Google Play失败");
                    if (null != mOnOpenCVInitListener) {
                        mOnOpenCVInitListener.onMarketError();
                    }
                    break;
                case LoaderCallbackInterface.INSTALL_CANCELED: // Package installation has been canceled.
                    Log.i(TAG, "onManagerConnected: 安装被取消");
                    if (null != mOnOpenCVInitListener) {
                        mOnOpenCVInitListener.onInstallCanceled();
                    }
                    break;
                case LoaderCallbackInterface.INCOMPATIBLE_MANAGER_VERSION: // Application is incompatible with this version of OpenCV Manager. Possibly, a service update is required.
                    Log.i(TAG, "onManagerConnected: 版本不正确");
                    if (null != mOnOpenCVInitListener) {
                        mOnOpenCVInitListener.onIncompatibleManagerVersion();
                    }
                    break;
                default: // Other status,
                    Log.i(TAG, "onManagerConnected: 其他错误");
                    if (null != mOnOpenCVInitListener) {
                        mOnOpenCVInitListener.onOtherError();
                    }
                    // super.onManagerConnected(status);
                    break;
            }
        }
    };

    @Override
    public void enableView() {
        if (isLoadSuccess) {
            super.enableView();
        }
    }

    @Override
    public void disableView() {
        if (isLoadSuccess) {
            super.disableView();
        }
    }

    @Override
    public void onCameraViewStarted(int width, int height) {
        mGray = new Mat();
        mRgba = new Mat();
    }

    @Override
    public void onCameraViewStopped() {
        mGray.release();
        mRgba.release();
    }

    private ClickCallback callback;

    public void setClickCallback(ClickCallback callback) {
        this.callback = callback;
    }

    public interface ClickCallback{
        /**
         * 点击返回按钮回调
         */
        void Find_face(int l_x,int l_y,int x_x,int x_y);
    }

    @Override
    public Mat onCameraFrame(CvCameraViewFrame inputFrame) {
        // 子线程（非UI线程）
        mRgba = inputFrame.rgba();
        mGray = inputFrame.gray();//图像

        if (mAbsoluteFaceSize == 0) {
            int height = mGray.rows();
            if (Math.round(height * RELATIVE_FACE_SIZE) > 0) {
                mAbsoluteFaceSize = Math.round(height * RELATIVE_FACE_SIZE);
            }
        }
        String dr = "node";
        int l_x = 0;
        int l_y = 0;
        int x_x = 0;
        int x_y = 0;

        if (mJavaDetector != null) {
            MatOfRect faces = new MatOfRect();
            mJavaDetector.detectMultiScale(mGray, // 要检查的灰度图像
                    faces, // 检测到的人脸
                    1.1, // 表示在前后两次相继的扫描中，搜索窗口的比例系数。默认为1.1即每次搜索窗口依次扩大10%;
                    3, // 默认是3 控制误检测，表示默认几次重叠检测到人脸，才认为人脸存在
                    CV_HAAR_FIND_BIGGEST_OBJECT // 返回一张最大的人脸（无效？）
                            | CV_HAAR_SCALE_IMAGE
                            | CV_HAAR_DO_ROUGH_SEARCH
                            | CV_HAAR_DO_CANNY_PRUNING, //CV_HAAR_DO_CANNY_PRUNING ,// CV_HAAR_SCALE_IMAGE, // TODO: objdetect.CV_HAAR_SCALE_IMAGE
                    new Size(mAbsoluteFaceSize, mAbsoluteFaceSize),
                    new Size(mGray.width(), mGray.height()));
            // 检测到人脸
            Rect[] facesArray = faces.toArray();
            for (Rect aFacesArray : facesArray) {
                Core.rectangle(mRgba, aFacesArray.tl(), aFacesArray.br(), FACE_RECT_COLOR, 3);
                Rect aFacesArray2 = aFacesArray;
                aFacesArray2.y = aFacesArray.y-40;
                Core.putText(mRgba,aFacesArray.tl().toString() + aFacesArray.br().toString(),aFacesArray2.tl(),2,1,FACE_RECT_COLOR);

                if(aFacesArray.tl().x > 250) {
                    dr = "right";
                }

                if(aFacesArray.tl().x < 230) {
                    dr = "lift";
                }
                l_x = (int) aFacesArray.tl().x;
                l_y = (int) aFacesArray.tl().y;
                x_x = (int) aFacesArray.br().x;
                x_y = (int) aFacesArray.br().y;
                if (null != mOnFaceDetectorListener) {
                    mOnFaceDetectorListener.onFace(mRgba, aFacesArray);
                }
            }
        }
//        int threshold = 150;
//        int minLineSize = 50;
//        int lineGap = 35;
//        double minYgap = 30;
//        int number=0;
//
//        double average_x1=0;
//        double average_y1=0;
//        double average_x2=0;
//        double average_y2=0;
//        String mMsg ="";
//        //将之前的信息清空，这里很重要，之前未清除导致严重BUG
//
//        Mat mErodeElement = Imgproc.getStructuringElement(Imgproc.MORPH_RECT, new Size(10,25));
//        Mat mDilateElement = Imgproc.getStructuringElement(Imgproc.MORPH_RECT, new Size(10,25));
//        Mat mShow = new Mat();
//        Mat lines = new Mat();
//        List<Line> mLines = new ArrayList<Line>();
//        Line[] twoLines = new Line[2];
//        //将摄像头得到的RGB图像转换为灰度图像，方便后续的其他算法处理
//        Imgproc.cvtColor(mRgba, mRgba, Imgproc.COLOR_BGRA2GRAY, 4);
//        //进行高斯滤波
//        Imgproc.GaussianBlur(mRgba, mRgba, new Size(5,5), 2.2,2);
//        //形态学中的腐蚀膨胀变换去除干扰的直线边缘，根据不同情况来设置结构元素mDilateElement和mErodeElement
//        Imgproc.dilate(mRgba, mRgba, mDilateElement);
//        Imgproc.erode(mRgba, mRgba, mErodeElement);
//        // 利用Canny算子得到道路边缘
//        Imgproc.Canny(mRgba, mRgba,50, 50);
//        //mShow是用来显示的RGB图像
//        Imgproc.cvtColor(mRgba, mShow, Imgproc.COLOR_GRAY2BGRA, 4);
//        //利用Hough变换得到直线组，HoughLinesP是HoughLines的改进版，得到的直线组是以起始点和终点坐标来表示
//        Imgproc.HoughLinesP(mRgba, lines, 1, Math.PI/180, threshold,minLineSize,lineGap);
//
//        for (int x = 0; x < lines.cols(); x+=1)
//        {
//            double[] vec = lines.get(0, x);
//            if(vec!=null) {
//                double x1 = vec[0],
//                        y1 = vec[1],
//                        x2 = vec[2],
//                        y2 = vec[3];
//                Point start = new Point(x1, y1);
//                Point end = new Point(x2, y2);
//
//                if(!(Math.abs(x2-x1)<minYgap) ){
//
//                    Line mSingleline = new Line(start, end) ;
//                    mLines.add(mSingleline);
//                    number++;
//                    average_x1+=x1;
//                    average_y1+=y1;
//                    average_x2+=x2;
//                    average_y2+=y2;
//
//                    Core.line(mShow, start, end, new Scalar(255,0,0,255), 3);
//                }
//            }
//        }
//
//        if(number!=0) {
//
//            average_x1/=number;
//            average_y1/=number;
//            average_x2/=number;
//            average_y2/=number;
//            double angle = MathUtils.getAngle(mLines);
//
//            // Log.i(TAG,"angle"+angle);
//
//            if(Math.abs(angle)<10) {
//
//                Line midLine = new Line(new Point(average_x1,average_y1),new Point(average_x2,average_y2));
//                mLines.add(midLine);
//
//                Collections.sort(mLines, new SortedPoint());
//
//                twoLines = MathUtils.getTwoLines(mLines,mLines.lastIndexOf(midLine));
//                if(twoLines[0].start!=null&&twoLines[0].end!=null&&twoLines[1].start!=null&&twoLines[1].end!=null) {
//                    Point pStart = new Point((twoLines[0].start.x+twoLines[1].start.x)/2, (twoLines[0].start.y+twoLines[1].start.y)/2);
//                    Point pEnd = new Point((twoLines[0].end.x+twoLines[1].end.x)/2, (twoLines[0].end.y+twoLines[1].end.y)/2);
//
//                    midLine = new Line(pStart, pEnd);
//                    //Log.i(TAG,number+"::::::"+average_x1+":"+average_y1+";"+average_x2+":"+average_y2);
//                    Core.line(mShow, twoLines[0].start, twoLines[0].end, new Scalar(255,255,0,100), 15);
//                    Core.line(mShow, twoLines[1].start, twoLines[1].end, new Scalar(255,255,0,100), 15);
//
//                    Core.line(mShow, midLine.start, midLine.end, new Scalar(255,0,255,100), 15);
//                    double distance = (midLine.start.y+midLine.end.y)/2-360;
//                    if(distance<0) {
//                        mMsg = "在道路中心线左侧"+(int)(-distance)+"个单位,需要向右侧调整";
//                    }else {
//                        mMsg = "在道路中心线右侧"+(int)distance+"个单位,需要向左侧调整";
//                    }
//                    // Core.putText(mShow,mMsg, new Point(0, 360), 2, 2, new Scalar(255,255,255,0),1);
//                }else {
//                    mMsg = "道路消失";
//                    // Core.putText(mShow,mMsg, new Point(0, 360), 2, 2, new Scalar(255,255,255,0),1);
//                }
//            }else {
//
//                if(angle>0) {
//                    mMsg = "偏向左侧"+(int)angle+"度，需要向右侧偏转";
//                }else {
//                    mMsg = "偏向右侧"+(int)(-angle)+"度，需要向左侧偏转";
//                }
//                //Core.putText(mShow,mMsg, new Point(0, 360), 3, 2, new Scalar(255,255,0,0),2);
//            }
//        }
//        Log.i(TAG, "" + mMsg);
        callback.Find_face(l_x,l_y,x_x,x_y);
        Core.putText(mRgba,dr,new Point(10,60),2,1,FACE_RECT_COLOR);
        return mRgba;
    }

    /**
     * 切换摄像头
     *
     * @return 切换摄像头是否成功
     */
    public boolean switchCamera() {
        // 摄像头总数
        int numberOfCameras = 0;
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.GINGERBREAD) {
            numberOfCameras = Camera.getNumberOfCameras();
        }
        // 2个及以上摄像头
        if (1 < numberOfCameras) {
            // 设备没有摄像头
            int index = ++mCameraSwitchCount % numberOfCameras;
            disableView();
            setCameraIndex(index);
            enableView();
            return true;
        }
        return false;
    }

    /**
     * 添加人脸识别额监听
     *
     * @param listener 回调接口
     */
    public void setOnFaceDetectorListener(OnFaceDetectorListener listener) {
        mOnFaceDetectorListener = listener;
    }

    /**
     * 添加加载OpenCV的监听
     *
     * @param listener 回调接口
     */
    public void setOnOpenCVInitListener(OnOpenCVInitListener listener) {
        mOnOpenCVInitListener = listener;
    }
}
