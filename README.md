# Mac 配置C++ OpenCV环境

今天公司需求有部分要涉及到对图片的处理，因此需要集成OpenCV, 本篇文章为记录集成OpenCV环境的过程


## 安装OpenCV

```
brew install opencv
```

`注意`:请务必找一个稳定的vpn使用，否则既耗时又费精力

## 新建Xcode工程

### 新建一个xcode工程

![command line tool](https://tva1.sinaimg.cn/large/008vxvgGly1h75pxm7p4qj314k0swwhc.jpg)

### 新建一个lib文件夹，导入库文件

直接通过右键，Add Files to "Lib" 方法导入，导入lib文件夹目录为 `/usr/local/Cellar/opencv/4.5.3/lib/`下的文件，这里先不用考虑太多直接全部导入即可。

### 设置配置文件

- 设置 HEADER_SEARCH_PATHS = /usr/local/Cellar/opencv/4.5.3/include/opencv4
- 设置 LIBRARY_SEARCH_PATHS = /usr/local/Cellar/opencv/4.5.3/lib、/usr/local/Cellar/opencv/4.5.3/lib/opencv4/3rdparty

![](https://tva1.sinaimg.cn/large/008vxvgGly1h75qclosyoj31a008wwfg.jpg)


## 运行代码

```
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    Mat src = imread("/Users/LeeWong/Downloads/photo_red.jpeg");
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
```

运行代码后，刚好可以看到咱们初步调研的抠图代码，下面是运行效果：

![运行结果](https://tva1.sinaimg.cn/large/008vxvgGly1h75qlv03isj31a20u0wiu.jpg)

## 参考文档

[cmake error 'the source does not appear to contain CMakeLists.txt'](https://stackoverflow.com/questions/46448682/cmake-error-the-source-does-not-appear-to-contain-cmakelists-txt)

[Mac上opencv的安装和Xcode c++的配置](https://crysple.github.io/2018/04/04/2018-04-03/)

[Mac下安装opencv，以及xcode的调用添](https://www.361shipin.com/blog/1547348358195052544)

[Homebrew 安裝 OpenCV4.5.0搭配 Xcode GUI](https://medium.com/paulchous-fantasy-world/homebrew-%E5%AE%89%E8%A3%9D-opencv4-5-0%E6%90%AD%E9%85%8D-xcode-gui-3106af09fe36)

