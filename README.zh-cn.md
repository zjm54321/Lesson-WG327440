# 8051 Nix 模板

Intel MCS-51（通常称为 8051）是 Intel 于 1980 年为嵌入式系统推出的单片微控制器（MCU）系列。

本模板提供了使用 SDCC（Small Device C Compiler）工具链与 XMake 构建系统来开发 8051 项目的基础配置，包含编译 C 与汇编、链接以及面向代码体积优化的配置。

## 特性
- 通过 [SDCC](https://sdcc.sourceforge.net/) 使用 C 进行编程
- 使用 [XMake](https://xmake.io/#/) 的构建系统
- 基于 [clangd](https://clangd.llvm.org/) 的 LSP 支持
- 使用 [emu8051](https://github.com/cjacker/emu8051) 进行仿真
- 支持直接烧录

## 使用方法
1. 克隆仓库
2. 进入项目目录
3. 若安装了 direnv，运行 `direnv allow` 或执行 `nix develop` 进入开发环境
4. 使用 `xmake f -c [chip]` 设置目标芯片（例如：`xmake f -c STC15`）
5. 使用 `xmake` 构建项目
6. 在 `main.c` 的首行加入 `#include <clangd.h>` 以启用 clangd LSP 功能；加入 `#include <keil.h>` 以支持 Keil 风格代码。或使用 `#include <utility.h>` 同时支持两者。

> [!WARNING]
> `#include <clangd.h>` 或 `#include <utility.h>` 必须位于`main.c`的第一行以激活 clangd。

## 命令
- `xmake` - 构建项目
- `xmake build` - 构建项目
- `xmake format` - 格式化源码

- `xmake emulator` - 仿真程序
- `xmake flash` - 烧录程序到单片机

- `xmake clean` - 清理构建产物

## 烧录工具
| 平台 | 工具     | 可用   |
| ---- | -------- | ------ |
| STC  | stcflash | 是     |

## Todo
- [ ] 支持调试工具
- [ ] 支持更多烧录工具
- [ ] 支持 edsim51
- [ ] 支持 naken-asm

## 示例

点灯
```C
#include <utility.h>
#include <softdelay.h>
#include <stc51.h>

int main(void) {
    for (;;) {
        P00 = !P00;
        delay500ms();
    }
}
```

# 许可证

我是以下免责声明的每个文件的作者：
```bash
# @author zjm54321
```

我根据 [GNU GPL-3.0](./LICENSE) 许可证对它们进行许可。在适用法律允许的范围内，不提供任何保证。

一些脚本或配置文件来自其他人，应该标注对相应作者的致谢。

# 参考
[opensource-toolchain-8051](https://github.com/cjacker/opensource-toolchain-8051#for-stc8051-from-stc)
