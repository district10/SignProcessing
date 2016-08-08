**【[qt4cv3vs2015](https://github.com/district10/SignProcessing/tree/qt4cv3vs2015) 分支包含了编译和使用说明。请先从那里开始。】**

---

# SignProcessing

利用 Qt、OpenCV 程序库，进行路牌的扣取、分类和识别。使用 ANN::MLP（神经网络 - 多层感知器）方法。

## Binary Release

动态链接 OpenCV 和 Qt，如果没有配置安装 Qt4 和 OpenCV2，
需要的下载 dll：<http://gnat.qiniudn.com/homework/dlls-qt-opencv.7z>
如果还是不能运行，就下载：<http://gnat.qiniudn.com/homework/dlls-full.7z>，
把下载后的 dll 放在程序一起即可运行。

## 手工编译

0. 需要 Qt4，OpenCV 库，以及 CMake。
1. cmake 产生 VS2010 工程；
2. 用 VS 编译。

## Envs

Env | Env Value
--- | ---
OpenCV | `C:\dev\OpenCV\build`
OpenCV_Dir | `%OpenCV%`
QTDIR | `C:\Qt\qt-4.8.6-x64-msvc2010\qt-4.8.6-x64-msvc2010`
PATH | `%PATH%;%QTDIR%\bin;%OpenCV%\x64\vc10\bin;`

## 成员

* 组长：ZHAO PengCheng
* 成员：（拼音排序）
    + GUO Chen
    + LIAO YanYan
    + LIU Xuan
    + **TANG ZhiXiong**
    + WANG JinYi
    + ZHANG Yuan
    + ZHAO PengCheng

## Acknowledgements

感谢 LI YinXuan 师姐的参考程序。
感谢 Zhao PengCheng 的 feature 的设计方案。
