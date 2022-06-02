
### 前置知识点
##### 1. 常见汇编指令
- **`mrs`** : `Move System Register allows the PE to read an AArch64 System register into a general-purpose register` (读取系统寄存器值到通用寄存器).

- **`msr`** : `Move general-purpose register to System Register allows the PE to write an AArch64 System register from a
general-purpose register` (读取通用寄存器的值到系统寄存器).

- **`adr`** : `Load a label's relative address into the target register`.

- **`ldr`** : `calculates an address from a base register value and an offset register value, loads a word
from memory, and writes it to a register` (从基地址 + offset 处读取内存值到寄存器).

- **`mov`** : `copies a value from a register to the destination register` (从寄存器拷贝赋值给目标寄存器).

- **`isb`** : 它会清洗流水线，以保证所有它前面的指令都执行完毕之后，才执行它后面的指令.
- **`cbz`** : `and Branch on Zero compares the value in a register with zero, and conditionally branches to a label at a PC-relative offset if the comparison is equal` (将寄存器的值和0 比较，如果相等则跳转到label).
- **`lsr`** : `Logical Shift Right (register) shifts a register value right by a variable number of bits, shifting in zeros, and writes the result to the destination register.` 

![基本指令.png](https://s2.loli.net/2022/05/30/gjPkeQyOG4fL1ZJ.png)


##### 2. ARM寄存器
- sctlr_el2 : 系统控制寄存器.
```
bit 25:[0] little_endian
       [1] big-endian 

bit 12:[0] Disable instruction cache
       [1] Enable  instruction cache

bit 2 :[0] Disable data cache.
      :[0] Enable  data cache.

bit 0 :[0] EL2 stage 1 address translation disabled
      :[1] EL2 stage 1 address translation enabled
```
- HCR_EL2 : Hypervisor Configuration Register
```
bit 31：[0] : EL0/1 all is AARCH32
        [1] : EL1 is AARCH64
```

##### 3. 运行等级介绍
使用  ARM.v8 架构的CPU 有4个运行等级，分别为EL0、EL1、EL2、EL3, 其中EL0 运行等级最低，基本上只能访问通用寄存器、栈指针寄存器、`STR`、`LDR`，后面到EL3 运行等级越来越高. 为了实现进程隔离，用户空间的进程就是运行在EL0 等级，各进程只能看到自己的信息，而感知不到其他进程，也不能访问其他进程的地址空间.

而操作系统是运行在EL1 运行等级，该等级能够访问系统寄存器和配置管理进程资源. EL2 用于虚拟机场景，EL3 用于实现Trustzone 技术隔离REE 和TEE.


![tz_06.jpg](https://s2.loli.net/2022/05/25/vPhnGpVylETaUZq.jpg)


---
### 启动

在 linker.ld 链接文件中我们指定了 section `.text.boot`放在 kernel image 最开头，所以启动最开始执行的代码，我们需要放在这个 section 中，kernel 启动时会执行该 section 中的 start 函数.

* 读取  `mpidr_el1` 寄存器的值拿到process id, 取低8位，如果process_id 为0，则跳转执行`master`否则 hang 住.
* 在master 中读取`CurrentEL`寄存器，取该寄存器中的bit[3:2]，如果为2，则表示启动时运行在EL2 等级，则跳转到`el2_entry`,否则跳转到 `el1_entry`.
* 在`el2_entry` 中关闭 MMU，并设置EL1 运行状态为AArch64, 设置完 spsr_el2\elr_el2 寄存器后调用nret 指令，会以 EL1 的运行等级继续执行 elr_el2 的地址，即 el1_entry，从而完成了从EL2 切换到EL1.
* 在 el1_entry 中设置完 SP 之后就跳转到main 函数开始执行了.

```
.section ".text.boot"

.globl _start
_start:
	mrs	x0, mpidr_el1		
	and	x0, x0,#0xFF        // Check processor id
	cbz	x0, master            // Hang for all non-primary CPU
	b	proc_hang

proc_hang: 
	b 	proc_hang

master:
        mrs     x2, CurrentEL
        lsr     x3, x2, #2
        cmp     x3, #2
	    b.eq    el2_entry
        b       el1_entry


el2_entry:
	ldr x0, =SCTLR_EL2_VALUE_MMU_DISABLED
	msr sctlr_el2, x0

	/* The Execution state for EL1 is AArch64 */
	ldr x0, =HCR_HOST_NVHE_FLAGS
	msr hcr_el2, x0

	ldr x0, =SCTLR_EL1_VALUE_MMU_DISABLED
	msr sctlr_el1, x0

	ldr x0, =SPSR_EL1
	msr spsr_el2, x0

	adr x0, el1_entry
	msr elr_el2, x0
	eret

el1_entry:
	adr     x2, vectors
	msr     vbar_el1, x2
	isb
	adr	x0, bss_begin
	adr	x1, bss_end
	sub	x1, x1, x0
	bl 	memzero

	mov	sp, #LOW_MEMORY 
	bl	kernel_main
	b 	proc_hang		// should never come here
```



### 主函数
##### 1. 串口初始化
UART 相关的GPIO 描述如下.

![1.3.PNG](https://s2.loli.net/2022/05/25/u1HNkzU7CcxmtEe.png)



我们使用GPIO14/15作为串口的发送和接收引脚，对应表格可以看到我们需要使用这两个引脚的alt0 function.

```
GPFSEL0: GPIO0 ~ GPIO9 alt function setting
GPFSEL1: GPIO10 ~ GPIO19 alt function setting
       bit [14:12] : gpio14 alt setting, 100 is alt function 0.
       bit [17:15] : gpio15 alt setting, 100 is alt function 0.
```

![1.2.PNG](https://s2.loli.net/2022/05/25/OlMuKIZT7hzNLkU.png)


并将其配置为`no pull up/down`，完成基本的gpio 属性配置后就是波特率的设置了


![111.PNG](https://s2.loli.net/2022/05/27/1Y5TtFioQyPHcZV.png)


```
void uart_init(void)
{
        unsigned int selector;
        
        /* clean gpio14 */
        selector = readl(GPFSEL1); selector &= ~(7<<12);
        
        /* set alt0 for gpio14 */
        selector |= 4<<12;
        
        /* clean gpio15 */
        selector &= ~(7<<15);
        
        /* set alt0 for gpio15 */
        selector |= 4<<15;
        writel(selector, GPFSEL1);

        writel(0, GPPUD);
        delay(150);
        writel((1<<14) | (1<<15), GPPUDCLK0);
        delay(150);
        writel(0, GPPUDCLK0);

        /* disable UART until configuration is done */
        writel(0, U_CR_REG);

        /*
         * baud divisor = UARTCLK / (16 * baud_rate)
        = 48 * 10^6 / (16 * 115200) = 26.0416666667
        integer part = 26
        fractional part = (int) ((0.0416666667 * 64) + 
        generated baud rate divisor = 26 + (3 / 64) = 
        generated baud rate = (48 * 10^6) / (16 * 
        error = |(115177 - 115200) / 115200 * 100| = 
        */

        /* baud rate divisor, integer part */
        writel(26, U_IBRD_REG);
        /* baud rate divisor, fractional part */
        writel(3, U_FBRD_REG);

        /* enable FIFOs and 8 bits frames */
        writel((1<<4) | (3<<5), U_LCRH_REG);

        /* mask interupts */
        writel(0, U_IMSC_REG);
        /* enable UART, receive and transmit */
        writel(1 | (1<<8) | (1<<9), U_CR_REG);
}
```

##### 2. 打印 
这部分就先直接用 [A printf / sprintf Implementation for Embedded Systems](https://github.com/mpaland/printf) ,但是需要定义`PRINTF_DISABLE_SUPPORT_FLOAT` 宏来关闭浮点支持，并打开gcc 编译项`-mgeneral-regs-only`,打开后编译器会阻止使用浮点寄存器或者SIMD 高级寄存器.
```
-mgeneral-regs-only
    Generate code which uses only the general-purpose registers. This will prevent
    the compiler from using floating-point and Advanced SIMD registers but will
    not impose any restrictions on the assembler.
```
之所以需要关闭是因为在EL1/EL0 运行等级默认情况下，任何访问 SVE、高级SIMD、浮点寄存器的操作都会被trap到EL2或者EL1. 具体描述在 [CPACR_EL1, Architectural Feature Access Control Register](https://developer.arm.com/documentation/ddi0595/2021-12/AArch64-Registers/CPACR-EL1--Architectural-Feature-Access-Control-Register?lang=en), 另一种方法就是设置该寄存器的 FPEN 字段，设置为0b01\0b11 时 EL1 状态下访问这些寄存器都不会被捕获.



