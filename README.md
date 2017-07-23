之前写过的软渲染因为老机器挂掉搞没了，只剩下若干总结留下来，放在Git Pages上面，如下。

* [图形管线中的矩阵运算](http://solaxu.github.io/2015/12/31/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E7%9F%A9%E9%98%B5%E8%BF%90%E7%AE%97/)
* [图形管线中的裁剪与剔除](http://solaxu.github.io/2016/01/03/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E8%A3%81%E5%89%AA%E4%B8%8E%E5%89%94%E9%99%A4/)
* [Software Renderer的若干要点](http://solaxu.github.io/2016/02/17/Software-Renderer/)


当然那个Renderer是写的不怎么样的，代码略丑，不过好在能跑，效率略微惨了点；现在看来上面的总结记录也有不足之处。之前一直没有把做的东西放Git上的习惯，这丢了就没了 :\

不过过了这一年眼界了开阔了不少，又想重新弄一个比以前强一点的出来，先开个坑慢慢填。

鉴于之前总是急于码代码，最后搞得一塌糊涂看不下去（其实看另外两个项目就可以看出来了，凡是从零开码的后面都是乱糟糟的），这次先不着急写代码，理顺了再下手。

主要参考文献：

1. 朱玥, Tile-Based图形处理方法及高质量图形算法设计. Phd Thesis, USTC, 2012. from cnki.
2. 沈阳, 基于SIMD指令的多核软件渲染器的设计与实现. Master Thesis, ZJU, 2015. from cnki.

文献[1]虽然是嵌入式方向的，不过同样可以借鉴。两者都采用Middle-Sort方式进行Tile-BasedTile-Based光栅化，但文献[1]的实现包含了一套完整的可兼容OpenGL ES的指令系统，同时对嵌入式移动设备做了针对性处理，文献[2]则单纯只包含了一个软件渲染器。因此这个坑主要依据文献[1]及其援引进行填补，文献[2]及其援引为辅。

向空明流转的[Salvia](https://github.com/wuye9036/SalviaRenderer)看齐，我觉得努力一下还是有希望的，趁着编译底子还没丢光。至于名字么……就叫Serval吧。

前期阶段的资料查阅、准备工作放在项目内Notes文件夹中，欢迎查阅。