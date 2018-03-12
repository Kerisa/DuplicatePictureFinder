# DuplicatePictureFinder
用于相似图像的查找。<br>
主要做法就是统计图像RGB分量的颜色数，得到图像的特征向量，并两两计算其巴氏距离。对于小于设定阈值的文件，将其归为同一类，即相似图像。<br>
目前可处理 bmp、png、jpg 三类图像，使用了 libpng 和 libjpeg 的库，使用 vs2015 x86 编译。<br>

用 Qt 弄了一个 Gui。<br>
添加想要查找的目录后即可进行查找。<br>
查找结果会将相似结果按组显示，界面上可对图像进行快速预览，以及批量删除。<br>

![Alt text](http://i68.tinypic.com/95sllu.jpg "1")