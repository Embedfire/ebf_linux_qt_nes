### 本代码来源于github： https://github.com/nejidev/arm-NES-linux，感谢前辈的开源

## 编译使用
1、首先安装gcc，或直接使用野火qt demo中的sdk均可
2、配置gcc环境变量

export PATH=/opt/arm-gcc/bin:$PATH

#编译
# arm-NES-linux emulate for InfoNES open source
## Planform
	Raspberry Pi or Arm linux or ubuntu
	Direct draw /dev/fb0 Framebuffer

## Build
	cd arm-NES-linux-/linux/
	make

## USE
	insmod joypad.ko
	cd arm-NES-linux-/linux/
	./InfoNES SuperMario.nes

