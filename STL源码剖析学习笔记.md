# STL 源码剖析学习笔记

## allocator
![picture 0](https://s2.loli.net/2024/09/06/tJTsdYnLjmr6kVi.png)  

SGI使用特殊的空间适配器`std::alloc`。SGI也有符合STL标准的`std::allocator`,但这只是对`new/delete`做了一层薄薄的封装，并没有任何效率上的强化。

1_allocator的阅读推荐。先看`lp_memory.h`,其内包含了`alloc.h,construct.h,uninitialized.h`。然后阅读`lp_construct.h`，然后是`lp_alloc.h`。`lp_uninitialized.h`中定义了一些全局函数，用来填充或复制大块内存数据。

* `lp_alloc.h`负责内存的配置和释放
* `lp_construct.h`负责对象内容的构造与析构
* `lp_uninitialized.h`中定义了一些全局函数，用来填充或复制大块内存数据。


### construct
![picture 1](https://s2.loli.net/2024/09/06/sUgdheIVPRCwlMA.png)  

`lp_construct.h`做的事情很简单，但为了提高效率，做了很多优化，导致看起来很复杂。其实`alloc.h`就是对对象的构造和析构，但针对对象的不同类型做了特化。

对于构造`construct`，直接使用`placement new`在已经分配好的内存上进行构造对象。

对于析构`destory`,如果对象是`char*,wchar*`，则什么都不干。如果对象是一个其他指针，则调用该指针指向对象的析构函数。对于其他的泛化指针区域，根据对象是否平凡，采用两种做法，如果是平凡的，则什么都不做；如果是非平凡的，则对区域内对象进行依次析构。

### alloc
![picture 2](https://s2.loli.net/2024/09/06/m8w2b1vgiznakPs.png)  

`alloc.h`中有一个`simple_alloc`是对alloc的封装，使其具备标准接口。

实现了一级配置器和二级配置器。一级配置器使用`malloc,free`申请和释放内存，使用强制退出处理分配失败情况。

二级配置器维护一个由`malloc`从堆空间配置获得的内存池，在内存池中维护16个自由链表，负责16个小型区块的次配置能力。

当需求区块大于138bytes时，调用一级配置器；当二级配置器的内存池内存不足时，调用一级配置器申请内存。

### uninitialize

![picture 3](https://s2.loli.net/2024/09/06/9HRJC4rgq5EflXI.png)  

实际上就是3个大块区域构造函数，用于**已经分配好内存**，但没有在内存上构造对象的内存块进行批量构造。

同`alloc.h`中的`destory`，根据构造对象进行优化。



