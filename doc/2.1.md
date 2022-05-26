### exceptions
在ARM.v8体系结构中, 中断只是异常的一种类型，异常有4种类型.

- **同步异常**: 这种类型的异常总是由当前执行的指令引起. 例如, 可以使用`str` 指令将一些数据存储在不存在的内存位置. 在这种情况下, 将生成同步异常. 同步异常也可以用于生成 `软件中断`. 软件中断是由`svc`指令有意产生的同步异常. 
- **IRQ(中断请求)**: 这些是正常的中断. 它们始终是异步的, 这意味着它们与当前执行的指令无关. 与同步异常相反, 它们始终不是由处理器本身生成的, 而是由外部硬件生成的.
- **FIQ(快速中断请求)**: 这种类型的异常称为`快速中断`, 仅出于优先处理异常的目的而存在. 可以将某些中断配置为“正常”, 将其他中断配置为“快速”. 快速中断将首先发出信号, 并将由单独的异常处理程序处理. `Linux`不使用快速中断.
- **SError(系统错误)**: 像`IRQ`和`FIQ`一样, `SError`异常是异步的, 由外部硬件生成. 与 `IRQ` 和 `FIQ` 不同, `SError` 始终表示某种错误情况.

#### 异常向量

每种异常类型都需要有自己的处理程序. 另外, 同一种异常类型下不同的状态也需要定义单独的处理程序. 典型的有4种状态. 以EL1 为例, 这些状态可以定义如下：
1. **EL1t** 与EL0共享堆栈指针时, `EL1`发生异常. 当 `SPSel` 寄存器的值为 `0` 时, 就会发生这种情况. 
1. **EL1h** 为EL1分配了专用堆栈指针时, EL1发生了异常. 这意味着 `SPSel` 拥有值 `1`, 这是我们当前正在使用的模式. 
1. **EL0_64** 以64位模式执行的EL0产生异常. 
1. **EL0_32** 以32位模式执行的EL0产生异常. 

总共, 我们需要定义16个异常处理程序(4个异常级别乘以4个执行状态). 一个保存所有异常处理程序地址的特殊结构称为 *exception vector table* 或 *vector table*. 向量表的结构在[AArch64-Reference-Manual](https://developer.arm.com/docs/ddi0487/ca/arm-architecture-reference-manual-armv8-for-armv8-a-architecture-profile) 第1876页的 `表D1-7向量与向量表基址的向量偏移量` 中定义. 我们可以把它当做是有16个条目的数组，每个条目占 128字节，且128字节对齐，总大小2048.
```
.macro  ventry  label
.align  7
b       \label
.endm

/********************************************/
/***************** vectors ******************/
.align 11
.globl vectors
vectors:
        ventry  sync_invalid_el1t                       // Synchronous EL1t
        ventry  irq_invalid_el1t                        // IRQ EL1t
        ventry  fiq_invalid_el1t                        // FIQ EL1t
        ventry  error_invalid_el1t                      // Error EL1t
        ventry  sync_invalid_el1h                       // Synchronous EL1h
        ventry  el1_irq                                 // IRQ EL1h
        ventry  fiq_invalid_el1h                        // FIQ EL1h
        ventry  error_invalid_el1h                      // Error EL1h
        ventry  sync_invalid_el0_64                     // Synchronous 64-bit EL0
        ventry  irq_invalid_el0_64                      // IRQ 64-bit EL0
        ventry  fiq_invalid_el0_64                      // FIQ 64-bit EL0
        ventry  error_invalid_el0_64                    // Error 64-bit EL0
        ventry  sync_invalid_el0_32                     // Synchronous 32-bit EL0
        ventry  irq_invalid_el0_32                      // IRQ 32-bit EL0
        ventry  fiq_invalid_el0_32                      // FIQ 32-bit EL0
        ventry  error_invalid_el0_32                    // Error 32-bit EL0

```


宏 ventry 用于描述向量表中每个向量条目 128 字节对齐. 

```
    .macro    ventry    label
    .align    7
    b    \label
    .endm
```
现在只关注中断这一部分的实现，在处理中断前需要先保存当前CPU 程序运行状态，这个通过 `kernel_entry` 函数完成，处理完中断后需要恢复到中断之前的状态,通过`kernel_exit` 来实现.

![image](https://documentation-service.arm.com/static/5f872814405d955c5176de29?token=)

```
el1_irq:
        kernel_entry
        bl      handle_irq
        kernel_exit
        

.macro  kernel_entry
        sub     sp, sp, #S_FRAME_SIZE
        stp     x0, x1, [sp, #16 * 0]
        stp     x2, x3, [sp, #16 * 1]
        stp     x4, x5, [sp, #16 * 2]
        stp     x6, x7, [sp, #16 * 3]
        stp     x8, x9, [sp, #16 * 4]
        stp     x10, x11, [sp, #16 * 5]
        stp     x12, x13, [sp, #16 * 6]
        stp     x14, x15, [sp, #16 * 7]
        stp     x16, x17, [sp, #16 * 8]
        stp     x18, x19, [sp, #16 * 9]
        stp     x20, x21, [sp, #16 * 10]
        stp     x22, x23, [sp, #16 * 11]
        stp     x24, x25, [sp, #16 * 12]
        stp     x26, x27, [sp, #16 * 13]
        stp     x28, x29, [sp, #16 * 14]
        str     x30, [sp, #16 * 15]
        .endm

.macro  kernel_exit
        ldp     x0, x1, [sp, #16 * 0]
        ldp     x2, x3, [sp, #16 * 1]
        ldp     x4, x5, [sp, #16 * 2]
        ldp     x6, x7, [sp, #16 * 3]
        ldp     x8, x9, [sp, #16 * 4]
        ldp     x10, x11, [sp, #16 * 5]
        ldp     x12, x13, [sp, #16 * 6]
        ldp     x14, x15, [sp, #16 * 7]
        ldp     x16, x17, [sp, #16 * 8]
        ldp     x18, x19, [sp, #16 * 9]
        ldp     x20, x21, [sp, #16 * 10]
        ldp     x22, x23, [sp, #16 * 11]
        ldp     x24, x25, [sp, #16 * 12]
        ldp     x26, x27, [sp, #16 * 13]
        ldp     x28, x29, [sp, #16 * 14]
        ldr     x30, [sp, #16 * 15]
        add     sp, sp, #S_FRAME_SIZE
        eret
        .endm
```
`kernel_entry`函数就是扩展栈空间256 字节，然后将`x0-x30`寄存器值全部压栈，保存执行现场, `kernel_exit`就是从栈空间出栈，将对应的值加载到`x0-x30`寄存器,返还栈空间, 然后执行 `eret` 指令, 返回到正常的执行流程.

#### 设置向量表

向量表内容填充好了, 但是处理器不知道它的位置还是无法使用它. 为了使异常处理有效, 我们必须将 `vbar_el1`(向量基址寄存器)设置为向量表地址，如下. 

```
    adr    x0, vectors        // load VBAR_EL1 with virtual
    msr    vbar_el1, x0        // vector table address
```

#### 屏蔽/取消屏蔽中断

ARM处理器状态有4位, 负责保持不同类型中断的屏蔽状态. 这些位定义如下. 

* **D**  屏蔽调试异常. 这些是同步异常的一种特殊类型. 出于明显的原因, 不可能屏蔽所有同步异常, 但是使用单独的标志可以屏蔽调试异常很方便. 
* **A** 屏蔽`SErrors`.
* **I** 屏蔽 `IRQs`.
* **F** 屏蔽 `FIQs`.

```
.globl enable_irq
enable_irq:
    msr    daifclr, #2
    ret

.globl disable_irq
disable_irq:
    msr    daifset, #2
        ret
```

### 定时器
#### 1. generic timer
ARMv8架构集成了`generic timer`的硬件设计，为系统提供了计时、计数以及触发timer event的功能.

ARM generic timer相关的硬件block如下图所示

![2.2.PNG](https://s2.loli.net/2022/05/25/9HLGNmqI31JWUiR.png)


从图可知每个generic timer分两部分构成
- 全局的`system counter`
- 各个processor 上的timer.

![image](https://documentation-service.arm.com/static/600eb3264ccc190e5e680237?token=)


#### `system counter`
System counter是一个永远在线的设备，它提供固定频率的系统计数. 该计数值会被广播到系统中的所有core，让各个core 之间时间同步。System counter的宽度在 56 bit到 64 bit之间，频率通常在 1MHz 到 50MHz 的范围内。 关于`system counter`的规格, 在 [Arm Architecture Reference Manual](https://developer.arm.com/documentation/ddi0487/latest) D8.1.2章节有详细的描述.


#### `Generic Timer`
所有`generic timer`的`physical counter`都来自`system counter`，可通过`CNTPCT_EL0` 寄存器来获取, 有 physical 就有virtual, processor可以通过`CNTVCT`寄存器访问`virtual counter`, 不过对于不支持security extension和virtualization extension的系统, `virtual counter`和`physical counter`是一样的值.
每个timer都会有三个寄存器,如下表
![2.1.PNG](https://s2.loli.net/2022/05/25/VGHJs9BvNEzL864.png)

```
CV: 64-bit CompareValue register.
    该寄存器配合system counter可以实现一个64 bit unsigned upcounter, 如果physical counter - CompareValue >= 0的话，触发中断.
    
TV: 32-bit TimerValue register.
   该寄存器配合system counter可以实现一个32 bit signed downcounter.如果<=0则触发中断.
   
Control: Counter-timer Physical Timer Control register
    该寄存器主要对timer进行控制, enable或是disable该timer，mask或者unmask该timer的interrupt signal.
    bit[2] ISTATUS
           1:定时器被触发
           0:定时器没有触发
           
    bit[1] IMASK
           1:interrupt mask, 定时器触发时不生成中断信号.
           0:not masked, 定时器触发生成中断信号，delivery 到Timer对应的core.
           
    bit[0] ENABLE
           1:Timer enabled.
           0:Timer disabled.
```

#### `Timer interrrupt`
timer 通过`CTL`寄存器配置是否生成中断，每个core timer 生成的中断只能分发到该core，不能分发到别的core.属于PPI,各Timer 与中断号的对于关系如下.

![2.3.PNG](https://s2.loli.net/2022/05/25/cAVBX2ab4WYZlqj.png)


```
//Control 设置为1,打开了Timer并会生成interrupt 信号.
static int generic_timer_init(void)
{
        asm volatile(
                "mov x0, #1\n"
                "msr cntp_ctl_el0, x0"
                :
                :
                : "memory");
        return 0;
}

//读取 system counter 频率，计算 TV 填充值
unsigned int clk = read_sysreg(CNTFRQ_EL0);
val = clk / HZ
asm volatile(
"msr cntp_tval_el0, %x[timer_val]"
:
: [timer_val] "r" (val)
: "memory");
```


![2.4.gif](https://s2.loli.net/2022/05/25/fRQ5Um72Fp9hLH8.gif)






