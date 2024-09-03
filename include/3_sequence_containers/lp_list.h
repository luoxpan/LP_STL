/*
@author: LXP
@create time: 2024-5-2
@git repo: https://github.com/luoxpan/LP_STL
@主要参考: <STL源码剖析>侯捷 著 华中科技大学出版社 出版
*/
#ifndef LP_LIST_H
#define LP_LIST_H
#include <cstddef>
#include "../1_allocator/lp_memory.h"
#include "../2_iterator/lp_iterator.h"
// list是一个双向链表
namespace lp
{
    // 节点
    template <class T>
    struct _list_node
    {
        using pointer_type = _list_node<T> *;
        pointer_type prev;
        pointer_type next;
        T data;
    };

    // 迭代器
    template <class T, class Ref, class Ptr>
    struct _list_iterator : public lp::iterator<T, Ref, Ptr, lp::bidirectional_iterator_tag>
    {
        using iterator = _list_iterator<T, T &, T *>;
        using const_iterator = _list_iterator<T, const T &, const T *>;
        using self_type = _list_iterator<T, Ref, Ptr>;

        using size_type = size_t;
        /*
        value_type,pointer,reference,difference_type,iterator_category从lp::iterator继承了
        也可以在实现一遍,增加可读性,也许能优化性能
        using value_type = T;
        using pointer = Ptr;
        using reference = Ref;
        using difference_type = ptrdiff_t;
        using iterator_category = bidirectional_iterator_tag;
        */

        using node_type = _list_node<T>;
        node_type *node;

        _list_iterator() {}
        _list_iterator(node_type *x) : node(x) {}
        _list_iterator(const iterator &x) : node(x.node) {}

        bool operator==(const self_type &x) const { return node == x.node; }
        bool operator!=(const self_type &x) const { return node != x.node; }

        reference operator*() const { return (*node).data; }
        pointer operator->() const { return &(operator*()); }
        // ++ite;返回的还是this,只是this里的node变了,所以可以返回引用
        self_type &operator++()
        {
            node = (node_type *)((*node).next);
            return *this;
        }
        // ite++;返回的是一个临时创建的self_type,所以只能返回值,不能返回引用
        self_type operator++(int)
        {
            self_type tmp = *this;
            ++*this;
            return tmp;
        }
        // --ite;
        self_type &operator--()
        {
            node = (link_type)((*node).prev);
            return *this;
        }
        // ite--;
        self_type operator--(int)
        {
            self_type tmp = *this;
            --*this;
            return tmp;
        }
    };

    // 正式的List,一个环状双向链表
    template <class T, class Alloc = lp::alloc>
    class list
    {
    protected:
        using list_node = _list_node<T>;

    public:
        using link_type = list_node *;
        using iterator = _list_iterator<T>;

    protected:
        // 用一个node就能表现完整链表,只需让其指向链表的末尾的一个空白节点
        link_type node;

    public:
        iterator begin() { return (link_type)((*node).next); }
        iterator end() { return node; }
    };
};
#endif // LP_LIST_H