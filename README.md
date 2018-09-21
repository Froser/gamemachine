# GameMachine

构建方法：
构建Coding下的CMakeLists.txt

Windows:
直接构建release，并将media/gm.pk0拷贝到release目录下，可以运行程序。
如果运行debug，请将media/gm.pk0用WinRAR等压缩工具解压到D:/gmpk。

对于没有安装DirectX11的环境的机器，可以将CMake中的GM_USE_DX11开关关闭，这样将编译没有DirectX11的渲染环境。

ATTENTION:
只能使用VS2015及以上的版本编译

Linux: (GCC)
sudo apt-get install build-essential
sudo apt-get install libgl1-mesa-dev
sudo apt-get install libglu-dev
sudo apt-get install language-pack-zh-hans