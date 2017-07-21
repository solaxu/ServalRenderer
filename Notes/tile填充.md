尝试了一种Tile填充方式：使用单个线程填绘制充单个Pixel，其实这并不算是Tile-Based的东西。

线程通过事件控制进行绘制与等待，每个线程在执行的时候根据其线程id来获取其计算到的当前Tile的对应位置的像素值。

填充结果如下图[1]所示。图中的Tile大小为 8x8 ，每个Tile使用 64 个线程填充，填充完一个Tile，所有的线程跳去填充下一个Tile。效率很低，图[1]中的帧缓冲大小为 640 x 480 ，填充完毕一帧就需要270ms，这效率简直没法看。

![图 1](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/tile_fill.jpg)

效率问题很大可能出在对内核对象的控制上与像素位置的定位上，图例中一共64个threads，使用了128个Thread Event行控制；又一共 80 x 60 = 4800个Tile。那么在一帧中Thread Event对象的状态切换就需要 128 x 2 x 4800 = 1228800 次。使用Hash进行像素定位 640 x 480 = 307200次，怎么看都够呛。 :\

其实换掉Event不适用内核对象，单纯使用bool值作为开关同样效率不高。类似过程代码可以[点击查看](https://github.com/solaxu/Codes/blob/master/multi_thread_to_fill_matrix.cpp)。而且从严格意义上讲，也并非是采用内存池的方式对像素进行填充，因为每次都要等待一个Tile完成才能转入下一个Tile。

对两篇文章的通读，大概明白了Tile-Based是这样工作的：

1. 将帧缓冲化分为Tile。
2. 在每一帧的开始，Tile的填充任务放到Thread Pool的任务列表中。
3. Thread Pool中的线程开始工作，并行处理这些Tile。
4. 当Thread Pool中的任务列表为空的时候，说明当前帧已经填充完毕。
5. 将当前帧显示到屏幕上。

其中几个要点：

1. 每个Tile应当包含其对应的显示列表，也即Primitive List。什么时候组织这个显示列表以及如何对图元进行排序，分为 Sort-First, Sort-Middle, Sort-Last三种方式。参见下图[2]：

![图 2](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/Sorting_Classification.jpg)

参考文献：[文献一](http://www.cs.unc.edu/techreports/94-023.pdf)， [文献二](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.38.5414&rep=rep1&type=pdf)