//
//  main.cpp
//  OpenCVDemo1
//
//  Created by LeeWong on 2022/9/20.
//

#include <iostream>
//#include<opencv2/opencv.hpp>
//
//using namespace cv;
//using namespace std;


//int main(int argc, const char * argv[]) {
//    // insert code here...
//    std::cout << "Hello, World!\n";
//    string path = "/Users/LeeWong/Desktop/ef857c156jfd7c089fd06870eae8efff.JPG";
//    Mat image = imread(path);
//    namedWindow("WGG");
//    imshow("WGG", image);
//
//    Mat gray;
//    cvtColor(image, gray, COLOR_RGBA2GRAY);
//    namedWindow("gray");
//    imshow("gray", gray);
//    waitKey(0);
//    return 0;
//}

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    Mat src = imread("/Users/LeeWong/Downloads/photo_red.jpg");
    if (src.empty()) {
        printf("could not load image...\n");
        return -1;
    }
    imshow("原圖", src);

    // 1.將二維影象資料線性化
    Mat data;
    for (int i = 0; i < src.rows; i++)     //畫素點線性排列
        for (int j = 0; j < src.cols; j++)
        {
            Vec3b point = src.at<Vec3b>(i, j);
            Mat tmp = (Mat_<float>(1, 3) << point[0], point[1], point[2]);
            data.push_back(tmp);
        }

    // 2.使用K-means聚類；分離出背景色
    int numCluster = 4;
    Mat labels;
    TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1);
    kmeans(data, numCluster, labels, criteria, 3, KMEANS_PP_CENTERS);

    // 3.背景與人物二值化
    Mat mask = Mat::zeros(src.size(), CV_8UC1);
    int index = src.rows * 2 + 2;  //獲取點（2，2）作為背景色
    int cindex = labels.at<int>(index);
    /*  提取背景特徵 */
    for (int row = 0; row < src.rows; row++) {
        for (int col = 0; col < src.cols; col++) {
            index = row * src.cols + col;
            int label = labels.at<int>(index);
            if (label == cindex) { // 背景
                mask.at<uchar>(row, col) = 0;
            }
            else {
                mask.at<uchar>(row, col) = 255;
            }
        }
    }
    //imshow("mask", mask);

    // 4.腐蝕 + 高斯模糊：影象與背景交匯處高斯模糊化
    Mat k = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));
    erode(mask, mask, k);
    //imshow("erode-mask", mask);
    GaussianBlur(mask, mask, Size(3, 3), 0, 0);
    //imshow("Blur Mask", mask);

    // 5.更換背景色以及交匯處融合處理
    RNG rng(12345);
    Vec3b color;  //設定的背景色
    color[0] = 255;//rng.uniform(0, 255);
    color[1] = 255;// rng.uniform(0, 255);
    color[2] = 255;// rng.uniform(0, 255);
    Mat result(src.size(), src.type());

    double w = 0.0;   //融合權重
    int b = 0, g = 0, r = 0;
    int b1 = 0, g1 = 0, r1 = 0;
    int b2 = 0, g2 = 0, r2 = 0;

    for (int row = 0; row < src.rows; row++) {
        for (int col = 0; col < src.cols; col++) {
            int m = mask.at<uchar>(row, col);
            if (m == 255) {
                result.at<Vec3b>(row, col) = src.at<Vec3b>(row, col); // 前景
            }
            else if (m == 0) {
                result.at<Vec3b>(row, col) = color; // 背景
            }
            else {/* 融合處理部分 */
                w = m / 255.0;
                b1 = src.at<Vec3b>(row, col)[0];
                g1 = src.at<Vec3b>(row, col)[1];
                r1 = src.at<Vec3b>(row, col)[2];

                b2 = color[0];
                g2 = color[1];
                r2 = color[2];

                b = b1 * w + b2 * (1.0 - w);
                g = g1 * w + g2 * (1.0 - w);
                r = r1 * w + r2 * (1.0 - w);

                result.at<Vec3b>(row, col)[0] = b;
                result.at<Vec3b>(row, col)[1] = g;
                result.at<Vec3b>(row, col)[2] = r;
            }
        }
    }
    imshow("背景替換", result);

    waitKey(0);
    return 0;
}
//
//#include <opencv2/opencv.hpp>
//#include <opencv2/xfeatures2d.hpp>
//#include<opencv2/face.hpp>
//#include<iostream>
//#include<math.h>
//#include <string>
//#include<fstream>
//
//using namespace cv::face;
//using namespace cv;
//using namespace std;
//using namespace cv::xfeatures2d;
//
//int numRun = 0;//记录run了几次
//Rect rect;
//bool init = false;
//Mat src, mask, bgmodel, fgmodel;
//
//void showImage()//显示选择的前景区域
//{
//Mat result, binMask;
//binMask.create(mask.size(), CV_8UC1);
//binMask = mask & 1;//&=操作符重载
//if (init)//init后给result设置背景色，前景色
//{
//cout << "binMask depth=" << binMask.depth() << ",type=" << binMask.type() << endl;
//src.copyTo(result, binMask);
//}
//else
//{
//src.copyTo(result);
//}
//rectangle(result, rect, Scalar(0, 0, 255), 2, 8);//绘制红色的矩形框
//imshow("src", result);
//}
//
//void setROIMask()//设置背景 前景 区域
//{       // GC_FGD = 1       // 属于前景色的像素
//// GC_BGD =0;       // 属于背景色的像素
//// GC_PR_FGD = 3    // 可能属于前景的像素
//// GC_PR_BGD = 2    // 可能属于背景的像素
//mask.setTo(Scalar::all(GC_BGD));//设置为Grabcut的背景色
//rect.x = max(0, rect.x);//max min都是防止rect未初始化导致的差错
//rect.y = max(0, rect.y);
//rect.width = min(rect.width, src.cols - rect.x);
//rect.height = min(rect.height, src.rows - rect.y);
//mask(rect).setTo(Scalar(GC_PR_FGD));//rect区域设置为Grabcut的前景， mask(rect)获取的Mat也是浅拷贝，指针还是指向原mask矩阵
//}
//
//void onMouse(int event, int x, int y, int flags, void* param)//鼠标响应事件
//{
//switch (event)
//{
//case EVENT_LBUTTONDOWN://鼠标左键按下事件
//rect.x = x;
//rect.y = y;
//rect.width = 1;
//rect.height = 1;
//init = false;
//numRun = 0;
//break;
//case EVENT_MOUSEMOVE://鼠标移动事件
//if (flags&EVENT_FLAG_LBUTTON)//左键按下
//{
//rect = Rect(Point(rect.x, rect.y), Point(x, y));//随鼠标移动的矩形框  左上 右下
//showImage();
//}
//break;
//case EVENT_LBUTTONUP://鼠标左键抬起事件
//if (rect.width > 1 && rect.height > 1)
//{
//setROIMask();
//showImage();
//}
//break;
//default:
//break;
//}
//}
//
//void runGrabCut()// Grabcut抠图，算法耗时
//{
//if (rect.width < 2 || rect.height < 2)
//return;//框太小
//if (init)
//{
//grabCut(src, mask, rect, bgmodel, fgmodel, 1, GC_EVAL);//分割，抠图
//}
//else
//{
//grabCut(src, mask, rect, bgmodel, fgmodel, 1, GC_INIT_WITH_RECT);// 初始化，也有一定的图像分割的作用，但是上面的执行分割可以在此基础上更进一步的分割
//init = true;
//}
//}
//
//int main()
//{
//src = imread("/Users/LeeWong/Downloads/photo_red.jpg");
//mask.create(src.size(), CV_8UC1);
//mask.setTo(Scalar::all(GC_BGD));//背景为黑色
//    namedWindow("src", WINDOW_AUTOSIZE);
//setMouseCallback("src", onMouse, 0);
//imshow("src", src);
//
//while (true)
//{
//char c = (char)waitKey(0);
//if (c == 'b') // 按字母 b
//{
//runGrabCut();
//numRun++;
//showImage();
//printf("current iteative times : %d\n", numRun);
//}
//if ((int)c == 27) break;//esc
//}
//}

//#include <opencv2/opencv.hpp>
//#include <opencv2/xfeatures2d.hpp>
//#include<opencv2/face.hpp>
//#include<iostream>
//#include<math.h>
//#include <string>
//#include<fstream>
//
//using namespace cv::face;
//using namespace cv;
//using namespace std;
//using namespace cv::xfeatures2d;
//
//int  main() {
//Mat src = imread("/Users/LeeWong/Downloads/photo1.jpeg");
//imshow("src", src);
////组装数据
//
//int width = src.cols;
//int height = src.rows;
//int samplecount = width * height;
//int dims = src.channels();
////行数为src的像素点数，列数为通道数，每列数据分别为src的bgr，从上到下 从左到右顺序读数据
//Mat points(samplecount, dims, CV_32F, Scalar(10));
//int ind = 0;
//for (int row = 0; row < height; row++) {
//for (int col = 0; col < width; col++) {
//ind = row * width + col;//
//Vec3b bgr = src.at<Vec3b>(row, col);
//points.at<float>(ind, 0) = static_cast<int>(bgr[0]);
//points.at<float>(ind, 1) = static_cast<int>(bgr[1]);
//points.at<float>(ind, 2) = static_cast<int>(bgr[2]);
//}
//}
////运行kmeans
//int numCluster = 4;
//Mat labels;
//Mat centers;
//TermCriteria criteria = TermCriteria(TermCriteria::EPS + TermCriteria::COUNT, 10, 0.1);
//kmeans(points, numCluster, labels, criteria, 3, KMEANS_PP_CENTERS, centers);
////去背景+遮罩生成
//Mat mask = Mat::zeros(src.size(), CV_8UC1);
//int index = src.rows * 2 + 2;//不取边缘的左上点，往里靠2个位置
//int cindex = labels.at<int>(index, 0);
//int height1 = src.rows;
//int width1 = src.cols;
//Mat dst;//人的轮廓周围会有一些杂点，所以需要腐蚀和高斯模糊取干扰
//src.copyTo(dst);
//for (int row = 0; row < height1; row++) {
//for (int col = 0; col < width1; col++) {
//index = row * width1 + col;
//int label = labels.at<int>(index, 0);
//if (label == cindex) {
//dst.at<Vec3b>(row, col)[0] = 0;
//dst.at<Vec3b>(row, col)[1] = 0;
//dst.at<Vec3b>(row, col)[2] = 0;
//mask.at<uchar>(row, col) = 0;
//}
//else {
//dst.at<Vec3b>(row, col) = src.at<Vec3b>(row, col);
//mask.at<uchar>(row, col) = 255;//人脸部分设为白色，以便于下面的腐蚀与高斯模糊
//}
//}
//}
//imshow("dst", dst);
//imshow("mask", dst);
////腐蚀+高斯模糊
//Mat k = getStructuringElement(MORPH_RECT, Size(3, 3));
//erode(mask, mask, k);
//GaussianBlur(mask, mask, Size(3, 3), 0, 0);
//imshow("gaosimohu", mask);
////通道混合
//RNG rng(12345);
//Vec3b color;
//color[0] = 180;//rng.uniform(0, 255);
//color[1] =180;//rng.uniform(0, 255);
//color[2] =238;//rng.uniform(0, 255);
//Mat result(src.size(), src.type());
//
//double w = 0.0;
//int b = 0, g = 0, r = 0;
//int b1 = 0, g1 = 0, r1 = 0;
//int b2 = 0, g2 = 0, r2 = 0;
//
//double time = getTickCount();
//for (int row = 0; row < height1; row++) {
//for (int col = 0; col < width; col++) {
//int m = mask.at<uchar>(row, col);
//if (m == 255) {
//result.at<Vec3b>(row, col) = src.at<Vec3b>(row, col);//前景
//}
//else if (m == 0) {
//result.at<Vec3b>(row, col) = color; // 背景
//}
//else {//因为高斯模糊的关系，所以mask元素的颜色除了黑白色还有黑白边缘经过模糊后的非黑白值
//w = m / 255.0;
//b1 = src.at<Vec3b>(row, col)[0];
//g1 = src.at<Vec3b>(row, col)[1];
//r1 = src.at<Vec3b>(row, col)[2];
//b2 = color[0];
//g2 = color[0];
//r2 = color[0];
//
//b = b1 * w + b2 * (1.0 - w);
//g = g1 * w + g2 * (1.0 - w);
//r = r1 * w + r2 * (1.0 - w);
//
//result.at<Vec3b>(row, col)[0] = b;//最终边缘颜色值
//result.at<Vec3b>(row, col)[1] = g;
//result.at<Vec3b>(row, col)[2] = r;
//
//}
//}
//}
//cout << "time=" << (getTickCount() - time) / getTickFrequency() << endl;
//imshow("backgroud repalce", result);
//waitKey(0);
//}
