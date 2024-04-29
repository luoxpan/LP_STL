#ifndef LP_CONSTRUCT_H
#define LP_CONSTRUCT_H
/*
负责对象内容的构造和析构
定义有两个基本函数:
* construct() 构造函数
* destroy() 析构函数
*/
#include <new> //for placement new
#include <type_traits>
namespace lp
{
    template <class T1, class T2>
    inline void construct(T1 *p, const T2 &value)
    {
        // placement new的用法:
        // 不分配内存,而是在已经分配好的内存地址p上构造一个对象
        new (p) T1(value);
    }

    // region:destory的第一版本,接受一个指针
    template <class T>
    inline void destory(T *p)
    {
        p->~T(); // 调用T的析构函数
    }
    // endregion:destory的第一版本

    // region:destory的第二版本,接受两个迭代器
    // 先要判断元素的类型是否为平凡的析构函数
    // 主要是为了提高性能,所谓的平凡的析构函数就是没有任何操作的析构函数,即没有任何代码,
    // 例如:
    // struct A {
    //     ~A() {}
    // };
    // 这种析构函数就是平凡的,它的作用就是什么都不做,因此可以直接跳过,不用调用
    // 对于平凡的析构函数,可以直接跳过,不用调用析构函数,这样可以提高性能
    template <class ForwardIterator, class T>
    inline void destroy(ForwardIterator first, ForwardIterator last)
    {
        destroy_aux(first, last, std::is_trivially_destructible<T>{});
    }

    template <class ForwardIterator, class T>
    inline void destroy_aux(ForwardIterator first, ForwardIterator last, std::false_type)
    {
        for (; first < last; ++first)
        {
            destroy(&*first);
        }
    }

    template <class ForwardIterator, class T>
    inline void destroy_aux(ForwardIterator first, ForwardIterator last, std::true_type)
    {
    }
    // endregion:destory的第二版本

    // region:destory的第三版本,对于char*和wchar_t*的特化版本
    // 第三版本其实可以没有的,因为char*和wchar_t*的析构函数什么都不做,所以将其特化出来,提高性能
    // 注意这里使用两个参数,因为对char*进行析构一般是对一个范围进行的
    inline void destroy(char *first, char *last)
    {
        // 对于char*和wchar_t*的特化版本,什么都不干
    }

    inline void destroy(wchar_t *first, wchar_t *last)
    {
        // 对于char*和wchar_t*的特化版本,什么都不干
    }
    // endregion
} // namespace lp
#endif // LP_CONSTRUCT_H