/*
@author: LXP
@create time: 2024-4-30
@git repo: https://github.com/luoxpan/LP_STL
@主要参考: <STL源码剖析>侯捷 著 华中科技大学出版社 出版
*/
#ifndef LP_ITERATOR_H
#define LP_ITERATOR_H
#include <cstddef>
namespace lp
{
    // region: 基础iterator类,自定义的迭代器类应继承该类
    template <
        class Category,             // 迭代器类型
        class T,                    // 元素类型
        class Distance = ptrdiff_t, // 距离类型
        class Pointer = T *,        // 指针类型
        class Reference = T &>      // 引用类型
    struct iterator
    {
        using iterator_category = Category;
        using value_type = T;
        using difference_type = Distance;
        using pointer = Pointer;
        using reference = Reference;
    };
    // endregion 基础iterator类

    // region: 常用迭代器类型tag
    //  五种迭代器类型tag,用于区分不同类型的迭代器
    //  只是作为tag,没有具体的实现
    struct input_iterator_tag
    {
        /*输入迭代器
         * 只可读
         * 只支持ite++操作
         */
    };

    struct output_iterator_tag
    {
        /*输出迭代器
         * 只可写
         * 只支持ite++操作
         */
    };
    struct forward_iterator_tag : public input_iterator_tag
    {
        /* 前向迭代器
         * 可读可写
         * 只支持++操作
         */
    };

    struct bidirectional_iterator_tag : public forward_iterator_tag
    {
        /*双向迭代器
         * 可读可写,可反向遍历
         * 支持++和--操作
         */
    };
    struct random_access_iterator_tag : public bidirectional_iterator_tag
    {
        /*随机访问迭代器
         * 可读可写,可随机访问
         * 支持所有指针算术能力,包括++、--,p+n,p-n,p1-p2,p1<p2
         */
    };
    // endregion 常用迭代器类型tag
    //  region:萃取器,用于获取迭代器的相关信息
    template <class Iterator>
    struct iterator_traits
    {
        using iterator_category = typename Iterator::iterator_category;
        using value_type = typename Iterator::value_type;
        using difference_type = typename Iterator::difference_type;
        using pointer = typename Iterator::pointer;
        using reference = typename Iterator::reference;
    };

    // 针对原生指针而设计的 traits 偏特化
    template <class T>
    struct iterator_traits<T *>
    {
        using iterator_category = random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = T *;
        using reference = T &;
    };

    // 针对常量指针而设计的 traits 偏特化
    template <class T>
    struct iterator_traits<const T *>
    {
        using iterator_category = random_access_iterator_tag;
        using value_type = T;
        using difference_type = ptrdiff_t;
        using pointer = const T *;
        using reference = const T &;
    };
    // endregion 萃取器
    // 定义一些函数用来进行萃取
    // region:萃取获得迭代器的类型(category)
    template <class Iterator>
    inline typename iterator_traits<Iterator>::iterator_category
    iterator_category(const Iterator &)
    {
        using category = typename iterator_traits<Iterator>::iterator_category;
        return category();
    }
    // 萃取获得迭代器的元素类型
    // template <class Iterator>
    // inline typename iterator_traits<Iterator>::value_type *
    // value_type(const Iterator &)
    // {
    //     return static_cast<typename iterator_traits<Iterator>::value_type *>(0);
    // }

    // // 萃取获得迭代器的距离类型
    // template <class Iterator>
    // inline typename iterator_traits<Iterator>::difference_type *
    // distance_type(const Iterator &)
    // {
    //     return static_cast<typename iterator_traits<Iterator>::difference_type *>(0);
    // }
    /*
    value_type()萃取一个可能非常复杂的类型信息而使用空指针的技巧
    value_type() 函数的设计主要是为了在编译时提供关于迭代器所指向元素的类型信息,不会在运行时进行任何计算
    iterator_category()萃取一个轻量级的类别信息,可以直接返回一个类型标签实例,
    因为iterator_category是一个空的类标签,一般请款,编译器会优化掉这个临时对象的创建,从而使其开销最小化
    */
    // endregion 萃取获得迭代器的类型(category)
    // region: 计算迭代器之间的距离
    template <class InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    _distance(InputIterator first, InputIterator last, input_iterator_tag)
    {
        typename iterator_traits<InputIterator>::difference_type n = 0;
        while (first != last)
        {
            ++first;
            ++n;
        }
        return n;
    }

    template <class RandomAccessIterator>
    inline typename iterator_traits<RandomAccessIterator>::difference_type
    _distance(RandomAccessIterator first, RandomAccessIterator last, random_access_iterator_tag)
    {
        return last - first;
    }

    template <class InputIterator>
    inline typename iterator_traits<InputIterator>::difference_type
    distance(InputIterator first, InputIterator last)
    {
        return _distance(first, last, iterator_category(first));
    }
    // endregion 计算迭代器之间的距离
    // region:advance()函数用于移动迭代器
    // 对于input_iterator_tag,只支持ite++操作
    template <class InputIterator, class Distance>
    inline void _advance(InputIterator &it, Distance n, input_iterator_tag)
    {
        while (n--)
            ++it;
    }

    // 对于bidirectional_iterator_tag,支持++和--操作
    template <class BidirectionalIterator, class Distance>
    inline void _advance(BidirectionalIterator &it, Distance n, bidirectional_iterator_tag)
    {
        if (n >= 0)
        {
            while (n--)
                ++it;
        }
        else
        {
            while (n++)
                --it;
        }
    }

    // 对于random_access_iterator_tag,支持指针算术能力(随机移动)
    template <class RandomAccessIterator, class Distance>
    inline void _advance(RandomAccessIterator &it, Distance n, random_access_iterator_tag)
    {
        it += n;
    }

    template <class InputIterator, class Distance>
    inline void advance(InputIterator &it, Distance n)
    {
        _advance(it, n, iterator_category(it));
    }
    // endregion advance()函数用于移动迭代器
}
#endif // LP_ITERATOR_H