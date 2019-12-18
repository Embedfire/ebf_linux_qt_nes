### 本代码来源于github： https://github.com/nejidev/arm-NES-linux  感谢前辈的开源

## 编译使用
1. 首先安装交叉编译器：可参考[Linux的编译环境](https://github.com/Embdefire/ebf_6ull_linux)
2. Linux上编译需要依赖相关的库文件：`libnes-arm.tar.bz2`，可在[https://github.com/Embdefire/ebf_linux_qt_nes/releases](https://github.com/Embdefire/ebf_linux_qt_nes/releases)页面下载！，也可在终端通过命令获取：
```bash
wget https://github.com/Embdefire/ebf_linux_qt_nes/releases/download/v1.0/libnes-arm.tar.bz2
```
3. 将下载好的`libnes-arm.tar.bz2`文件解压到`/usr/local/`目录下：
```bash
sudo tar -jxf libnes-arm.tar.bz2 -C /usr/local/
```
4. 进入源码的linux目录下，然后编译：
```bash
cd linux/
make
```
5. 此时当前目录下会生成一个 `InfoNes` 可执行文件，与Qt 配合使用即可，将其拷贝到Qt App相同的目录下即可！！

## 注意事项
`InfoNes` 可执行文件 是需要与野火Qt配合使用才会有图形界面，单独运行是不会有图形界面的！！

## 编译错误
当遇到以下错误就表明找不到相关的依赖文件，检查`/usr/local/`目录下是否存在`libnes-arm/libasound-arm/`、`libnes-arm/libzlib-arm/`等文件夹！
```bash
arm-linux-gnueabihf-gcc  -c -O2 -fsigned-char  ../InfoNES_pAPU.cpp  -o ../InfoNES_pAPU.o -I /usr/local/libnes-arm/libasound-arm/include
arm-linux-gnueabihf-gcc  -c -O2 -fsigned-char  InfoNES_System_Linux.cpp  -o InfoNES_System_Linux.o -I /usr/local/libnes-arm/libasound-arm/include
InfoNES_System_Linux.cpp:23:28: fatal error: alsa/asoundlib.h: No such file or directory
 #include "alsa/asoundlib.h"
                            ^
compilation terminated.
Makefile:27: recipe for target 'InfoNES_System_Linux.o' failed
make: *** [InfoNES_System_Linux.o] Error 1
```