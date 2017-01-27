#DuplicatePictureFinder
用于相似图像的查找<br>
主要做法就是统计图像RGB分量的颜色数，得到图像的特征向量，并两两计算其巴氏距离。<br>

##用法
```
DuplicatePictureFinder.exe <目录名1> [<文件名1> or <目录名2> ...]
```
目前会将查找结果输出到一个文本文件中。
