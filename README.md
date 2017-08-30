# Description

Serval is a tile-based software renderer which is inspired by [Salvia](https://github.com/wuye9036/SalviaRenderer).

Serval will include a tile-based rasterizer and a programmable pipeline, that means if it is done, it shows what i have learnt in the passed three years.

I do not understand dx/opengl standard and llvm very well so i do not think Serval couold reach the level of Salvia. In my plan, the programmable pipeline of Serval will build on a self-made register-based VM, and the resterizer of Serval should be easily expand to a 2D graphic engine for games (That's one reason why my own Touhou Tactics project stops(There is an brave Doujin game named [Touhou Tactics in C87](http://frostzenmaiden.exteen.com/20141231/c87-touhou-tactics))).

There are so many differences between tiled and full screen, even through i have implementation a full screen scan-line software renderer (triangle list supported only, lost now :\ ), there are still many traps on my way.

And Now, i have built a simple fixed pipeline for Serval to make some explorations such as how to pick and triangles for tiles, how to built vertex buffers, how to rasterize with multiple threads, etc. 

* middle-sort.
* hericarchy edge function test rasterizer.

Some notes in my "childhood of programming", in chinese, with navie words and some errors.

---
* [图形管线中的矩阵运算](http://solaxu.github.io/2015/12/31/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E7%9F%A9%E9%98%B5%E8%BF%90%E7%AE%97/)
* [图形管线中的裁剪与剔除](http://solaxu.github.io/2016/01/03/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E8%A3%81%E5%89%AA%E4%B8%8E%E5%89%94%E9%99%A4/)
* [Software Renderer的若干要点](http://solaxu.github.io/2016/02/17/Software-Renderer/)
---
* [Lexer](https://solaxu.github.io/2016/02/19/Lexer/)
* [Parser](https://solaxu.github.io/2016/02/22/Parser/)
* [Semantic](https://solaxu.github.io/2016/02/27/Semantic/)
* [GC初步](https://solaxu.github.io/2016/03/04/GC%E5%88%9D%E6%AD%A5/)
---

and a pretty better note on Backedn Optimizationof a MiniJava Compiler (According to the Tiger book)

* [后端优化部分的总结](https://github.com/solaxu/ServalRenderer/blob/master/Notes/BackendOptimizationOfMiniJavaCompiler.md)

# Main References

1. 朱玥, Tile-Based图形处理方法及高质量图形算法设计. Phd Thesis, USTC, 2012. from cnki.
2. 沈阳, 基于SIMD指令的多核软件渲染器的设计与实现. Master Thesis, ZJU, 2015. from cnki.

# Notes

Find notes in 'Notes' directory, in chinese.

---
---
---

# 描述 

想要写 Serval 是因为看到了 [Salvia](https://github.com/wuye9036/SalviaRenderer) 的缘故，觉得弄一个小打小闹的版本正好能够把最近两三年自己学到的东西揉到一起用一下。正好移动设备的渲染架构也大多是基于 tile-base 架构的， 恰好可以借此机会学习一下。

计划中的 Serval 除了一个 tile-based 的光栅器之外还应支持可编程管线，由于我对 dx/opengl 标准的理解并不足够，对 llvm 也不太了解，因此不指望 Serval 能够达到 Salvia 的水准。可编程管线将会建立在一个自制的简单的基于寄存器的虚拟机上，同时希望能 Serval 的光栅化部分能够比较容易的改写为一个 2D graphic engine ( 这也是为何我自己的游戏项目 Touhou Tactics 停止的原因之一。另外，[C87上也有一个名为 Touhou Tactics 的同人游戏](http://frostzenmaiden.exteen.com/20141231/c87-touhou-tactics))。

目前已经为 Serval 写了一个简单的固定管线来做一些探索性的工作顺带踩坑，包括：多线程下的光栅器，middle-sort，尽可能灵活的顶点格式，等。由于 tile-based 与 full-screen 的光栅化方式不同，因此即便是在写过 full screen scan-line 方式的 software renderer （因故已丢失）的情况下依然有不少坑要踩。目前采用的主要方式大致是：

* middle-sort.
* hericarchy edge function test rasterizer.

以下是一些在当时水平还处于初学者的时候的笔记，遣词用句略显幼稚、并且有一些错误：

---
* [图形管线中的矩阵运算](http://solaxu.github.io/2015/12/31/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E7%9F%A9%E9%98%B5%E8%BF%90%E7%AE%97/)
* [图形管线中的裁剪与剔除](http://solaxu.github.io/2016/01/03/%E5%9B%BE%E5%BD%A2%E7%AE%A1%E7%BA%BF%E4%B8%AD%E7%9A%84%E8%A3%81%E5%89%AA%E4%B8%8E%E5%89%94%E9%99%A4/)
* [Software Renderer的若干要点](http://solaxu.github.io/2016/02/17/Software-Renderer/)
---
* [Lexer](https://solaxu.github.io/2016/02/19/Lexer/)
* [Parser](https://solaxu.github.io/2016/02/22/Parser/)
* [Semantic](https://solaxu.github.io/2016/02/27/Semantic/)
* [GC初步](https://solaxu.github.io/2016/03/04/GC%E5%88%9D%E6%AD%A5/)
---
以及最近返回去重新看 Mini-Java 编译器后端优化部分的总结（根据虎书，暂不包含循环优化部分）
* [后端优化部分的总结](https://github.com/solaxu/ServalRenderer/blob/master/Notes/BackendOptimizationOfMiniJavaCompiler.md)

# 主要参考文献

1. 朱玥, Tile-Based图形处理方法及高质量图形算法设计. Phd Thesis, USTC, 2012. from cnki.
2. 沈阳, 基于SIMD指令的多核软件渲染器的设计与实现. Master Thesis, ZJU, 2015. from cnki.

# 笔记

放在 'Notes' 文件夹中，欢迎查阅。