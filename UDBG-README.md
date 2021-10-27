
此分支用于维护udbg所依赖的lqt模块，改进点
- 对 lua5.4 和 qt 5.14 进行了适配
- 保证`lqtL_unregister`函数在主线程中运行，因为udbg的lua是多线程环境，lqt的对象在gc时会调用lqtL_unregister函数，如果gc发生在其他线程中，会导致抢占主线程的lua_State
- 增强了lua54版本下对QVariant、QGenericArgument的转换
- 增加了辅助方法：`QMetaMethod::invoke2`，相比原来的invoke，增加了转换返回值的功能
- 使用[xmake](https://xmake.io/)作为构建系统

English:
this branch will be used to maintain the lqt relied by udbg
- adapt to lua5.4 and qt 5.14
- ensure the function `lqtL_unregister` to be executed in main thread
- improve the convertion of `QVariant` and `QGenericArgument` in lua54
- add method `QMetaMethod::invoke2`, compared to the original `invoke`, it can convert the returned value to lua
- use [xmake](https://xmake.io/) as the build system