
### ARMv8 相关寄存器
- **`TCR_ELx`**: 地址翻译控制寄存器

```
T0SZ, bits[5:0]   : 定义TTBR0_EL1 虚拟内存地址范围.
TG0, bit[15:14]   : 用户态 Page size, 0[4k]  1[64k]  2[16k].

IPS               : 物理内存地址宽度.

T1SZ, bits[21:16] : 定义TTBR1_EL1 虚拟内存地址范围.
TG1,  bits[31:30] : 内核态 Page size. 1[16k]  2[4k] 3[64k].
```

```math
VArange = 2^{64 - TXSZ}
```
对于`ARM64` 来说,虚拟地址最大宽度为48`bit`. 内核 `config` 配置中有`VA`、`PA` 和`page size` 的定义，这些定义最终都体现在`TCR_ELx`寄存器上.

```
CONFIG_ARM64_4K_PAGES=y
CONFIG_ARM64_VA_BITS_48=y
CONFIG_ARM64_VA_BITS=48
CONFIG_ARM64_PA_BITS_48=y
CONFIG_ARM64_PA_BITS=48
```
代码中用的是`VA_BITS` 宏来体现虚拟空间的大小.
```
#define VA_BITS            (CONFIG_ARM64_VA_BITS)
```
- **`TTBR0_ELx`**: 地址翻译表基址寄存器, 对应的输入虚拟地址范围 `[0, 0x0000ffffffffffff]`.
- **`TTBR1_ELx`**: 地址翻译表基址寄存器, 对应的输入虚拟地址范围 `[0xffff000000000000, 0xffffffffffffffff]`.
- **`MAIR_ELx`**:  内存属性寄存器. 提供可供`AttrIndx` 选择的内存属性,`AttrIndx` 为`block descriptor` 或者`L3 table descriptor` 中的成员. 具体属性含义可参考 `AArch64-Reference-Manual` 第 2609 页.
- **`FAR_ELx`**: 当`Instruction Abort`、`Data Abort`、`PC对齐中止`引发的异常时，该寄存器会保存出错的虚拟地址.
- **`ESR_EL1`**: EC = 0x24 时表示 `Data abort`, 此时`ISS bit[24:0]` 编码如下
![5.3_ESR_EL1_da.PNG](https://s2.loli.net/2022/06/02/dIyBGFahD2Qu3E8.png)
根据 `DFSC` 字段可知道 `Data abort`的错误码，部分错误码枚举如下
![5.3_ESR_EL1_dfs.PNG](https://s2.loli.net/2022/06/02/TZjWJRGqeArvup2.png)



### 页表格式
页表中每项为`8byte`, 关键 bit 解释如下

```
bit0 : 页表项是否有效，0 时表示输入的虚拟地址unmapped,任何访问操作会生成 Translation fault.
bit1 : 1：bit[47:12]给出的是下一级页表地址.
       0: 给出的是块内存的基地址及其属性.
```
下一级为页表时，页表项描述结构体如下
![5.1_PGD_descriptor.PNG](https://s2.loli.net/2022/05/31/4GL2efF6V3i5nmA.png)


下一级为块内存时，页表项描述结构体如下
![5.2_PMD_block_descriptor.PNG](https://s2.loli.net/2022/05/31/v1aL7gHGCEQonBp.png)

```
AttrIndx[2:0], bits[4:2] : 内存属性 index, 和 MAIR_EL1 寄存器组合使用.
AF, bit[10]              : access flag.
```


### VA 翻译
`页表等级`由页大小和虚拟地址宽度确定,计算公式如下，例如页大小为`4K`，虚拟地址宽度为`48bit` 时，使用4级页表.

```
#define ARM64_HW_PGTABLE_LEVEL_SHIFT(n)	((PAGE_SHIFT - 3) * (4 - (n)) + 3)
```
各等级的索引 如下

![5.1_translation_table.PNG](https://s2.loli.net/2022/05/30/gUezbLIWCYc8wGD.png)

- **`页内偏移`**：相对于页地址的偏移量, 最大为`page size`,所以占`PAGE_SHIFT` bit;  页大小为`4K`时, `PAGE_SHIFT`=12.
- **`PT index`** ：页表索引. 页表项格式如上面章节描述, 占8个字节，页表总大小为一个`page size`,则页表有`page size/8` 个项, 所以索引占用`PAGE_SHIFT - 3` bit; 页大小为`4K` 时，页表有 512 项, 索引占用 `9bit`.
- **`PMD index`** : 页表中级目录索引，和`PT index`类似，占用`PAGE_SHIFT - 3` bit; 页大小为`4K` 时, 占用 `9bit`.
- **`PUD index`** : 页表高级目录索引，和`PMD index`类似，占用`PAGE_SHIFT - 3` bit; 页大小为`4K` 时, 占用 `9bit`.
- **`PGD index`** : 页表全局目录索引，和`PUD index`类似，占用`PAGE_SHIFT - 3` bit; 页大小为`4K` 时, 占用 `9bit`.





![5_1_memory_mapping.PNG](https://s2.loli.net/2022/05/31/dwzZusFTUYPbC59.png)


### 初始化

##### VA、PA、Page Size 配置
将`tcr_el1` 寄存器的 `T0SZ`,`T1SZ` 位设置为16 来配置 `VA` 位宽为 48; `TG0` 部分配置为 0 表示用户态`page size` 为4K; `TG1` 部分配置为2 表示内核态`page size` 为4K.

```
#define TCR_T0SZ			(64 - 48) 
#define TCR_T1SZ			((64 - 48) << 16)
#define TCR_TG0_4K			(0 << 14)
#define TCR_TG1_4K			(2 << 30)
#define TCR_VALUE			(TCR_T0SZ | TCR_T1SZ | TCR_TG0_4K | TCR_TG1_4K)

ldr	x0, =(TCR_VALUE)		
msr	tcr_el1, x0
```

##### 内存属性配置
定义了两组内存 Attribute: `Attr0` , `Attr1`.
- `Attr0`: 0 , 设备内存, `Device-nGnRnE memory`;内存访问不合并，按照`program order`执行，禁止指令重排;ack 必须来自最终目的地.
- `Attr1`: 0x44 , 正常内存, Inner + Outer  Non-cacheable.


```
#define MT_DEVICE_nGnRnE 		    0x0
#define MT_NORMAL_NC			    0x1
#define MT_DEVICE_nGnRnE_FLAGS		0x00
#define MT_NORMAL_NC_FLAGS  		0x44
#define MAIR_VALUE    (MT_DEVICE_nGnRnE_FLAGS << (8 * MT_DEVICE_nGnRnE)) | (MT_NORMAL_NC_FLAGS << (8 * MT_NORMAL_NC))
ldr	x0, =(MAIR_VALUE)

msr	mair_el1, x0
```
这些属性在页表初始化时通过 `index` 来引用.

##### 多级页表创建
`BCM2837` 地址分布如下，我们先只关注`0x40000000` (1G) 以下的空间, 这部分分成两部分
- `0x3f000000` 以下为常规内存.
- [`0x3f000000` ~ `0x40000000`] 这段物理地址是留给外设的.

1G 空间的映射只需要`PGD`,`PUD`,`PMD` 即可完成.

![image.png](https://s2.loli.net/2022/06/01/hRvufdPW3Jnl5wi.png)

内核虚拟地址从`0xffff000000000000` 开始, bit[63:48] 全部为1, 使用`TTBR1_EL1`来缓存`PGD`地址. 下面代码调用`create_pgd_entry` 宏在`pg_dir`内存 处创建填充了页表，然后将`pg_dir` 的地址存入`ttbr1_el1`.

```
.macro	create_pgd_entry, tbl, virt, tmp1, tmp2
	create_table_entry \tbl, \virt, PGD_SHIFT, \tmp1, \tmp2
	create_table_entry \tbl, \virt, PUD_SHIFT, \tmp1, \tmp2
	.endm

	.macro	create_table_entry, tbl, virt, shift, tmp1, tmp2
	lsr	\tmp1, \virt, #\shift
	and	\tmp1, \tmp1, #PTRS_PER_TABLE - 1			// table index
	add	\tmp2, \tbl, #PAGE_SIZE
	orr	\tmp2, \tmp2, #MM_TYPE_PAGE_TABLE	
	str	\tmp2, [\tbl, \tmp1, lsl #3]
	add	\tbl, \tbl, #PAGE_SIZE					// next level table page
	.endm
	
	adrp	x0, pg_dir
	msr	ttbr1_el1, x0
```
`create_pgd_entry`创建了 `PGD` 和`PUD`，但只用了他们的 `index0` 这一个表项，如下图，实际上这两级页表位置可以不连续. 这两级页表项都还是页表属性,下一级创建的是`block entity`

![5_2_memory_mapping.PNG](https://s2.loli.net/2022/06/01/dCoA6pNxlXtVHDq.png)

##### MMU Enable
将 `sctlr_el1` bit0 置位即打开了MMU.







