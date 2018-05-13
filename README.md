# GameMachine

构建方法：
构建Coding下的CMakeLists.txt
直接构建release，并将media/gm.pk0拷贝到release目录下，可以运行程序。
如果运行debug，请将media/gm.pk0用WinRAR等压缩工具解压到D:/gmpk。

对于没有安装DirectX11的环境的机器，可以将CMake中的GM_USE_DX11开关关闭，这样将编译没有DirectX11的渲染环境。

ATTENTION:
目前只能在Windows平台通过编译，其他平台暂时未支持；
只能使用VS2015及以上的版本编译