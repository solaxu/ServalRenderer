# Backend Optimization Of MiniJava Compiler

请配合[虎书](https://www.amazon.cn/Modern-Compiler-Implementation-in-Java-Appel-Andrew-W/dp/052182060X/ref=sr_1_1?ie=UTF8&qid=1502965613&sr=8-1&keywords=Modern+Compiler+Implementation+in+Java)食用。

## Optimization On AST(Abstract Syntax Tree)

AST上的优化属于早期优化，主要目的是裁剪 AST 从而生成更加简洁的中间代码形式，以减少控制流优化的输入。

### Dead Class/Methed Elimination

由程序入口开始（对 Java\C# 是 Main Class, 对 C 则是 main 函数），遍历代码的抽象语法树。方式如下：

Dead Method Elimination

```
func dead_method_elimination(in_method, alive_set):
    for each statement in in_method, do:
        for each astnode in statement, do:
            if astnode's type is function call, do:
                if funtion's name dont equal to method's name do: // avoid recusive function call
                    add the function's name to alive_set
                    dead_method_elimination(method)
                end
            end
        end
    end
end


func dead_method_elimination(alive_set):
    for each function'name in alive_set do:
        code generation
    end
end
```

Dead Class Elimination

```
func dead_class_elimination(in_class, alive_set):
    for each statement in method, do:
        for each astnode in statement, do:
            if astnode's type is member function call, do:
                if class's name dont equal to the in_class's name do: // avoid recusive function call
                    add the class's name to alive_set
                    dead_class_elimination(class, alive_set)
                end
            end
            if astnode's type is member, do:
                if member type's name name dont equal to the in_class's name do: // avoid recusive
                    add the member type's name to alive_set
                    dead_class_elimination(member_type, alive_set)
                end
            end
        end
    end
end


func dead_class_elimination(alive_set):
    for each function'name in alive_set do:
        code generation
    end
end
```

注意以上使用了 set，这是为了防止同名重复加入的情况。可以看到整个过程实际上是由程序入口开始的一个递归过程，遍历程序中可能用到的类与方法，并记录他们，最后根据记录来进行代码生成。

### Dead Code Elimination

Dead code elimination 主要是删除一些无用代码，例如：

```
func foo1()
{
    x = 1;
    y = 2;
    z = 3;
    m = 4;
    n = foo3(&m);   // caution: m is changed here, that means n should be keeped.
    w = foo2(m);    // caution: z is not changed here, so we can delelte w
    p = x + y;
    q = p + m;
    return q;
}
```
在上面的代码中 z = 3 与 w = foo2() 是与返回值无关的，是无意义的，因此需要从 AST 上将它们删除。需要注意的是 n 与 w，n 所在语句的函数调用可能会改变 m 的值， 而 m 参与了返回值的运算，因此 n 应当被保留下来， 而 w 则不属于这种情况，因此可以直接进行删除。

删除方式如下：

```
func dead_code_elimination(original_method, new_method)
{
    iterator last = the last statement in original_method
    iterator first = the first statement in original_method
    Set work_set
    for each parameter in original_method, do:
        if the parameter can be changed in the original_method, do:
            add parameter symbol to work_set
        end
    end
    for each interator it from last to first, do:
        boolean can_delete = true
        if it->statement's type is return, do:
            add return symbal to work_set
            can_delete = false
        end
        else if it->statement's type is assignment, do:
            if assign target is in work_set, do:
                add symbols in the right of equation to work_set, 
                can_delete = false
            end
            else if it->statement's type is controller, do:
                add symbols in the right of equation to work_set, 
                can_delete = false
            end
            else
                boolean need_break = false
                for each function call in the statement, do:
                    if need_break, do:
                        break
                    end
                    for each parameter in function call, do:
                        if the parameter can be changed in the mathod, do:
                            add parameter symbol to work_set
                            need_break = true
                            break;
                        end
                    end
                end
                can_delete = false
            end
        end
        if not can_delete, do:
            head insert it->statement into new_method's statement list
        end
    end
}
```

整个过程首先检查从 Method 的参数开始，将可能会在函数中被改变的参数加入到 work\_set 中，然后返回语句开始向前推进，先将返回参数加入到 work\_set 中； 然后，对于每一个赋值语句，检查它的赋值目标，如果存在于 work\_set 中，那么就将该语句中与运算相关的所有参数都加入到 work_\set；对于跳转控制语句，将其使用到的相关参数无条件加入 work\_set；对于其他语句，检测其中包含的函数调用，如果函数调用的参数有可能在函数内会被更改，那么就将该参数也加入到 work\_set 中。当一个语句中由参数被加入到 work\_set 中的时候，就意味着该语句是“活”的，可以添加到优化后的 method 中。

### Algebraic Simplification

代数式化简，一般是用来预先计算代数运算，例如 x = 9 * 8 + y => x = 72 + y 以及 x = 0 * y => x = 0。

基本就是树上合并叶子结点的过程，不多赘述。

### Constant Folding

常量折叠，例如：
```
y = 2 + 3；
```
实际上可以直接简化为：
```
y = 5;
```

实际上是一个替换的过程。

## Control-flow Graph (CFG)

控制流图，其基本单位是 Basic Balocks(基本块)。

### Basic Blocks

在 MiniJava Compiler 中，代码首先会被编译为 C style 的三地址码，然后在此基础上做基本块的划分。

```
通常情况下的C表示：
void head_insert(struct list_node* list, list_node* n)
{
    if (list == NULL)
    {
        list = n;
    }
    else
    {
        n->next = list;
        list = n;
    }
}

MiniJava的 C 表示法：

void head_insert(struct list_node* list, list_node* n)
{
label1: if (list == NULL)
    {
        goto label2;
    }
    else
    {
        goto label3;
    }
label2: list = n;
        goto label4;
label3: n->next = list;
        list = n;
        goto label4;
label4: return;
}
```
基本块的性质：

1. 第一个语句是 Label

2. 最后的语句是一个跳转语句

3. 没有其他的 Label以及跳转语句

首先要确定基本块的入口：

1. 第一个语句是入口

2. 任何可能由条件转移语句或无条件转移语句转移到语句都是入口语句

3. 紧跟在转移语句或者条件转移语句后面的都是入口语句

从一个基本块的入口开始，直到另一个基本块的入口语句之间的语句都属于一个基本块。

对于上面的代码，基本块为：

block 1
```
label1: if (list == NULL)
    {
        goto label2;
    }
    else
    {
        goto label3;
    }
```

block 2

```
label2: list = n;
        goto label4;
```

block 3

```
label3: n->next = list;
        list = n;
        goto label4;
```

block4

```
label4: return;
````

之所以如此，是因为 if-if else- else是作为一个完整的语句出现的，并非每一个单独的分支都作为独立的语句。

```
通常情况下的C表示：
void traversal(struct list_node* list, void(*f)(void*))
{
    list_node* temp = list;
    while(temp != NULL)
    {
        if (f != NULL)
        {
            f((void*)temp);
        }
        temp = temp->next;
    }
    return;
}

MiniJava的 C 表示法：

void traversal(struct list_node* list, void(*f)(void*))
{
traversal1:     list_node* temp = list;
                goto while1;        // 由于基本块中要求除入口外不包含额外的 label，因此需要额外添加一个跳转语句
// while loop
while1:         if (temp != NULL)
                {
                    goto while12;   // 循环条件判断，通过跳转到 while 内的第一个 label
                }
                else
                {
                    goto traversalRet;  // 未通过判断跳转至 while 语句后面的第一个 label
                }
while12:        if (f != NULL)
                {
                    goto while121;   // 判断通过，跳转到紧跟的 label
                }
                else
                {
                    goto while11;  // else 悬空，跳到紧跟的 label
                }
while121:       f((void*)temp);
                goto while11;
while11:        temp = temp->next;
                goto while1;        // 添加跳转语句
// while loop end
traversalRet:   return;
}
```

对应基本块：

block1

```
traversal1:     list_node* temp = list;
                goto while1; 
```

block2

```
while1:         if (temp != NULL)
                {
                    goto while12;   // 循环条件判断，通过跳转到 while 内的第一个 label
                }
                else
                {
                    goto traversalRet;  // 未通过判断跳转至 while 语句后面的第一个 label
                }
```

block3

```
while12:        if (f != NULL)
                {
                    goto while121;   // 判断通过，跳转到紧跟的 label
                }
                else
                {
                    goto while11;  // else 悬空，跳到紧跟的 label
                }
```

block4

```
while121:       f((void*)temp);
                goto while11;
```

block5

```
while11:        temp = temp->next;
                goto while1;
```

block6

```
traversalRet:   return;
```

通过上面第二个例子可以发现，转化为基本块的主要依赖于如何使用正确的方式去分配跳转的 label，这里采用的生成方式是：对于每一个函数中的语句，由return语句开始从后至前对每一个语句进行 label 的分配，对于嵌套语句形如：

```
if (...)
{
    if (...)
    {

    }
}

或者

while (...)
{
    while (...)
    {

    }
}
```

等类型，额外加一位数字来表示嵌套深度。

同样，也可以看出基本块的划分方法还是很简单的：依次遍历语句，只要发现 label 就开始一个新的基本块并结束前一个基本块，若此时未遇到跳转语句就在该语句后面加一条跳转到下一个 label 的语句；只要发现跳转语句(这里是goto)就结束基本块。

需要注意的是上面的例子中没有对 函数调用 进行处理，事实上，函数调用也可以视为是另外一种特殊形式的跳转，不过指令复杂了一点而已。

当划分了基本块之后，它们之间的顺序就不会影响到程序的最终执行结果。因此可以依次对基本块进行进一步的处理。

### Live Analysis

活性分析。

程序中的每一条语句都是控制流中的一个结点，如果一条语句后面紧跟另外一条语句，那么就称其为一条边。

这里首先定义每个结点的 use 与 def。

对变量或者临时变量的赋值行为称为变量定义：

* use: 在赋值号右边(或者其他表达式中)的变量的出现称为使用了该变量。
* def: 赋值号左边的变量称为定义了该变量。

每一个结点均使用两个集合来表示其 use-def。

定义活性：如果有一个变量从它所在的边到它的使用不经过任何定义，则称之为在该边上是活跃的。若一个变量在某一个结点的所有入边上均活跃，则称之为live-in；在出边上则称之为live-out。

```
void foo()
{
    i = 1;
    j = 1;
    k = 0;
    while (k < 100)
    {
        if (j < 20)
        {
            j = i;
            k = k + 1;
        }
        else
        {
            j = k;
            k = k + 1;
        }
    }
    return;
}

基本块：

void foo()
{
foo1:           i = 1;              // def={i} use={}
                j = 1;              // def={j} use={}
                k = 0;              // def={k} use={}
                goto while1;        // 出边 foo1->while1

while1:         if (k < 100)        // def={} use={k}, 入边 foo1->while1, while111->while1, while112->while1
                {
                    goto while11;   // 出边 while1->while11
                }
                else
                {
                    goto fooRet;    // 出边 while1->fooRet
                }

while11:        if (j < 20)         // def={} use={j}, 入边 while1->while11
                {
                    goto while111;  // 出边 while11->while111
                }
                else
                {
                    goto while112;  // 出边 while11->while112
                }

while111:       j = i;              // def={j} use={i}, 入边 while11->while111
                k = k + 1;          // def={k} use={k}
                goto while1;        // 出边 while111->while1

while112:       j = k;              // def={j} use={k}, 入边 while11->while112
                k = k + 1;          // def={k} use={k}
                goto while112;      // 出边 while112->while1

fooRet: return;
}
```

计算活性信息的方法：

* 如果一条语句使用了变量 x，则说明 x 在进入该语句时是 live-in 的。
* 如果一个变量 x 在进入一条语句时是 live-in 的，那么它在该语句的所有前驱语句中都是 live-out 的。
* 如果一个变量 x 在一条语句中是 live-out 的，且它不在该语句的 def 集合内，那么 x 在该语句中是 live-in 的。

需要做的就是对基本块重复上面的过程，直到集合不再发生变化为止。因为在得到完整的 live-in-out 信息之前，每一次迭代都会导致每条语句的 live-in, live-out 发生变化，考虑：

```
a = 1;          // def={a} use={}
b = 2;          // def={b} use={}
c = 3;          // def={c} use={}
d = b + c;      // def={d} use={b,c}
e = a + b;      // def={e} use={a,b}
c = d + e;      // def={c} use={d,e}
```
第一次迭代：
```
a = 1;          // def={a} use={}       in={} out={}
b = 2;          // def={b} use={}       in={} out={}
c = 3;          // def={c} use={}       in={} out={b,c}
d = b + c;      // def={d} use={b,c}    in={b,c} out={a,b}
e = a + b;      // def={e} use={a,b}    in={a,b} out={d,e}
c = d + e;      // def={c} use={d,e}    in={d,e} out={}
```
第二次迭代：
```
a = 1;          // def={a} use={}       in={} out={}
b = 2;          // def={b} use={}       in={} out={b}
c = 3;          // def={c} use={}       in={b} out={b,c,a}
d = b + c;      // def={d} use={b,c}    in={b,c,a} out={a,b,d}
e = a + b;      // def={e} use={a,b}    in={a,b,d} out={d,e}
c = d + e;      // def={c} use={d,e}    in={d,e} out={}
```
第三次迭代：
```
a = 1;          // def={a} use={}       in={} out={}
b = 2;          // def={b} use={}       in={} out={b,a}
c = 3;          // def={c} use={}       in={b,a} out={b,c,a}
d = b + c;      // def={d} use={b,c}    in={b,c,a} out={a,b,d}
e = a + b;      // def={e} use={a,b}    in={a,b,d} out={d,e}
c = d + e;      // def={c} use={d,e}    in={d,e} out={}
```
第四次迭代：
```
a = 1;          // def={a} use={}       in={} out={a}
b = 2;          // def={b} use={}       in={a} out={b,a}
c = 3;          // def={c} use={}       in={b,a} out={b,c,a}
d = b + c;      // def={d} use={b,c}    in={b,c,a} out={a,b,d}
e = a + b;      // def={e} use={a,b}    in={a,b,d} out={d,e}
c = d + e;      // def={c} use={d,e}    in={d,e} out={}
```
第五次迭代：
```
a = 1;          // def={a} use={}       in={} out={a}
b = 2;          // def={b} use={}       in={a} out={b,a}
c = 3;          // def={c} use={}       in={b,a} out={b,c,a}
d = b + c;      // def={d} use={b,c}    in={b,c,a} out={a,b,d}
e = a + b;      // def={e} use={a,b}    in={a,b,d} out={d,e}
c = d + e;      // def={c} use={d,e}    in={d,e} out={}
```
第五次迭代与第四次迭代结果相同，终止该过程.

接下来我们将逐次应用活性分析的结果来做一些具体的优化工作。

### Dead Code Elimination

假设数据流图中包含语句 x = a + b 或者是 x = Mem[n] , 其中 x 不属于 s 的有效 out 集合，那么该语句可以被删除。

考虑：
```
x = a + b;  // def={x} use={a,b}    in={a,b} out={b,d,x}
y = b + d;  // def={y} use={b,d}    in={b,d,x} out={x,d}
z = x + d;  // def={z} use={x,d}    in={x,d} out={}
```

可以看到 y 不属于它所在语句 y = b + d 的有效 out 集合，因此可以删除。

### Reaching Definitions

到达定义主要考虑对于一个变量 x 的特殊复制是否会影响到程序中另外一个位置的 x 的值。对于为变量 x 赋值/修改（传递引用/指针的函数调用）的语句例如：x = a + b 或者 x = Mem[n] 称之为一个无歧义定义。假设有两个定义 j 与 语句 s，如果从 j 到 s 的控制流不包含 j 的无任何无歧义定义，则说明 j 到达语句 s。

为了完成到达定义的分析，需要为每一条语句定义一个标识符 id 和两个集合 gen={} 和 kill={}，其中 kill 则代表了该语句消除了哪些变量的其它定义（即这些定义将不能够继续影响后面语句的执行），gen 则代表生成了一个定义语句 id。此外还需要定义一个集合 defs(var) 代表了定义变量 var 的所有语句的集合。

迭代计算，每条语句的 in 与 out 可以通过下面的方式进行计算：

* 当前语句的 in 集合是该语句的所有前驱的 out  集合的并集。
* 当前语句的 out 集合等于该语句的 gen 集合并上 (该与语句的 in 集合与 kill 集合的差)

重复以上过程直到集合不再发生变化为止。

考虑：（以下过程中 in' 与 out' 集合是到达定义的计算结果， in 与 out 是只采用活性分析计算的结果）

第一次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}            gen={s1} kill={s3}=defs(a)-{s1}     in'={} out'={s1}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}        gen={s2} kill={}=defs(c)-{s2}       in'={s1} out'={s1,s2}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}        gen={s3} kill={s1}=defs(a)-{s3}     in'={s1,s2} out'={s2,s3}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}          gen={s4} kill={}=defs(b)-{s4}       in'={s2,s3} out'={s2,s3,s4}
s5: d = c + a;  //  def={d} use={c,a}   in={c,a} out={}             gen={s5} kill={}=defs(d)-{s5}       in'={s2,s3,s4} out'={s2,s3,s4,s5}

defs(a) = {s1, s3}
defs(b) = {s4}
defs(c) = {s2}
defs(d) = {s5}
```

第二次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}        gen={s1} kill={s3}=defs(a)-{s1}     in'={} out'={s1}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}    gen={s2} kill={}=defs(c)-{s2}       in'={s1} out'={s1,s2}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}    gen={s3} kill={s1}=defs(a)-{s3}     in'={s1,s2} out'={s2,s3}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}      gen={s4} kill={}=defs(b)-{s4}       in'={s2,s3} out'={s2,s3,s4}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}         gen={s5} kill={}=defs(d)-{s5}       in'={s2,s3,s4} out'={s2,s3,s4,s5}
```

第一次迭代与第二次迭代结果相同，因此到达定义过程分析结束。

s1 中的 a 的定义只在 s3 s2 s1中有效， 而在 s4 s5中无效，而在单纯的活性分析中，变量 a 则从 s1~s5 都活跃。

可以看到，到达定义得到的 in-out 集合主要是表明语句之间的关系而并非变量之间的关系，两者结合可以有这样的一个发现：

```
gen 与 kill 可以用来表示活性：变量的任何一次使用将产生一个活性，而一次定义则将消除一个活性。
```

利用达定义可以进行活性分析的计算。

考虑上面的例子：使用产生一个 gen，定义产生一个 kill。

计算 in-out 的方法就变为：

* 语句 s 的 in 集合等于 gen 集合并上 (out 集合与 kill 集合的差)。
* 语句 s 的 out 集合是其前所有驱语句 in 集合的并集。

该过程的迭代顺序与前面的顺序相反，由最后一个语句开始，前面的前驱语句变为后继语句，后继语句变为前驱语句。

第一次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}          gen={} kill={a}         in'={} out'={}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}      gen={b} kill={c}        in'={b} out'={}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}      gen={a,b} kill={a}      in'={a,b} out'={}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}        gen={} kill={b}         in'={} out'={}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}           gen={c,a} kill={d}      in'={c,a} out'={}
```

第二次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}          gen={} kill={a}         in'={} out'={b}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}      gen={b} kill={c}        in'={b} out'={a,b}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}      gen={a,b} kill={a}      in'={a,b} out'={}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}        gen={} kill={b}         in'={} out'={c,a}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}           gen={c,a} kill={d}      in'={c,a} out'={}
```

第三次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}          gen={} kill={a}         in'={b} out'={b}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}      gen={b} kill={c}        in'={b,a} out'={a,b}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}      gen={a,b} kill={a}      in'={a,b} out'={c,a}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}        gen={} kill={b}         in'={c,a} out'={c,a}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}           gen={c,a} kill={d}      in'={c,a} out'={}
```

第四次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}          gen={} kill={a}         in'={b} out'={b,a}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}      gen={b} kill={c}        in'={b,a} out'={a,b}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}      gen={a,b} kill={a}      in'={a,b,c} out'={c,a}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}        gen={} kill={b}         in'={c,a} out'={c,a}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}           gen={c,a} kill={d}      in'={c,a} out'={}
```

第五次迭代：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}          gen={} kill={a}         in'={b} out'={b,a}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}      gen={b} kill={c}        in'={b,a} out'={a,b}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}      gen={a,b} kill={a}      in'={a,b,c} out'={c,a}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}        gen={} kill={b}         in'={c,a} out'={c,a}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}           gen={c,a} kill={d}      in'={c,a} out'={}
```

第四次迭代第五次迭代结果相同，过程终止。

### Constant Propagation

考虑：

```
x = 2;
m = a + b;
n = x + m;
y = x + 3;

实际上可以简化为：

m = a + b;
n = 2 + m;
y = 2 + 3;
```

常量传播：如果一条常量赋值语句 s 的定义能够到达另外一条语句 n，且一路上没有其他定义，那么 s 定义的变量 d 就是一个常量。此时，可以将路径上的变量 d 的使用替换为常量。

在前面的例子中：

```
s1: a = 2;      //  def={a} use={}      in={b} out={b,a}        gen={s1} kill={s3}=defs(a)-{s1}     in'={} out'={s1}
s2: c = b + 1;  //  def={c} use={b}     in={b,a} out={a,b,c}    gen={s2} kill={}=defs(c)-{s2}       in'={s1} out'={s1,s2}
s3: a = a + b;  //  def={a} use={a,b}   in={a,b,c} out={c,a}    gen={s3} kill={s1}=defs(a)-{s3}     in'={s1,s2} out'={s2,s3}
s4: b = 2;      //  def={b} use={}      in={c,a} out={c,a}      gen={s4} kill={}=defs(b)-{s4}       in'={s2,s3} out'={s2,s3,s4}
s5: d = c + a;  //  def={d} use={c,b}   in={c,a} out={}         gen={s5} kill={}=defs(d)-{s5}       in'={s2,s3,s4} out'={s2,s3,s4,s5}
```

常量赋值语句 s1 的定义 a 到达 s3，那么在 s2 s3 中使用的 a 都可以用常数 2 进行替换。而 s3 定义的 a 则并非常量赋值语句，因此不能使用该优化方式。

```
s1: a = 2;
s2: c = b + 1;
s3: a = 2 + b;
s4: b = 2;
s5: d = c + a;
```

### Copy Propagation

复制传播：如果一条变量 s: d = t 赋值语句 s 的定义 d 能够到达另外一条语句 n，且一路上没有其他定义，那么可以将路径上的变量 d 的使用替换为变量 t。

总体类似于常量传播，不赘述。

### Available Expressions

可用表达式。该优化方式针对以下情况，主要用来消除重复计算。

有效表达式定义：数据流图中结点 n 处的表达式 x op y 是有效的，则说明从入口节点到结点 n 的所有路径中， x 与 y 都至少被计算一次，并且该在该路径上 x op y 出现之前，没有任何 x 或 y 的定义。

```
s1: x = a + b;      // gen={[a,b]} kill={x}        in={} out={[a,b]}
s2: y = a + c;      // gen={[a,c]} kill={y}        in={[a,b]} out={[a,c],[a,b]}
s3: z = d + b;      // gen={[d,b]} kill={z}        in={[a,c],[a,b]} out={[a,c],[a,b],[d,b]}
s4: w = a + b;      // gen={[a,b]} kill={w}        in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b]}  注意这里的 gen 与 in 之间的交集 [a,b]
s5: z = z + b;      // gen={} kill={z}             in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b]}  注意这里的 gen 与 kill，[z + b] - z 使得 gen 为空
s6: m = w + z;      // gen={[w,z]} kill={m}        in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b],[w,z]}
s7: b = a + c;      // gen={[a,c]} kill={b}        in={[a,c],[a,b],[d,b],[w,z]} out={[a,c],[w,z]}  
                    // 注意这里的 gen 与 in 的交集 [a,c]，同时由于 kill 了 b，因此原有的可达表达式 [a + b] 与 [d + b] 都在此失效。
s8: y = a + b;      // gen={[a,b]} kill={y}        in={[a,c],[w,z]} out={[a,c],[w,z],[a,b]}
```

注意 s1 与 s4，两个语句中均包含表达式 a + b，在不优化的情况下，a + b 将会被计算 2 次，而明显地，x 的定义可以由 s1 到达 s4。

此外还要注意 s5 中的 z，该语句并不能得到一个表达式 z + b，因为 z 在该语句中不仅被使用，还被重新定义了。

以及 s7 中对 b 的重新赋值，使得前面的 [a + b] [d + b] 都不再有效。

### Common Sub-Expression Elimination (CSE)

在得到上面的可用表达式的信息之后，就可以进行公用子表达式消除了。

```
s1: x = a + b;      // gen={[a,b]} kill={x}        in={} out={[a,b]}
s2: y = a + c;      // gen={[a,c]} kill={y}        in={[a,b]} out={[a,c],[a,b]}
s3: z = d + b;      // gen={[d,b]} kill={z}        in={[a,c],[a,b]} out={[a,c],[a,b],[d,b]}
s4: w = a + b;      // gen={[a,b]} kill={w}        in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b]}  注意这里的 gen 与 in 之间的交集 [a,b]
s5: z = z + b;      // gen={} kill={z}             in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b]}
s6: m = w + z;      // gen={[w,z]} kill={m}        in={[a,c],[a,b],[d,b]} out={[a,c],[a,b],[d,b],[w,z]}
s7: b = a + c;      // gen={[a,c]} kill={b}        in={[a,c],[a,b],[d,b],[w,z]} out={[a,c],[w,z]}  注意这里的 gen 与 in 的交集 [a,c]
s8: y = a + b;      // gen={[a,b]} kill={y}        in={[a,c],[w,z]} out={[a,c],[w,z],[a,b]}
```

交集出现的地方就是公用表达式出现的地方，可以对此语句进行替换，以消除公用子表达式。

### Register Allocate

该部分配合[线性扫描分配寄存器](http://web.cs.ucla.edu/~palsberg/course/cs132/linearscan.pdf)食用。

这个是原课程没有的，因为打算弄一个基于寄存器的虚拟机给自己的 Software Renderer 做可编程管线，因此额外加上这一条。由于是初次接触，故采用较为简单的线性扫描的方式进行寄存器分配。

下面例子是从虎书第十章拉出来的。

```
BLOCK1:     a = 0;                  # 1             in={c}  out={a,c}
            goto BLOCK2;

BLOCK2:     b = a + 1;              # 2             in={a,c} out={b.c}
            c = c + b;              # 3             in={b,c} out={b,c}
            a = b * 2;              # 4             in={b,c} out={a,c}
            if (a < 100)            # 5             in={a,c} out={a,c}
            {
                goto BLOCK3;
            }
            else
            {
                goto BLOCK2;
            }

BLOCK3:     return c;               # 6             in={c} out={}   
```

#### Live Intervals

定义区间 [i, j]，当对于一个变量 x 而言，不存在一条指令 j’ > j 使得 x 在 j' 上活跃，且不存在一条指令 i' < i 使得 x 在 i' 上活跃。区间 [i, j] 可能包含有一部分 x 不活跃的子区间段，但这些区间段将会被忽略（即变量从第一次定义到最后一次使用的两个指令编号的区间表示）。

对于上面的例子：

```
  s0          s1          s2          s3          s4          s5          s6
a  |           |-----------|           |           |-----------|           |
b  |           |           |-----------|-----------|           |           |
c  |-----------|-----------|-----------|-----------|-----------|-----------|
```

对于变量 a，它的活跃区间是 [1,5]
对于变量 b，它的活跃区间是 [2,4] 
对于变量 c，它的活跃区间是 [0,6]

注意，如果以上区间划分利用到达定义的方式进行划分，那么属于 lifetime hole。例如 c 虽然在 入口 s0 处直到 s6 处均活跃，但在 s3 处重新被定义了，因此它的活跃区间应当分为两份。该方式可参考[Quality and Speed in Linear-Scan Register Allocation](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.1.9128&rep=rep1&type=pdf)。

#### 算法

```
LinearScanRegisterAllocation
    active ←{}
    foreach live interval i, in order of increasing start point
        ExpireOldIntervals(i)
        if length(active) = R then
            SpillAtInterval(i)
        else
            register[i] ← a register removed from pool of free registers
            add i to active, sorted by increasing end point

ExpireOldIntervals(i)
    foreach interval j in active, in order of increasing end point
    if endpoint[j] ≥ startpoint[i] then
        return
    remove j from active
    add register[j] to pool of free registers

SpillAtInterval(i)
    spill ← last interval in active
    if endpoint[spill] > endpoint[i] then
        register[i] ← register[spill]
        location[spill] ← new stack location
        remove spill from active
        add i to active, sorted by increasing end point
    else
        location[i] ← new stack location
```

将上述活跃区间按照起点的递增顺序排列：GA = {[0,6,c] [1,5,a] [2,4,b]}

对于每一步（总步数由变量的数目决定），算法需要一个将覆盖该语句的活跃区间按照终点递增的顺序排列的一个集合 actives。

因为一共 3 个变量，因此预先定义一个虚拟寄存器表 Register[a,b,c] 与一个 Location[a,b,c]。

```
actives = {}
// for GA[i]
GA[0] = [0,6,c]
    Register[c] = r1; --> 即将 r1 绑定到 c 上
    actives = {[0,6,c]}
GA[1] = [1,5,a]
    (((a[0]=[0,6,c]).endpoint)=6) >= (GA[1].startpoint=1):
        (length(active) = 1) != (R=2):
            Register[a] = r2; --> 将 r2 绑定到 a 上
            actives = {[1,5,a],[0,6,c]} --> 集合 active 新增 [1,5,a]
GA[2] = [2,4,b]
    (((a[0]=[1,5,a]).endpoint)=2) >= (GA[2].startpoint=4):
        (length(active) = 2) == (R=2): ----> actives中所有的区间都未“过期”，执行溢出操作
            spill = [0,6,c]
            spill.endpoint > GA[2].endpoint:   
                Register[b] = Register[c] 即 r1 绑定到 b 上
                Location[c] = new stack mem 将其放到stack上
                actives = {[1,5,a]} --> remove [0,6,c]
                actives = {[2,4,b], [1,5,a]} --> add [2,4,b]
```

算法逐步将每一个已经“过期”的区间从 actives 列表中剔除，所谓“过期”就是指 actives 中的某区间的 endpoint 比当前区间的 startpoint 还要小。

actives 列表的长度最长不会超过实际的寄存器数目，在最坏的情况下，actives 的长度等于实际寄存器的数目，并且所有 actives 列表中的区间都没有过期，此时应当采取“溢出”操作，重新分配，并将溢出变量的值放在栈上。

注意，这里的分配结果与虎书上面的描述不一致，是因为没有考虑到等式赋值的时候，寄存在哪一个时间点可以被复用，例如：
```
a = 0
b = a + 1;

实际上是：

mov eax 0           //  a = 0

add eax 1           //  b = a + 1
mov ebx axe

```

同样地：
```
a = b * 2

实际上是：

mul ebx 2
mov eax ebx
```

虽然**赋值语句**使得变量被重新定义，但是其实际上在变量被赋值之前(mov语句执行之前)并没有被使用，这一点需要注意。