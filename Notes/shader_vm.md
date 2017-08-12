2017.08.06：

吊针一周，遭一难。

现在基本上可以使用设定的指令进行编程了。实际上的指令与前面预期的不怎么一致，但总归可用。

立即数的加载被被设定为 loadf/load reg imm。为了省事，立即数也不放在数据区段，直接放在代码区段。load 指令占据 8 bytes, 第一个 4bytes 用来描述指令，第二个 4bytes 用来存放立即数。

遇到 call 指令之前，首先压栈该函数调用的返回值（如果有的话）。

call 指令执行过程中保存的 IP 应该指向 call 的下一条指令。

不打算进行支持 VM 与 host 的交互工作（主要是指不考虑支持函数接口之间的调用，只保留数据的设置与返回接口，需要绑定寄存器，被外部数据占用的寄存器将不会被释放）。

函数调用传参、返回一律使用赋值的方式。

可能将要遇到的事情：通用寄存器数目不够，需要进行再分配以及……有可能出现即便是分配了还是不够的极端情况，需要使用到控制流分析以及活性分析。

手写的SASM算是可用了。

[]()

------------------
2017.07.30：

所有的 register 中都只存放地址。
立即数都放到数据段 DS 中，这一点可以在编译的时候确定。采用单赋值形式可以将所有的值都放在内存中，寄存器只用来索引地址。

如此，在 Shader VM 中进行立即数加载的时候，应当把立即数在 DS 中的地址加载到一个寄存器中，然后将该寄存器连接到变量上，当然立即数也应当视为是一个单赋值语句。

int 数据的 算数运算与逻辑运算 和 float 数据区分开来，使用两套指令。

内存的分配与释放参考 STL 的chunk分块内存。暂时不考虑 GC，因为每次只为一个Render Batch 服务，服务完毕候直接将运行环境初始化。

call 指令采用 call offset 进行转移。 offset为 24 位，可直接寻址 16 MB，从而可以直接对整个进 VM 的内存空间行寻址。

------------------
整个 Shader VM 模仿 8086 微处理器。

8086具有:

* 四个16bits的数据寄存器，这里将数据寄存器扩展为 96 个，每个 32bits。
* 两个变址寄存器 SI、DI，这里不用（因为最多只有寄存器寻址）。
* 两个指针寄存器 BP、SP，这里照搬。
* 一个标志寄存器 FLAG， 这里简化。
* 一个指令寄存器 IP，这里照搬。
* 四个段寄存器，CS，SS，DS，ES，这里照搬。

因此 Shader VM 一共具有 104 个 寄存器，每一个大小均为 32bits。

8068具有三大类一共七种寻址方式：

* 立即数
* 寄存器寻址
* 存储器寻址：直接寻址、寄存器间接寻址、寄存器相对方式寻址、基址变址方式寻址、相对基址变址方式寻址

由于虚拟机设计的四个段大小均为2MB，一般足够 Shader 程序使用，这里只取立即数、寄存器寻址两种。

Shader VM 指令系统同样来源于8086指令系统：

### 通用数据传输指令：

> mov dest src 可以进行如下表达：
> * mov reg imm
> * mov reg reg
> * mov reg mem

### 堆栈操作指令：

> * push reg/seg 将目标压入栈顶，并将栈顶指针加4
> * pop reg/seg 将栈顶元素弹出到对应的位置，并将栈顶指针减4

### 地址传送指令 

> * lea reg, mem 将操作数的有效地址传入到寄存器。 reg = &data
> * lds reg, mem 将mem指定的内存数据传入到寄存器中。 reg = *mem

### 算术运算指令

> * add reg imm/reg 加法运算。reg = reg + imm/reg 与 reg = reg + *mem
> * inc reg 自加运算。
> * sub reg imm/reg 减法运算
> * dec reg 自减运算
> * mul reg imm/reg 乘法运算 
> * div reg imm/reg 除法运算，除法运算应当检测除数是否为0
> * mod reg imm/reg 取模运算
> * pow reg imm/reg 乘方运算

### int/float转换

> * i2f reg imm/reg
> * f2i reg imm/reg

### 逻辑运算

> * and reg imm/reg imm/reg
> * or reg imm/reg imm/reg
> * not reg imm/reg imm/reg

### 控制转移指令

> * jmp reg 段内转移 ip = reg / ip = *mem 
> * eq reg reg/imm reg/imm
> * le reg reg/imm reg/imm
> * leq reg reg/imm reg/imm

### 子程序指令

> * call
> * ret
>
> call ret其实可以认为是多个指令的合集

### 内存分配

> * alloc reg imm/reg
> * free reg

### 中断

> 暂略

一共26条指令，指令按照三地址码的形式安排，有：

>  | op code 8bits | dest 8bits | op1 8bits | op2 8bits | 

但是实际上在实施的时候，寄存器上存储的都是地址……