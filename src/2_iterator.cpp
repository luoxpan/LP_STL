#include <iostream>

template <class T>
struct MyIter
{
    typedef T value_type;
    T *ptr;
    MyIter(T *p = 0) : ptr(p) {}
    T &operator*() const { return *ptr; }
};

// template <class I>
// typename I::value_type func(I ite)
// {
//     return *ite;
// }

// 约定:
/*
所以的iterator都要有一个内嵌型别声明value_type
::value_type
::difference_type 差值类型,ite1-ite2
::pointer
::reference
::iterator_category
###########介绍
::value_type
作用：::value_type 代表了迭代器指向的元素的类型。
例子：如果你有一个 std::vector<int>::iterator 类型的迭代器，那么其 ::value_type 就是 int。
::difference_type
作用：::difference_type 代表了两个迭代器之间的距离类型。它是一个有符号整数类型，用于表示两个迭代器之间的元素个数差异。
例子：对于任意类型为 std::vector<int>::iterator 的迭代器 it1 和 it2，it2 - it1 的类型是 std::vector<int>::difference_type。
::pointer
作用：::pointer 代表了指向迭代器所指元素的原生指针类型。
例子：对于 std::vector<int>::iterator 类型的迭代器，其 ::pointer 就是 int*。
::reference
作用：::reference 代表了迭代器所指元素的引用类型。
例子：如果你有一个 std::vector<int>::iterator 类型的迭代器，那么通过它访问元素得到的是 int& 类型的引用，这就是其 ::reference。
::iterator_category
作用：::iterator_category 代表了迭代器类型的分类。C++ STL中的迭代器分为五种类别：输入迭代器（Input Iterator）、输出迭代器（Output Iterator）、前向迭代器（Forward Iterator）、双向迭代器（Bidirectional Iterator）和随机访问迭代器（Random Access Iterator）。这个特征标识了迭代器所支持的操作类型。
例子：对于 std::vector<int>::iterator 类型的迭代器，其 ::iterator_category 是 std::random_access_iterator_tag，表示它是一个支持随机访问的迭代器。
*/
// 加个中间层
template <class I>
struct iterator_traits
{
    typedef typename I::value_type value_type;
};

template <class I>
typename iterator_traits<I>::value_type func(I ite)
{
    return *ite;
}

template <class T>
struct iterator_traits<T *>
{
    typedef T value_type;
};

int main()
{
    MyIter<int> ite(new int(8));
    std::cout << func(ite);
    int *p = new int(8);
    std::cout << func(p);
    return 0;
}