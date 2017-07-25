之前写过的软渲染因为老机器挂掉搞没了，只剩下若干总结留下来，放在Git Pages上面（今天找了一下，编译工程的项目作业也没了），如下。

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

关于编译器部分，请查阅我们的[编译工程课程项目主页](http://staff.ustc.edu.cn/~bjhua/courses/compiler/2014/)。该课程以 Tiger Book （《现代编译器的Java实现》）为主，将一个Java语言的子集翻译成 C 语言，并进行了一定程度的后端优化。当然该项目是为教学服务的，所以其实现的语言以及其测试用例必然不够严谨，但足够覆盖多数情形。项目包括了：词法分析、自顶向下的递归的语法分析、抽象语法树构建、简易的垃圾回收机制（单线程，Cheney算法）、语法树上的优化、控制流分析以及基于其上的优化、活性分析及基于其上的优化；未包含实现静态单赋值以及寄存器分配。Fork项目 tiger-comp 是qc1iu同学的优秀作品，其将目标代码置为 JVM Byte-code 并成功将其运行在自己实现的一个JVM上（[turkeyVM](https://github.com/qc1iu/turkeyVM)）。我自己的版本则因资料意外丢失而仅存几篇总结 :\

* [Lexer](https://solaxu.github.io/2016/02/19/Lexer/)
* [Parser](https://solaxu.github.io/2016/02/22/Parser/)
* [Semantic](https://solaxu.github.io/2016/02/27/Semantic/)
* [GC初步](https://solaxu.github.io/2016/03/04/GC%E5%88%9D%E6%AD%A5/)


另外，bjhua老师的课程还包括《形式化方法》（主讲类型推导以及Lambda演算）以及《高级软件技术专题》（分为两期，一期讲OS，一期讲函数式编程），均在其主页上有对应的Schedule，如有兴趣可自行查阅。

前期阶段的资料查阅、准备工作放在项目内Notes文件夹中，欢迎查阅。