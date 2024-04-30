/*
负责内存空间的配置与释放
*/
#ifndef LP_ALLOC_H
#define LP_ALLOC_H

#include <new>      //for placement new
#include <cstddef>  //for ptrdiff_t,size_t
#include <cstdlib>  //for exit
#include <iostream> //for std::cerr
/*
ptrdiff_t: 指针差值类型，即两个指针相减的结果类型
size_t: 无符号整数类型，size_t的大小和系统有关,32位系统就是32,64位系统就是64
*/
/*
 * 不考虑多线程
 * 内存不足的情况直接强制退出
 */
namespace lp
{
    /*###################################simple_alloc begin#############################################*/
    template <class T, class Alloc>
    class simple_alloc
    {
    public:
        static T *allocate(size_t n)
        {
            return 0 == n ? 0 : (T *)Alloc::allocate(n * sizeof(T));
        }
        static T *allocate(void)
        {
            return (T *)Alloc::allocate(sizeof(T));
        }
        static void deallocate(T *p, size_t n)
        {
            if (0 != n)
            {
                Alloc::deallocate(p, n * sizeof(T));
            }
        }
        static void deallocate(T *p)
        {
            Alloc::deallocate(p, sizeof(T));
        }
    };
    /* simple_alloc的使用示例
    template <class T, class Alloc = alloc>
    class vector
    {
        using data_alloctor=simple_alloc<T,Alloc>;
        data_alloctor::allocate(n);
    };
    */
    /*###################################simple_alloc end#############################################*/

    /*###################################_malloc_alloc_template begin#############################################*/

    // region:一级配置器
    /*
     * 一级配置器使用malloc,realloc,free进行内存的分配和释放
     * 采用强行退出的方式解决内存不足(分配失败)的问题
     */
    template <int inst>
    class _malloc_alloc_template
    {
    private:
        // oom:out of memory
        // 这里只提供最简单的处理办法,即内存分配失败就直接exit()
        static void *oom_malloc(size_t);
        static void *oom_realloc(void *, size_t);
        // static void (*_malloc_alloc_oom_handler)();

    public:
        static void *allocate(size_t n)
        {
            // 一级配置器直接使用malloc
            void *result = malloc(n);
            if (0 == result)
            {
                result = oom_malloc(n);
            }
            return result;
        }

        static void deallocate(void *p, size_t n)
        {
            // 一级配置器直接使用free
            free(p);
        }

        static void *reallocate(void *p, size_t old_size, size_t new_size)
        {
            void *result = realloc(p, new_size);
            if (0 == result)
            {
                result = oom_realloc(p, new_size);
            }
            return result;
        }
    };

    template <int inst>
    void *_malloc_alloc_template<inst>::oom_malloc(size_t n)
    {
        std::cerr << "out of memory" << std::endl;
        exit(1);
    }

    template <int inst>
    void *_malloc_alloc_template<inst>::oom_realloc(void *p, size_t n)
    {
        std::cerr << "out of memory" << std::endl;
        exit(1);
    }

    // 注意,以下直接将参数inst指定为0
    using malloc_alloc = _malloc_alloc_template<0>;
    // endregion:一级配置器
    /*###################################_malloc_alloc_template end#############################################*/

    /*###################################_default_alloc_template begin#############################################*/
    // region:二级配置器
    /*
     * 区块大于128 bytes时,移交一级配置器
     * 区块小于128 bytes时,用内存池进行管理
     * 内存池管理办法:每次取一块大内存,维护对应的free_list,
     *  有内存需求时从free_list中找内存满足,释放内存时,加入到free_list中
     * 二级配置器会主动将任何小额区块的内存需求上调至8的倍数,并维护16个free_list
     */

    enum
    {
        _ALIGN = 8,                       // 每个区块都要调整为8的倍数
        _MAX_BYTES = 128,                 // 区块最大为128
        _NFREELISTS = _MAX_BYTES / _ALIGN // free_list的个数:16
    };
    // 注意,无"template型别参数",且第二参数完全没派上用场
    // 第一参数用于多线程环境下,我们不考虑多线程环境,但依然保留该参数
    template <bool threads, int inst>
    class _default_alloc_template
    {
    public:
        static void *allocate(size_t n);
        static void deallocate(void *p, size_t n);
        static void *reallocate(void *p, size_t old_size, size_t new_size);

    private:
        // free_list的节点使用union节省开销
        union obj
        {
            union obj *free_list_link;
            char client_data[1];
        };

        // 16个free_list
        static obj *volatile free_list[_NFREELISTS];

    private:
        // round_up将bytes调整为8的倍数
        static size_t round_up(size_t bytes)
        {
            return (((bytes) + _ALIGN - 1) & ~(_ALIGN - 1));
        }
        // 根据区块大小,决定使用第n好free_list,n从1开始算
        static size_t free_list_index(size_t bytes)
        {
            std::cerr << (((bytes) + _ALIGN - 1) / _ALIGN) - 1 << std::endl;
            return (((bytes) + _ALIGN - 1) / _ALIGN) - 1;
        }
        // 重新填充区块大小为n的内存池
        static void *refill(size_t n);
        // 配置一大块空间,可容纳n个特定大小的obf
        static char *chunk_alloc(size_t size, int &nobjs);
        // chunk alloc 中使用的状态参数
        static char *start_free; // 内存池起始位置,只在chunk_alloc中改变
        static char *end_free;   // 内存池结束位置,只在chunk_alloc中改变

        static size_t heap_size;
    };
    // static参数初值设定
    template <bool threads, int inst>
    char *_default_alloc_template<threads, inst>::start_free = 0;

    template <bool threads, int inst>
    char *_default_alloc_template<threads, inst>::end_free = 0;

    template <bool threads, int inst>
    size_t _default_alloc_template<threads, inst>::heap_size = 0;

    template <bool threads, int inst>
    typename _default_alloc_template<threads, inst>::obj *volatile _default_alloc_template<threads, inst>::free_list[_NFREELISTS] =
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    // allocate,reallocate,deallocate的具体实现
    template <bool threads, int inst>
    void *_default_alloc_template<threads, inst>::allocate(size_t n)
    {
        obj *volatile *my_free_list;
        obj *result;
        if (n > _MAX_BYTES)
        {
            return (malloc_alloc::allocate(n));
        }
        my_free_list = free_list + free_list_index(n);
        result = *my_free_list;
        if (0 == result)
        {
            void *r = refill(round_up(n));
            return r;
        }
        *my_free_list = result->free_list_link;
        return result;
    }

    template <bool threads, int inst>
    void _default_alloc_template<threads, inst>::deallocate(void *p, size_t n)
    {
        obj *q = (obj *)p;
        obj *volatile *my_free_list;
        if (n > _MAX_BYTES)
        {
            malloc_alloc::deallocate(p, n);
            return;
        }
        my_free_list = free_list + free_list_index(n);
        q->free_list_link = *my_free_list;
        *my_free_list = q;
    }

    template <bool threads, int inst>
    void *_default_alloc_template<threads, inst>::reallocate(void *p, size_t old_size, size_t new_size)
    {
        void *result;
        size_t copyz;

        if (old_size > (size_t)_MAX_BYTES && new_size > (size_t)_MAX_BYTES)
        {
            return (realloc(p, new_size));
        }
        if (round_up(old_size) == round_up(new_size))
            return (p);
        result = allocate(new_size);
        copyz = new_size > old_size ? old_size : new_size;
        memcpy(result, p, copyz);
        deallocate(p, old_size);
        return (result);
    }

    // 默认内存分配器的实现
    template <bool threads, int inst>
    void *_default_alloc_template<threads, inst>::refill(size_t n)
    {
        int nobjs = 20; // 默认尝试获取20个对象
        // 注意nobjs以引用方式传递
        char *chunk = chunk_alloc(n, nobjs); // 从内存池申请内存，以填充free list

        // 如果只获得了一个对象，则直接返回给客户程序
        if (1 == nobjs)
        {
            return chunk;
        }

        // 下面的代码将剩余的对象链接起来，供后续分配使用

        // my_free_list指向我们的空闲链表
        obj *volatile *my_free_list;
        obj *result;
        obj *current_obj, *next_obj;

        // my_free_list指向相应的空闲链表桶
        my_free_list = free_list + free_list_index(n);
        // 返回的块是第一个对象
        result = (obj *)chunk;

        // 从chunk之后的内存开始进行链表构造
        *my_free_list = next_obj = (obj *)(chunk + n);
        // 将free_list的各节点串接起来
        for (int i = 1;; i++)
        {
            current_obj = next_obj;
            next_obj = (obj *)((char *)next_obj + n);
            // 当链表中仅剩最后一个对象时，终止链接
            if (nobjs - 1 == i)
            {
                current_obj->free_list_link = 0;
                break;
            }
            else
            {
                // 否则将对象链接到free list中
                current_obj->free_list_link = next_obj;
            }
        }
        return result; // 返回给客户端的第一个对象
    }

    // 给内存池申请内存
    template <bool threads, int inst>
    char *_default_alloc_template<threads, inst>::chunk_alloc(size_t size, int &nobjs)
    {
        char *result;
        size_t total_bytes = size * nobjs;         // 计算需要申请的总字节
        size_t bytes_left = end_free - start_free; // 计算内存池剩余字节

        // 如果内存池剩余空间完全满足需求，则直接分配
        if (bytes_left >= total_bytes)
        {
            result = start_free;
            start_free += total_bytes;
            return (result);
        }
        else if (bytes_left >= size)
        {
            // 如果内存池剩余空间不能满足全部需求，但至少能满足一个对象的需求
            nobjs = (int)(bytes_left / size);
            total_bytes = size * nobjs;
            result = start_free;
            start_free += total_bytes;
            return (result);
        }
        else
        {
            // 内存池空间不足，需要从系统堆上申请
            size_t bytes_to_get = 2 * total_bytes + round_up(heap_size >> 4);

            // 尝试将内存池中剩余的部分也加入到对应的free list中
            if (bytes_left > 0)
            {
                obj *volatile *my_free_list = free_list + free_list_index(bytes_left);

                ((obj *)start_free)->free_list_link = *my_free_list;
                *my_free_list = (obj *)start_free;
            }

            // 从堆上申请内存
            start_free = (char *)malloc(bytes_to_get);
            if (0 == start_free)
            {
                // 处理堆内存申请失败
                // 尝试利用我们手上的东西（较大的空闲块）来满足需求
                size_t i;
                obj *volatile *my_free_list;
                obj *p;
                for (i = size; i <= (size_t)_MAX_BYTES; i += (size_t)_ALIGN)
                {
                    my_free_list = free_list + free_list_index(i);
                    p = *my_free_list;
                    if (0 != p)
                    {
                        // 调整free list以释放出未使用的内存块
                        *my_free_list = p->free_list_link;
                        start_free = (char *)p;
                        end_free = start_free + i;
                        // 递归调用自己，以再次尝试满足内存申请的需求
                        return (chunk_alloc(size, nobjs));
                    }
                }
                end_free = 0; // 没有内存可以用了

                // 调用一级配置器，异常情况下的处理
                start_free = (char *)malloc_alloc::allocate(bytes_to_get);
            }

            // 成功从堆上获取了内存，更新内存池
            heap_size += bytes_to_get;
            end_free = start_free + bytes_to_get;

            // 递归调用自己，以再次尝试满足申请
            return (chunk_alloc(size, nobjs));
        }
    }

    // 重载运算符==
    template <bool threads, int inst>
    inline bool operator==(const _default_alloc_template<threads, inst> &,
                           const _default_alloc_template<threads, inst> &)
    {
        return true;
    }

    // 类型别名
    using alloc = _default_alloc_template<false, 0>;
    // endregion:二级配置器
    /*###################################_default_alloc_template end#############################################*/

};

#endif // LP_ALLOCATOR_H