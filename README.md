# GameMachine 介绍：

## 构建方法：
安装3.9.0以上版本的CMake，并构建Coding下的CMakeLists.txt。

### Windows下的构建方法:
* 直接构建release，并将media/gm.pk0拷贝到release目录下，可以运行程序。
* 如果运行debug，请将media/gm.pk0用WinRAR等压缩工具解压到D:/gmpk。
* 对于没有安装DirectX11的环境的机器，可以将CMake中的GM_USE_DX11开关关闭，这样将编译没有DirectX11的渲染环境。

**由于用到了诸多C++11/14的特性，只能使用VS2015及以上的版本编译**

### Linux下使用GCC构建方法：
你需要安装一下依赖：
* sudo apt-get install build-essential
* sudo apt-get install libgl1-mesa-dev
* sudo apt-get install libglu-dev
* sudo apt-get install language-pack-zh-hans
当安装完以上依赖后，使用CMake构建，即可完成编译。

## GameMachine提供的功能：
1. 底层功能：
  1. 提供跨平台渲染环境。提供DirectX 11和OpenGL两种方式。
  2. 提供跨平台数学库。提供基于DirectX数学库和glm的数学库，支持SIMD。
  3. 提供跨平台的多种设施，如窗口创建、消息循环、内存池、渲染文字等。
  4. 提供资产管理器。
  5. 提供平台无关渲染流水线。
  6. 提供部分反射机制。
  7. 提供Lua类的封装，可以轻松对接Lua。
  8. 提供可扩展接口，可以之后支持Vulkan及其它渲染引擎。
  9. 提供可扩展渲染，可以加入自己的渲染过程，实现自定义渲染。
  10. 提供键盘、鼠标、手柄消息封装等。
  11. 声音播放封装。
  12. UI控件及完备的事件机制。
  13. 其它功能。

2. 集成的渲染功能：
  1. 基本渲染，图元、纹理绘制（顶点绘制，索引绘制）。
  2. 法线贴图、高光贴图。
  3. Phong光照模型，点光源，方向光，聚光灯。
  4. PBR渲染。
  5. 读取模型，如obj，md5等。
  6. 阴影贴图，CSM。
  7. 物理碰撞。
  8. 粒子系统。
  9. Billboard效果
  10. 延迟渲染。
  11. 骨骼动画。
  12. UI控件。
  13. 文字渲染。
  14. 抗锯齿渲染。
  15. 其它功能。
  ** 每一个渲染功能，GameMachine都提供了一个详细的demo来演示。 **

## 如何学习、改进GameMachine
  因本人能力有限，无法做到尽善尽美，欢迎大家一起来提升和改进GameMachine。以下是GameMachine文件夹下的目录结构：
  * extensions: 扩展功能。扩展功能是没有放在GameMachine主要功能中的一些功能模块。目前只实现了雷神之锤3的BSP文件的读取。并且，demo中还实现了雷神之锤3的场景的渲染和人物的移动。你可以用手柄或者键盘来移动玩家。
  * foundation: 基本框架，如主程序流程、工具类、字符串类、基本定义等。
  * gmdata: 数据相关类，如解析图片、解析模型、模型数据结构等。
  * gmengine: 渲染流水线的实现。它目前具体由gmgl和gmdx11实现。
  * gmdx11: DirectX 11 相关渲染的实现。
  * gmgl: OpenGL 相关渲染实现。
  * gmlua: GameMachine对Lua的封装的相关实现。里面提供了Lua来操控GameMachine内部对象的很多方法。gamemachineluademo中有详细的demo。
  * gmphysics: GameMachine物理引擎的实现。主要是封装了bullet3。
