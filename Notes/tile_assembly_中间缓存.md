
2017.07.28:

目前打算是允许用户一次性设置 8 个 Vertex Buffer 进来，根据 Vertex Declaration 来进行 Vertex Transform, 然后根据 Vertex Transform 的结果（由 Vertex Shader决定）得到 Pixel Shader的输入流声明，再经过 Pixel Shader 得到 Pixel Fragment，最后根据当前渲染状态得到最终的 Pixel 输出。

由此而带来了中间缓存的一些问题：

1. Vertex Shader 的输入 Stream 与 Pixel Sahder 的输入 Stream 极有可能不一样，因此应该分开存放。
2. 渲染批次问题，这是与所谓的sub-frame相关的。目前是这样的：若向 renderer 提交一个批次，则该批次应当被缓存到 Vertex Shader 的 Stream Pool 中，批次提交的命令由 DrawPrimitive 来完成。目前是这样规定的：用户在每一次 SetVertexBuffer 与SetIndexBuffer 之后应当调用 DrawPrimitive 来提交本批次，Renderer 将该批次渲染到 Render Target 中。如果该批次提交的量过大，则按照实际情况应当分为若干个子批次进行提交（也就是所谓的 sub-frame ），但是这里由于是使用 Software 进行模拟，因此可以将 Stream Pool 的缓存设置得足够大来规避这个问题。

------------
影响绘制速率的一个关键因素就是提交批次，因此需要在进行绘制之前对数据及逆行中间缓存处理，以提高程序的数据局部性，从而合理利用硬件缓存，提升Cache命中率，提高程序效率。

# 概览

Tile-Based结构的相关显示算法大致分为三类：直接法、两步法、分拣法。三者所需的空间复杂度以此上升，而事件复杂度则依次下降。在主要参考文献一《Tile-Based图形处理方法及高质量图形算法设计》一文中对这三种方法做了简述，实现了分拣法。以下为其伪代码：

```cpp
1  While(!end_of_frame) {
2      if (!sub_frame_truncation){
3          if (API)
4          {
5              merged into state block and linked with every tile list
6          }
7          else if (vertices)
8          {
9              translate the point or line to triangle strips
10             calculate the area to do the culling
11             find the bounding box ( obb )
12             out of screen test
13             small triangle test
14             if (triangle passes tests all above)
15             {
16                 put the triangle into the primitive block
17                 find the maybe overlapped tiles
18                 for every maybe overlapped tile
19                 {
20                     add the pointer into the related tile-list
21                 }
22             }
23         }
24     }
25  }
26 
27  for every tile
28  {
29      to execute the tile-list belong to it
30  }
```

其中：

1. 第13行的 “small triangle test”是指没有占用像素采样点的三角形，应当被剔除。


![图1](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/small_triangle_test.png)

2. 第5行的 “state block” 指第3行的 “API” ，即当使用接口来改变渲染状态的时候，状态应当被及时更新到每个Tile的状态块中。
3. 第16行的 “primitive block” 是基本图形块，存放顶点数据
4. 第2行的 “sub_frame_truncation" 是来处理场景过于复杂，无法一次性渲染完毕的情况。

## 兼容性设计

### sub_frame, 分割子帧

当系统分配的内存无法继续开辟新的 primitive block 或者 state block 的时候，则认为场景过于复杂。由此可见系统采用了内存池策略来减少频繁的调用 malloc / free ( new / delete )带来的开销。

当场景过于复杂的时候，会将场景划分为子帧进行渲染，即首先加载一部分场景进行渲染，待完成之后及进行剩下部分的渲染直至整个场景被处理完毕

# 中间缓存数据结构设计

## steam pool

主要是将各种图元转换为三角形，然后再将三角形列表转化为三角形条带来紧凑数据。主要数据结构比较简单：

![图2](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/stream_pool.png)

另外，考虑到需要向系统提交stream说明，需要对应的API接口。

## 相交、覆盖测试

这是为了拣选与Tile相关的三角形。

使用分离轴测试法（Separating Axes Test, SAT）来测试对三角形和Tile进行相交判断。由于是在2D平面上使用SAT方法，因此判断三角形与正方形的相交只需要判断 5 次即可：判断所有的边在三角形三条边方向以及 X / Y 轴方向上的投影是否重叠即可。 该结论可推广至多边形与AABB的判定。对于三角形，该方式可以进一步简化为LET（边界测试方程）。

该方法取Tile中心点 ( ctx, cty ) 到三角形三条边的最小距离 E (将中心点位置代入三角形两点式方程即可点到直线的距离)， 若三角形与Tile相交，则 E - w / (2 * 最小边端点的曼哈顿距离) 必然小于 0。

![图3](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/tile_triangle_intersect.png)


使用叉积测试（Cross Product Test, CPT)来对三角形对Tile是否进行了完全覆盖。其实很好理解，主要是计算从原点出发到 Tile 右下角与左上角的向量与 X 轴之间的夹角与三角形三边的向量与 X 轴之间的夹角的大小关系即可。需要注意的是参考系是整个屏幕坐标系。

![图4](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/cross_product_test.png)

# 渲染部分

在 “Tile 填充” 部分已经进行了一部分描述，这里给出总体步骤：

```cpp
1   store the original states into memeory
2   for every Tile in screen
3   {
4       set the rendering nodule with the original states stored in memory
5       hidden surface remove
6       texture with VF（visible fragment）
7       per-fragment
8       put image into framebuffer
9   }
```

其中：

1. 第6行的 “per-fragment” 意指对每一个 pixel 只对应一个 fragment，此举是为了支持延迟渲染。
2. 第7行的 “VF” 保存了该TIle中所有的fragment对应的三角形内存索引。
3. fragment 是指每个像素在完成最后的光栅化之后保存的数据对象，包括指向被显示的 primitive block 以及该像素的深度值。

# pick-sort

2017.08.14：

* 像素点是否在三角形内部：记 f(x,y) = Ax + By + C，对于某点 (X,Y)，f(X, Y) 的符号表明了点在该直线的某一侧（与向量方向相关），f(X, Y) = 0 则说明点在直线上。对三角形的三条边进行该测试，三条边的测试结果均一致，则说明点在三角形内/外（与采用左/右手定则有关）。

* 左上填充规则：用来处理三角形边界上的点。该方式的详细过程如下图所示。

> ![bound_test](https://github.com/solaxu/ServalRenderer/blob/master/Notes/Pics/Bound_test.png)
>
> 从图中可以看到，在进行 Tile-Rect 判定的时候，应当采用 [left, right, top, bottom] 进行，而对像素点进行判定时，应当使用  [left + 0.5f, right + 0.5f] 进行判定。这是因为使用 HET 进行填充固然可以填充出三角形，但是却无法对其边界进行精确的控制。对边界进行绘制需要一个连续的过程，而 HET 填充则是离散的。
>
> 三角形与其AABB组成了四个区域，当三条边界方程判定中有负->正或者正->负的改变出现，则说明像素位于三角形边界上。

下面伪码中的 left, right, 均与采用的坐标系有关。

```
    if pixel in left then:
        if pixel_state == inside then:
            while current pixel_state == inside do:
                move left for next pixel
        else then:
            while current pixel_state == outside do:
                move right for next pixel
    else then:
        if pixel_state == inside then:
            while current pixel_state == inside do:
                move right for next pixel
        else then:
            while current pixel_state == outside do:
                move left for nect pixel
```

# 喵~

基本上渲染部分到这里就差不多完结了，接下来是纹理采样部分。主要分为空间同性滤波与空间异性滤波。空间同性滤波常见的就是 MipMapping，这在[《Tricks of the 3D Game Programming Gurus-Advanced 3D Graphics and Rasterization》](https://www.amazon.com/Tricks-Programming-Gurus-Advanced-Graphics-Rasterization/dp/0672318350/ref=sr_1_1?ie=UTF8&qid=1500621282&sr=8-1&keywords=Andr%C3%A9+LaMothe)一书中进行过介绍。空间异性滤波则是主要参考文献一《Tile-Based图形处理方法及高质量图形算法设计》中使用的采样方式。