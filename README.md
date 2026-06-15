# 无限GUI DLL  
> A dynamic-link library for injecting HUD overlays into Java Edition Minecraft  
> 一个用于向 Java 版 Minecraft 注入信息显示层（HUD）的动态链接库

---

## ⚠ Disclaimer | 免责声明

**本程序不会读取、修改或写入任何 Minecraft 游戏内存数据。  
不会提供或造成任何不公平的游戏优势，也不涉及任何加速、透视、自动操作等功能。  
InfiniteGui 仅用于绘制独立的叠加界面（Overlay），与游戏逻辑无任何交互。**

另外需要特别说明：

**⚠ 请勿在网易版 Minecraft（网易MC）中使用本程序。**  
由于网易版存在独立的客户端保护机制，**DLL 注入行为可能会被错误识别为作弊行为，并导致封号风险。**  
如果用户在网易版 Minecraft 中注入本程序并导致账号封禁，作者概不负责。  

本项目使用了《**阿里巴巴普惠体**》，是一款由中国企业首次发布的可面向全场景使用的免费商用正文字体。

---

## 📌 Overview | 项目简介

**无限GUI** 是一个可以注入到 **大部分 Java 版 Minecraft** 的动态链接库（DLL）。  
注入成功后，你可以在游戏画面中显示各种实时信息，包括：

- 自定义文本  
- 时间  
- 粉丝数  
- 计数器  
- 游戏 FPS  
- B 站直播弹幕
- 按键显示
- CPS显示
  
适用于 **主播、视频作者、服务器玩家、工具开发者** 等希望在游戏中加入 HUD 信息显示的人群。

---

## ✨ Features | 功能特点

### ✔ 基础功能
- 游戏内文字绘制  
- 游戏帧率（FPS）显示  
- 自定义信息编辑  
- 时间显示  
- 计数器
- 动态模糊

### ✔ 直播相关功能
- B 站粉丝数显示  
- B 站直播间弹幕实时显示（需要配合第三方弹幕姬使用：https://www.danmuji.org )

### ✔ 技术特性
- 支持多版本 Java Minecraft-1.13~最新版本
- 自定义渲染  
- 独立 DLL，可用于二次开发

---

## 📦 Installation | 安装方式

### 1. 下载成品
你可以克隆代码并自行构建  

你也可以在 **爱发电** 以 8.8 元获得：  
- 编译好的成品
- 附赠注入器
- 自动更新  
- 技术支持  
- 使用教程  

（付费内容是“服务与编译成品”，不是源码）

---

## 🛠 Build | 构建教程（源码编译）
> 本代码仓库未包含 OpenGL (GLEW)、stb 和 nlohmann/json 依赖库 需要用户自行引入。

### 📦 依赖库说明

本项目依赖以下第三方库：  

- OpenGL / GLEW  
- nlohmann/json（仅头文件库）
- stb

### ✅ 依赖安装方式一（推荐）：使用 vcpkg  

#### 1️⃣ 安装 vcpkg（如果你还没装）
- git clone https://github.com/microsoft/vcpkg  
cd vcpkg  
./bootstrap-vcpkg.bat  

#### 2️⃣ 使用 vcpkg 安装依赖
- ./vcpkg install glew:x64-windows  
./vcpkg install nlohmann-json  
./vcpkg install stb

#### 3️⃣ 将 vcpkg 集成到 Visual Studio
- ./vcpkg integrate install

### ✅ 依赖安装方式二：手动下载并加入项目
#### 🔹 nlohmann/json（头文件库）  

前往官方仓库下载源码  
- https://github.com/nlohmann/json  

将 single_include/nlohmann/json.hpp  
放入项目的 include/ 或任意你设置的包含目录中  

#### 🔹 GLEW（OpenGL 扩展库）

前往官网下载 GLEW  
- http://glew.sourceforge.net/  

解压后：  

- 将 include/GL/glew.h 放入包含目录  
将 lib 目录加入 链接器 → 附加库目录  

在链接器中添加：  

- glew32.lib  
opengl32.lib

#### 🔹 stb
- https://github.com/nothings/stb

### 🧩 编译环境建议
- Visual Studio 2022  
平台工具集：MSVC v143  
编译模式：Release | x64  
C++ 标准：C++17 或更高  

### ⚠️ 注入教程
编译后会生成以下文件：
- InfiniteGUI-DLL.dll
- glew32.dll  

请将生成**glew32.dll**放入**C:\Windows\System32**中。
使用注入程序，如[CheatEngine](https://www.cheatengine.org/ "CheatEngine官方网站")，将**InfiniteGUI-DLL.dll**注入到Minecraft中即可使用。


