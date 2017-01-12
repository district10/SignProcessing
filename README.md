# SignProcessing2

看我的博客：[用 CvANN_MLP 进行路牌判别](http://tangzx.qiniudn.com/post-0072-cvann-mlp.html)。

![](http://whudoc.qiniudn.com/2016/SignProcessorDemo_2016-07-31_16-21-58.png)

刚才 fix 了几个错误，现在在 Ubuntu 下也跑起来了：

![](http://whudoc.qiniudn.com/2016/2016-08-01-204355_1366x768_scrot.png)

## 下载已经编译好的程序

应该能够在 Windows 上直接运行，下载链接见 [【下载二进制】](https://github.com/district10/SignProcessing/issues/1)。

## 如何使用

见 [【如何使用】](https://github.com/district10/SignProcessing/issues/2)。

---

## 下载源码并编译

-   [【qt4cv3vs2015 分支源码压缩包】](https://github.com/district10/SignProcessing/archive/qt4cv3vs2015.zip)
-   配置 VS2015，Qt4，OpenCV3，见：[代码编译 · Issue #3 · district10/SignProcessing](https://github.com/district10/SignProcessing/issues/3) :smile:

## 下载测试数据

如果你要自己训练模型，可以下载如下测试数据：（如果不用，就跳过）

-   [索引文件](http://whudoc.qiniudn.com/2016/index-csv-1..8.7z) 341 KB
-   [1.7z](http://gnat.qiniudn.com/sczip/1.7z) 8.9M
-   [2.7z](http://gnat.qiniudn.com/sczip/2.7z)  10M
-   [3.7z](http://gnat.qiniudn.com/sczip/3.7z)  13M
-   [4.7z](http://gnat.qiniudn.com/sczip/4.7z)  24M
-   [5.7z](http://gnat.qiniudn.com/sczip/5.7z)  13M
-   [6.7z](http://gnat.qiniudn.com/sczip/6.7z)  10M
-   [7.7z](http://gnat.qiniudn.com/sczip/7.7z) 8.5M
-   [8.7z](http://gnat.qiniudn.com/sczip/8.7z) 3.1M

放到 `data/input` 文件夹，然后把它们【解压到当前文件夹】。

---

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
