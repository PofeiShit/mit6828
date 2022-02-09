# lab1

## makefile 细节
```
$(OBJDIR)/boot/%.o: boot/%.S # %.o : %.s 把所有的.S文件编译成.o
    @echo + as $< # $< 表示boot.S
    @mkdir -p $(@D) # 
    $(V)$(CC) -nostdinc $(KERN_CFLAGS) -c -o $@ $<  # $@ 表示boot.o, $< 表示boot.S
```

```
1. $@--目标文件，$^--所有的依赖文件，$<--第一个依赖文件。

2. $(@D)

The directory part of the file name of the target, with the trailing slash removed. If the value of $@ is dir/foo.o then $(@D) is  dir. This value is . if $@ does not contain a slash.

3. @mkdir xxx / @echo xxx 加上@不会在shell显示mkdir或者echo命令，只运行命令。不加@则显示这条命令且运行命令。

4. -nostdinc:  ignores standard C include directories,在编译系统内核的时候会很有用，因为系统内核不使用标准C库，内核使用的是源码里面自带的库文件

```

```
$(OBJDIR)/boot/boot: $(BOOT_OBJS)
    @echo ld boot/boot
    $(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7c00 -o $@.out $^ # 把所有的依赖的文件都链接成boot.out
    $(V)$(OBJDUMP) -S $@.out >$@.asm
    $(V)$(OBJCOPY) -S -O binary -j .text $@.out $@ # 把boot.out中的.text段单独输出成boot文件
```


```
-N
--omagic
    Set the text and data sections to be readable and writable. Also, do not page-align the data segment. If the output format supports Unix style magic numbers, mark the output as OMAGIC.

-e entry
--entry=entry
    Use entry as the explicit symbol for beginning execution of your program, rather than the default entry point. See section The Entry Point, for a discussion of defaults and other ways of specifying the entry point.
符号start作为程序的入口。

-Ttext org
    Use org as the starting address for--respectively--the bss, data, or the text segment of the output file. org must be a single hexadecimal integer; for compatibility with other linkers, you may omit the leading `0x' usually associated with hexadecimal values.
```

1.  地址0x7c00更改了汇编代码中的标号(.msg, .start)在内存中的地址，指令中有mov 内存地址到寄存器的指令，地址就会相应的改变,寄存器就去该内存地址读取内容。但是系统运行，整个boot不受-Ttext的地址影响，一直被加载到内存地址0x7c00。

2. 也就是说将0x7c00改成0x8c00或者其他地址。boot程序依旧加载到0x7c00的起始地址。但是如果boot程序里面有需要把内存地址加载到寄存器中的指令，该指令保存的地址就会改变(比如变成0x8c00)。比如是要显示"Hello OS",这几个字符串一直都在0x7c00起始内存地址块内。但是指令是加载了0x8c00之后的内容会显示乱七八糟的东西.

```
-O bfdname
--output-target=bfdname
    Write the output file using the object format bfdname. (binary file descriptor)，二进制格式的输出文件

-j sectionname
    --only-section=sectionname
    Copy only the named section from the input file to the output file. This option may be given more than once. Note that using this option inappropriately may make the output file unusable. 将代码段.text从boot.out拷贝到boot中
```

## boot.S

### cli
---
禁止硬件中断发生(比如移动鼠标,键盘)，保护当前运行的代码不被打断，起到保护代码运行的作用

### cld
---
这条指令用于指定之后发生的串处理操作的指针移动方向，大致了解就够

### Set up the important data segment registers (DS, ES, SS).
---
```
  xorw    %ax,%ax             # Segment number zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment
```
这几条命令主要是在把三个段寄存器，ds，es，ss全部清零，因为经历了BIOS，操作系统不能保证这三个寄存器中存放的是什么数。所以这也是为后面进入保护模式做准备。


```
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64，对应状态寄存器，通知要写入数据
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60 真正写入数据,开启A20，实现突破1M内存
  outb    %al,$0x60
```
把A20地址线控制和键盘控制器的一个输出进行 AND 操作，这样来控制 A20 地址线的打开（使能）和关闭（屏蔽\禁止）。一开始时 A20 地址线控制是被屏蔽的（总为 0），直到系统软件通过一定的 IO 操作去打开它。在保护模式下，为了使能所有地址位的寻址能力，需要打开 A20 地址线控制，即需要通过向键盘控制器 8042 发送一个命令来完成。键盘控制器 8042 将会将它的的某个输出引脚的输出置高电平，作为 A20 地址线控制的输入。

理论上讲，我们只要操作 8042 芯片的输出端口（64h）的 bit 1，就可以控制 A20 Gate，但实际上，当你准备向 8042 的输入缓冲区里写数据时，可能里面还有其它数据没有处理，所以，我们要首先禁止键盘操作，同时等待数据缓冲区中没有数据以后，才能真正地去操作 8042 打开或者关闭 A20 Gate

https://learningos.github.io/ucore_os_webdocs/lab1/lab1_appendix_a20.html

https://blog.theerrorlog.com/the-funny-design-of-a20-zh.html

http://leenjewel.github.io/blog/2014/07/29/%5B(xue-xi-xv6)%5D-cong-shi-mo-shi-dao-bao-hu-mo-shi/


```
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE_ON, %eax
  movl    %eax, %cr0
```
在进入保护模式之前还需要将GDT准备好,Global Description Table.想要在保护模式下对内存寻址就先要有GDT。表中每一项叫做“段描述符”，用来记录每个内存分段的一些属性信息，每个段描述符占8字节。
```
gdt:
  SEG_NULL				# null seg
  SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)	        # data seg
```
宏计算直接翻译过来:
```
gdt:
  .word 0, 0;
  .byte 0, 0, 0, 0                             # 空
  .word 0xffff, 0x0000;
  .byte 0x00, 0x9a, 0xcf, 0x00                 # 代码段
  .word 0xffff, 0x0000;
  .byte 0x00, 0x92, 0xcf, 0x00                 # 数据段
```
可以看到定义了一个空段，代码段，数据段
然后我们再把代码段和数据段的段描述符具体每一位的对应值表展示出来，首先是代码段：
```
// 高32位
31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
            基地址	             G	 DB	 XX	 AA	        Limit	 P	   DPL	  S	  E	 ED	 RW	 A	            基地址
            0x00	            1	1	0	0	        0xf	    1	  00	 1	 1	0	1	0	            0x00
// 低32位
                                基地址	                                                Limit
                                0x0000	                                               0xffff
```
DB=1, 1 表示地址和操作数是 32 位，0 表示地址和操作数是 16 位
G=1,  1 表示 20 位段界限单位是 4KB，最大长度 4GB；一块段内存的大小可以设置4KB~4GB
三块“基地址”组装起来正好就是 32 位的段起始内存地址，两块 Limit 组成该内存分段的长度
基地址都是 0x00000000，内存分段长度都是 0xfffff。这说明他们都是用于 32 位寻址，所使用的内存是从 0 开始到 4GB 结束（全部内存）
0xfffff=2^20。表示有这么多个段，每个段的大小4KB，两者相乘正好4GB
```
// 高32位
31	30	29	28	27	26	25	24	23	22	21	20	19	18	17	16	15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
            基地址	             G	 DB	 XX	 AA	       Limit	 P	   DPL	  S	  E	 ED	 RW	 A	            基地址
            0x00	            1	1	0	0	       0xf	    1	  00	 1	 0	0	1	0	            0x00
// 低32位
                                基地址	                                                Limit
                                0x0000	                                               0xffff
```
GDT搞定后，接下来就要把我们刚刚在内存中设定好的 GDT 的位置告诉 CPU。CPU 单独为我们准备了一个寄存器叫做 GDTR 用来保存我们 GDT 在内存中的位置和我们 GDT 的长度。GDTR 寄存器一共 48 位，其中高 32 位用来存储 GDT 在内存中的位置，其余的低 16 位用来存 GDT 有多少个段描述符。16位最多可以表示65536种情况。一个段描述符是8个字节，占用8种情况。所以 GDT 最多可以有 8192 个段描述符.专门提供了一个指令用来让我们把 GDT 的地址和长度传给 GDTR 寄存器
```
lgdt gdtdesc

gdtdesc:
  .word   (gdtdesc - gdt - 1)                            # sizeof(gdt) - 1
  .long   gdt                             # address gdt
```
## ljmp $PROT_MODE_CSEG, $protcseg
---
```
PROT_MODE_CSEG=0x08
15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
0	  0	  0	  0	  0	  0	  0	0	0	0	0	0	1	0	0	0
```
这是一个跳转语句，通知 CPU 跳转到指定位置继续执行指令。这个跳转语句的两个参数就是典型的“基地址” + “偏移量”的方式告诉 CPU 要跳转到内存的什么位置去继续执行指令。而这时我们已经在分段式的保护模式下了.分段式保护模式下“段基址”（基地址）不再是内存地址，而是 GDT 表的下标.上面我们也说过 GDT 表最大可以有 8192 个表项（段描述符），2^13 = 8192，所以保存着“段基址”的 16 位段寄存器只需要其中的 13 位就可以表示一个 GDT 表的下标，其余的 3 位可用作他用。

这里这个 16 位的“段基址”的高 13 位代表 GDT 表的下标（学名应该叫“段选择子”），这里高 13 位刚好是 1，而我们的 GDT 里下标位 1 的内存段正好是我们的“代码段”。而“代码段”我们在 GDT 的“段描述符”中设置了它的其实内存地址是 0x00000000 ，内存段长度是 0xfffff，这是完整的 4GB 内存。

所以这里的跳转语句选择了“代码段”，由于“代码段”的起始内存地址是 0x00000000 ，长度是完整的 4GB，所以后面的“偏移量”仍然相当于是实际的内存地址，所以这里“偏移量”直接用了 $protcseg，也就是 protcseg 直接对应的代码位置。通过这个跳转实际上 CPU 就会跳转到 boot.S 文件的 protcseg 标识符处继续执行了。

## movw $PROT_MODE_DSEG, %ax
---
```
PROT_MODE_DSEG=0x10
15	14	13	12	11	10	9	8	7	6	5	4	3	2	1	0
0	  0	  0	  0	  0	  0	  0	0	0	0	0	1	0	0	0	0
```
这里高 13 位刚好是 2，而我们的 GDT 里下标位 2 的内存段正好是我们的“数据段”。
这里将所有数据段寄存器都赋值成数据段段选择子的值

##
---
最后参考30天自制操作系统的代码，添加如下
```
#define LEDS	0x0ff1
#define VMODE   0x0ff2
#define SCRNX   0x0ff4
#define SCRNY   0x0ff6
#define VRAM    0x0ff8
```
配合demo.c将屏幕显示白色。boot.S作用开启保护模式，然后call bootmain。bootmain的定义在main.c中

## main.c
---
bootmain是引导工作的最后部分，负责将内核从硬盘上加载到内存中，然后开始执行内核中的程序。
bootmain中的第一个readseg将内核中的一页(4kb)大小的数据加载到物理地址的0x10000。目的只是为了读取头部信息。使用readelf -h kernel可以看到ELF header信息
```
  #define ELFHDR ((struct Elf*)0x10000)
  // ph指向程序头在内存中的地址。ELFHDR->e_phoff(52字节)
	ph = (struct Proghdr *)((uint8_t *)ELFHDR + ELFHDR->e_phoff);
  // program header条目数有1个
  eph = ph + ELFHDR->e_phnum;
  // 将相对于ELF文件头p_offset位置处的p_memsz大小的程序加载到物理地址p_pa处
  		readseg(ph->p_pa, ph->p_memsz, ph->p_offset);
  p_offset=0x001000
  p_memsz=0x0006c
  p_pa=0x0010000

```
### readseg
```
pa &= ~(SECTSIZE - 1); => pa &= 0xFFFF FE00 确保pa地址指向内存扇区的第一个字节,将pa的低位全部置为0
offset = (offset / SECTSIZE) + 1; 将偏移量转换为扇区偏移。跳过hard disk的boot扇区。
```
一次读取一个扇区的数据到内存中，直到pa > end_pa
```
	((void(*)(void))(ELFHDR->e_entry))(); // 从内核 ELF 文件入口点开始执行内核,也就是执行内存0x100000的代码
```

### kernel.ld
暂时还没有把虚拟地址和物理地址的映射代码加进来。所以只要这两个地址设置相同即可。比如设置0x200000.然后用readelf -l kernel查看虚拟地址和物理地址都是0x200000.
```
SECTIONS
{
  . = 0x100000  指定section虚拟地址
  .text : AT(0x100000) 指定该sections的加载地址(physical memory) 
}
```
### inc/elf.h
```
// ELF header
// 详见 https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-43405/index.html
struct Elf {
    uint32_t e_magic;       // must equal ELF_MAGIC
    uint8_t e_elf[12];      // 机器无关数据，通过它们可以解码和解释文件的内容
    uint16_t e_type;        // 标识目标文件类型
    uint16_t e_machine;     // 指定单个文件所需的机器架构
    uint32_t e_version;     // 标识目标文件版本
    uint32_t e_entry;       // 程序入口，虚拟地址
    uint32_t e_phoff;       // program header table的文件偏移量。指与文件起始位置的offset
    uint32_t e_shoff;       // section header table的文件偏移量
    uint32_t e_flags;       // 与文件关联的特定于处理器的flags
    uint16_t e_ehsize;      // ELF头的大小，以字节为单位
    uint16_t e_phentsize;   // program header table一项的大小，每一项大小都相同
    uint16_t e_phnum;       // program header条目数
    uint16_t e_shentsize;   // section header table一项的大小，每一项大小都相同
    uint16_t e_shnum;       // section header条目数
    uint16_t e_shstrndx;    // 与section name string table相关的项的section header table索引
};
```

```
// program header
// 每个program header描述一个segment
// 一个segment由几个section组成，为能被映射进内存映像的最小独立单元
// 详见 https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-83432/index.html
struct Proghdr {
    uint32_t p_type;        // segment类型
    uint32_t p_offset;      // segment相对于ELF文件开头的偏移
    uint32_t p_va;          // segment的第一个字节在内存中的虚拟地址
    uint32_t p_pa;          // 物理地址
    uint32_t p_filesz;      // segment的文件映像中的字节数
    uint32_t p_memsz;       // segment的内存映像中的字节数
    uint32_t p_flags;       // 与segment相关的flags，读写执行权限
    uint32_t p_align;       // 在内存和文件中的对齐
};
```

```
// section header
// section header table可以帮助你定位该文件所有的sections
// section为ELF文件中能被处理的最小不可分割单元，比如.text .rodat .data
// 详见 https://docs.oracle.com/cd/E19683-01/816-1386/chapter6-94076/index.html
struct Secthdr {
    uint32_t sh_name;       // The name of the section
    uint32_t sh_type;       // 对section的内容和语义进行分类
    uint32_t sh_flags;      // flags
    uint32_t sh_addr;       // section的地址
    uint32_t sh_offset;     // 从文件起始处到section第一个字节的偏移量
    uint32_t sh_size;       // section的大小
    uint32_t sh_link;       // section header table index
    uint32_t sh_info;       // Extra information
    uint32_t sh_addralign;  // 地址对齐
    uint32_t sh_entsize;    // 定长的条目表(如符号表)的大小
};
```

## x86.h

```
    asm volatile("outw %0, %w1" : : "a" (data), "d" (port));
```
"asm"表示后面的代码为内嵌代码。"volatile"表示编译器不要优化代码,后面的指令保留原样。

__asm__(汇编语句模板: 输出部分: 输入部分: 破坏描述部分)

共四个部分:汇编语句模板,输出部分,输入部分,破坏描述部分,各部分使用”:“格开,
汇编语句模板必不可少,其他三部分可选,
如果使用了后面的部分,而前面部分为空,也需要用”:"格开,相应部分内容为空


输入部分描述输入操作数,不同的操作数描述符之间使用逗号格开,每个操作数描述符由限定字符串和C语言表达式或者C语言变量组成。

## Kern/Makefrag
---
```
wildcard: 扩展通配符
patsubst: 替换通配符

KERN_OBJFILES := $(patsubst %.c, $(OBJDIR)/%.o, $(KERN_SRCFILES))

patsubst把$(KERN_SRCFILES)中的变量符合后缀是.c的全部替换成.o

KERN_OBJFILES := $(patsubst $(OBJDIR)/lib/%, $(OBJDIR)/kern/%, $(KERN_OBJFILES))

obj/lib/printf.o转换成obj/kern/printf.o

将所有相关的.o生成到kern文件夹下
```

