# C语言实现SHA3-256和SHA3-384

本部分中我们根据 NIST.FIPS.202 标准实现了SHA3-224、SHA3-256、SHA3-384、SHA3-512四种算法，并进行了一定的优化。正确性验证方面，我们使用官方提供的不同长度bit测试样例进行了测试，并使用python库hashlib进行字节流测试和性能测试。

## 作业题目

### 正确性验证（调用标准库验证）

正确性验证我们使用了官方提供的6中比特流样例以及python的hashlib库进行了验证。其中官方样本验证的是比特流压缩的正确性，用hashlib比对验证的则是字节流压缩的正确性。

- 比特流压缩正确性
  
  官方对四种算法在0、5、30、1600、1605、1630bit输入下的正确性验证给出了相关样例，我们使用这些样例测试了算法的正确性。可以利用 test_example.py 文件运行测试。在所有的测试样例中，我们的算法结果都与标准答案匹配。

- 字节流压缩正确性
  
  python的hashlib库中实现了SHA3的四种算法，但只限于用字节流输入。我们利用hashlib作为标准库对比压缩结果，实现在 test_byte.py 文件中。我们在下面的数据压缩效率测试中使用的也是 test_byte.py 文件，得到的结果均匹配。下面是一些样例对比。

    ```
    python3 test_byte.py -x SHA3-512 -s crypto
    expect: 601057e930458c0ad2deb75f514fa6e31149877287495c1959e1528bc686071dde5e99ec31a24452b2731409bc3f7521a4d5da00b116e21f5c88ef53765a8ee5
    oldver: 601057e930458c0ad2deb75f514fa6e31149877287495c1959e1528bc686071dde5e99ec31a24452b2731409bc3f7521a4d5da00b116e21f5c88ef53765a8ee5
    Time Cost(without IO):0.000020
    result: 601057e930458c0ad2deb75f514fa6e31149877287495c1959e1528bc686071dde5e99ec31a24452b2731409bc3f7521a4d5da00b116e21f5c88ef53765a8ee5
    Time Cost(without IO):0.000004
    ```

    ```
    python3 test_byte.py -x SHA3-256 -s 2022
    expect: 698a9960d79aed947bb067260d18f5ee074ca40ebef9be26ebb1f21214199eb2
    oldver: 698a9960d79aed947bb067260d18f5ee074ca40ebef9be26ebb1f21214199eb2
    Time Cost(without IO):0.000020
    result: 698a9960d79aed947bb067260d18f5ee074ca40ebef9be26ebb1f21214199eb2
    Time Cost(without IO):0.000006
    ```

### 分别对16K，4M的数据进行压缩，测试实现效率

在Intel(R) Core(TM) i5-7500 CPU @ 3.40GHz下进行测试，调用 test_byte.py 进行字节流输入的长字符串压缩测试，结果如下。

相关命令：

    python3 test_byte.py -x SHA3-224 -f 16k.txt

| 测试项目     | 未优化版本用时(s) | 优化后版本用时(s) |
| ----------- | ----------- |-----------|
| SHA3-224(16K)    | 0.002102 | 0.000253 |
| SHA3-256(16K)    | 0.002203 | 0.000267 |
| SHA3-384(16K)    | 0.002905 | 0.000355 |
| SHA3-512(16K)    | 0.004135 | 0.000547 |
| SHA3-224(4M)    | 0.519772 | 0.064416 |
| SHA3-256(4M)    | 0.549819 | 0.068430 |
| SHA3-384(4M)    | 0.717020 | 0.088287 |
| SHA3-512(4M)    | 1.038731 | 0.127004 |

## 基础方案实现和性能优化

### 基础方案实现

基础方案实现在mysha3_old.c中，我们严格按照 NIST.FIPS.202 标准实现，算法各步骤的操作和命名与标准中一致。

### 性能优化

我们注意到SHA3中theta-iota共5个函数被反复调用较多次，因此主要对这一部分进行了优化。主要通过预处理的方法去除算法中被反复计算的下标或者数值。

theta：

    1、有 (x - 1) % 5 和 (x + 1) % 5 两个取模运算开销较大，利用两个数组预处理，用访存代替计算。

rho：

    1、((t + 1)*(t + 2) / 2) % 64 生成的序列在每次计算中是相同的，通过预处理计算出结果，用访存代替计算。
    2、(x, y)在每次函数调用中生成的点序列是相同的，通过预处理计算出结果，用访存代替计算。

pi：

    1、(x + 3 * y) % 5 中包含取模运算，开销较大，同时每次调用中生成的结果序列相同，通过预处理计算出结果，用访存代替计算。

chi：

    1、(x + 2) % 5 有取模运算，类似theta利用数组预处理，用访存代替计算

iota：

    1、iota在标准中只有24种输入可能，调用的rc函数也类似，并且iota只有最后一步才与A矩阵有交互。因此我们可以预处理出24种参数下，最终和A[0][0]求异或的RC值，用数组存储，用访存代替计算。

## 仓库结构

### mysha3.c & mysha3_old.c

mysha3_old.c是严格按照 NIST.FIPS.202 标准实现的c语言程序。mysha3.c则是基于前者优化的c语言程序，预处理了部分操作以提升性能，具体细节参考性能优化部分介绍。

### makefile

编译mysha3.c和mysha3_old.c生成可执行文件mysha3和mysha3_old

### mysha3 & mysha3_old

有比特流输入和字节流输入两种模式。比特流输入直接从控制台输入01字符串，解释为bit串。字节流输入从文件读取字符串，每个字符解释为一个字节。（mysha3和mysha3_old用法相同，下以mysha3为例）

    比特流:./mysha3 -x <method编号> -m <任意二进制串>
        <method>: 
            0 -> SHA3-224 
            1 -> SHA3-256 
            2 -> SHA3-384 
            3 -> SHA3-512
        <任意二进制串>: 只能输入01串，每个字符会被解释为1bit。bytetest.py代码调用时会将字符串翻译为二进制串。
    字节流:./mysha3 -x <method编号> -f <文件名>
        <文件名>: 字符串文件，每个字符会被解释为一个字节。性能测试环节使用的文件用filegen.py生成。

### test_example.py

测试脚本1，按照官方给出的样例测试文件中的测例进行测试，包括0、5、30、1600、1605、1630bit的六种测例。调用mysha3和mysha3_old进行测试，依次进行SHA3-224、SHA3-256、SHA3-384、SHA3-512的测试，输出结果与官方给出的结果作比对。

运行: 
    
    python3 test_example.py

结样样例：

    message5:                           // 5bit测例
    expect: xxx                         // 标答
    oldver: xxx                         // mysha3_old结果
    Time Cost(without IO):0.000020      // mysha3_old耗时
    result: xxx                         // mysha3结果
    Time Cost(without IO):0.000004      // mysha3耗时

### test_byte.py

测试脚本2，调用python库hashlib作为标准进行测试，包括字符串输入和文件输入两种模式。由于hashlib只能接收字节流的输入，因此本文件的测试均使用字节流输入方法。

    字符串输入:python3 test_byte.py -x <method> -s <任意字符串>
        <method>: SHA3-224 | SHA3-256 | SHA3-384 | SHA3-512
        <任意字符串>: 每个字符会被解释为8bit
    文件输入:python3 test_byte.py -x <method> -f <文件名>
        <method>: SHA3-224 | SHA3-256 | SHA3-384 | SHA3-512
        <文件名>: 字符串文件文件名，每个字符会被解释为8bit。可以用filegen.py生成随机文件。

结样样例：

    expect: xxx                         // hashlib标答
    oldver: xxx                         // mysha3_old结果
    Time Cost(without IO):0.000020      // mysha3_old耗时
    result: xxx                         // mysha3结果
    Time Cost(without IO):0.000004      // mysha3耗时

### test_charfilegen.py

随机字符串文件生成脚本，生成随机的16K，4M数据量文件。为避免复杂的字符串处理，使用的随机数范围是可见字符范围（32-126）。

运行: 

    python3 test_charfilegen.py