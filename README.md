# LiteText

轻量级跨平台代码编辑器，基于 Qt 6 和 C++17 开发。

## 特性

- 深色主题，支持多种配色方案（VS Code Dark+、GitHub Dark、高对比）
- 语法高亮：C/C++、Java、Python、JavaScript、HTML、CSS、XML、YAML、.gitignore、.properties、.ini
- 行号显示
- 查找/替换 (Ctrl+F)
- 文件编码切换 (UTF-8 / UTF-8-BOM / GBK)
- Tab 行为配置（插入制表符或自定义空格数）
- 智能缩进、快捷键操作（复制/剪切整行、复制行、Ctrl+滚轮缩放）
- 完全本地运行，无网络访问

## 构建要求

- Qt 6.11.0 或更高版本（mingw_64）
- CMake 3.23+
- MinGW 13.1.0（或其他兼容 C++17 的编译器）

## 构建步骤

`ash
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build . --config Release
`

## 许可证

本软件 (LiteText) 的源代码采用 Apache License 2.0 许可（详见 [LICENSE](LICENSE)）。  
本软件动态链接了 Qt 框架（LGPLv3 许可），Qt 的 LGPLv3 许可证全文见 [LGPL-3.0.txt](LGPL-3.0.txt)。  
根据 LGPLv3 的要求，您有权修改 Qt 库并重新链接本软件，Qt 库的源代码可通过 [https://download.qt.io/](https://download.qt.io/) 获取。

## 隐私政策

本软件不收集任何用户数据。详见 [PRIVACY.md](PRIVACY.md)。

---

Momster
