尝试了一种Tile填充方式：使用单个线程填绘制充单个Pixel，其实这并不算是Tile-Based的东西。

线程通过事件控制进行绘制与等待，每个线程在执行的时候根据其线程id来获取其计算到的当前Tile的对应位置的像素值。

填充结果如下图[1]所示。图中的Tile大小为 8x8 ，每个Tile使用 64 个线程填充，填充完一个Tile，所有的线程跳去填充下一个Tile。效率很低，图[1]中的帧缓冲大小为 640 x 480 ，填充完毕一帧就需要270ms，这效率简直没法看。

![图 1](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/tile_fill.jpg)

效率问题很大可能出在对内核对象的控制上与像素位置的定位上，图例中一共64个threads，使用了128个Thread Event行控制；又一共 80 x 60 = 4800个Tile。那么在一帧中Thread Event对象的状态切换就需要 128 x 2 x 4800 = 1228800 次。使用Hash进行像素定位 640 x 480 = 307200次，怎么看都够呛。 :\

其实换掉Event不使用内核对象，单纯使用bool值作为开关同样效率不高。类似过程代码可以[点击查看](https://github.com/solaxu/Codes/blob/master/multi_thread_to_fill_matrix.cpp)。而且从严格意义上讲，也并非是采用内存池的方式对像素进行填充，因为每次都要等待一个Tile完成才能转入下一个Tile。

对两篇文章的通读，大概明白了Tile-Based是这样工作的：

1. 将帧缓冲化分为Tile。
2. 在每一帧的开始，Tile的填充任务放到Thread Pool的任务列表中。
3. Thread Pool中的线程开始工作，并行处理这些Tile。
4. 当Thread Pool中的任务列表为空的时候，说明当前帧已经填充完毕。
5. 将当前帧显示到屏幕上。

其中要点：

> 每个Tile应当包含其对应的显示列表，也即Primitive List。什么时候组织这个显示列表以及如何对图元和像素进行排序，分为 Sort-First, Sort-Middle, Sort-Last三种方式。
>
> 参考文献：[文献一](http://www.cs.unc.edu/techreports/94-023.pdf)， [文献二](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.38.5414&rep=rep1&type=pdf)
>
> 参见下图[2]：
>
> ![图 2](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/Sorting_Classification.jpg)
>
> * Sort-First是在几何处理阶段就对图元进行排序，其中典型的方法包括：Ray Casting，BSP(Binary Space Partitioning)，Frustum detection等。其中Ray Casting作为Ray tracing的极简版甚至可以作为光栅化的一种方法；BSP常见于大型静态室内场景，在André LaMothe的[《Tricks of the 3D Game Programming Gurus-Advanced 3D Graphics and Rasterization》](https://www.amazon.com/Tricks-Programming-Gurus-Advanced-Graphics-Rasterization/dp/0672318350/ref=sr_1_1?ie=UTF8&qid=1500621282&sr=8-1&keywords=Andr%C3%A9+LaMothe)一书中有详细介绍；Frustum detection一般用来较为粗略地为每一个Tile挑选图元，一般情况下需要额外的方法来计算图元之间的遮挡关系。
> * Sort-Middle是在屏幕空间下对图元进行排序。此时各个图元已经便变换到屏幕坐标系下，可以直接使用Tile的位置来划分每个Tile需要显示的图元，同时对这些图元进行遮挡关系的判断，最后进行光栅化。这也是Serval将要采用的方法。
> * Sort-Last是在光栅化的时候对像素进行排序，是最为精确的方式。[《Tricks of the 3D Game Programming Gurus-Advanced 3D Graphics and Rasterization》](https://www.amazon.com/Tricks-Programming-Gurus-Advanced-Graphics-Rasterization/dp/0672318350/ref=sr_1_1?ie=UTF8&qid=1500621282&sr=8-1&keywords=Andr%C3%A9+LaMothe)中实现的渲染器就是采用Sort-Last，以画家算法作为最终杀招来处理图元排序的问题。
>
> 实际上，每种sort方式都有其可取之处，实际使用中经常将其分散在渲染引擎的各个不同阶段中：例如为了减少向显卡提交顶点数据量，可以借鉴Sort-First对场景做处理；而Sort-Middle与Sort-Last则因为包含在硬件处理过程中而较少被上层直接使用。

> Sort-Middle算法。全屏光栅化的过程一般使用Zigzag方法以及边界扫描法（在[《Tricks of the 3D Game Programming Gurus-Advanced 3D Graphics and Rasterization》](https://www.amazon.com/Tricks-Programming-Gurus-Advanced-Graphics-Rasterization/dp/0672318350/ref=sr_1_1?ie=UTF8&qid=1500621282&sr=8-1&keywords=Andr%C3%A9+LaMothe)一书中对边界扫描法进行了实现）。
> 见下图[3]：
>
> ![图 3](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/scanline.jpg)
>
> 但是Full Screen Rasterization的算法并不适用于Sort-Middle。常用Tile-Based使用的方法通常分为两类：多个像素点的并行光栅化（N-pixel parallel rasterization）以及层次化边界测试（Hierarchial  Edge function Test, HET）两种方式。
> * 主要参考文献《基于SIMD指令的多核软件渲染器的设计与实现》采用的Greene算法属于HET算法，即将Tile的范围按照层级缩小，直到最小一个像素为止，在每个层级上判断当前范围与Tile的相交情况，当范围变为 1 像素的时候完成光栅化。
> * 主要参考文献《Tile-Based图形处理方法及高质量图形算法设计》采用了 N 像素并行光栅化算法。该方法采用 N 的并行度在三角形包围盒内自底向上进行扫描，但是文献略有语焉不详之感。
> 
> 总之其要点在于应当判断当前光栅化的范围与三角形的相交情况，从而尽可能减少像素的重复计算。

基本方式就是这样，其实关键还是在于光栅化的过程。由于SIMD的存在，可以一次性对四个像素进行计算进而加速。纵观两篇论文，《基于SIMD指令的多核软件渲染器的设计与实现》对光栅化的过程描述更加翔实一些，《Tile-Based图形处理方法及高质量图形算法设计》则从系统总体出发，内容更多。另外，在资料查阅过程中发现台湾国立中山大学蔡宗桦先生[《嵌入式系统下的三维绘图之区块成像绘图引擎》](https://www.google.com.sg/url?sa=t&rct=j&q=&esrc=s&source=web&cd=1&cad=rja&uact=8&ved=0ahUKEwi4_Num-JnVAhVDfbwKHbebCssQFggkMAA&url=http%3A%2F%2Fetd.lib.nsysu.edu.tw%2FETD-db%2FETD-search%2Fgetfile%3FURN%3Detd-0903107-232744%26filename%3Detd-0903107-232744.pdf&usg=AFQjCNExoDQWnxDvk-driE_hlC7Gk83lww)一文对Tile-Based架构下的光栅化过程描述亦为详尽，有较高的参考价值。