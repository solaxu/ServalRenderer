重新回到 Renderer 上来。

之前打算自己弄一个小型的基于寄存器的 VM 以及一个小型的Shader Language用来为 Renderer 提供可编程的管线，但在是实际编写的时候发现，如果对于 Renderer 与 Shader 之间交互的过程认识不足，则会很容易陷入不停打补丁的情况，况且在实现玩具级 VM 的时候发现自己已经快要忘了 Renderer 的相关代码了……

为了保证 Renderer 的完成度，决定暂时将可编程部分推迟，先采用抽象的VertexProc与PixelProc对其进行替换。
 
### PixelProc

先对 PixelProc 进行实现的原因在于直接，不需要 3D 变换过程就可以看到效果，从而减少错误发生的可能。

过程大致如下：
```
    对于每一个光栅化线程:
        对每一个取到的 Tile :
            根据 Tile 包含的图元索引, 获取到 PS input stream 中的图源数据
            对图元数据进行 HET 检测，在检测的过程中进行光栅化：
                HET 过程直到单独像素点的时候对图元的各项属性进行插值。
                根据当前的 PixelProc 进行像素颜色的计算。
```
对于 PixelProc 中需要像 HLSL 中获取各个 HANDLE 的接口，直接通过类成员来设置就可以了。

需要注意的是插值过程。

与边界扫描不同的之处在于：边界扫描可以直接确定扫描线两端的像素对应的各项属性，而 HET 方式则不能直接获取这些信息。因此需要采用其他方法对其进行插值计算。

```
* 对于三角形图元：重心插值。由三角形内一点向三角形的三个顶点连接线段，将三角形分为三个子三角形，三个子三角形与原三角形之间的面积比即是该点数据与对应三角形顶点的比率。像素的的判断以像素是否在三角形内为准，并进行重心插值。

* 对于线段图元：线性插值。从实现结果来看，目前 HET 下的直线绘制并不合意，其主要问题出在如何判断当前像素点在直线上并需要绘制。

* 对于点图元：不需要插值，略。
```

下图是对随机的16条带有颜色的采用HET方式进行绘制的结果：

![HET_LINES](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/HET_LINES.png)

效果不怎么样。


需要注意的是，对于插值的结果，要进行额外保存，以便于PixelProcess对插值结果进行计算，已得到最终效果。由于采用多线程，插值结果与PixelProc运算结果都应当保存在每一个线程专用的对应地址中。

另外，对线图元和三角形图元进行插值的时候，需要知道 PS input stream 的输入格式，这需要额外的一个接口来告知 Renderer。在HLSL/GLSL中这一过程是通过 shader 的 input stream declaration 来告知的（layout, in, out, etc）。