
#ifndef _SGI_STL_INTERNAL_LIST_H
#define _SGI_STL_INTERNAL_LIST_H
#include "../1_allocator/lp_memory.h"
#include "../2_iterator/lp_iterator.h"
namespace lp
{

    struct _List_node_base
    {
        _List_node_base *_M_next;
        _List_node_base *_M_prev;
    };

    template <class _Tp>
    struct _List_node : public _List_node_base
    {
        _Tp _M_data;
    };

    struct _List_iterator_base
    {
        // nested type 声明
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;
        typedef bidirectional_iterator_tag iterator_category;

        _List_node_base *_M_node; // 原生指针，指向节点

        _List_iterator_base(_List_node_base *_x) : _M_node(_x) {}
        _List_iterator_base() {}

        void _M_incr() { _M_node = _M_node->_M_next; }
        void _M_decr() { _M_node = _M_node->_M_prev; }

        bool operator==(const _List_iterator_base &_x) const
        {
            return _M_node == _x._M_node;
        }
        bool operator!=(const _List_iterator_base &_x) const
        {
            return _M_node != _x._M_node;
        }
    };

    template <class _Tp, class _Ref, class _Ptr>
    struct _List_iterator : public _List_iterator_base
    {
        typedef _List_iterator<_Tp, _Tp &, _Tp *> iterator;
        typedef _List_iterator<_Tp, const _Tp &, const _Tp *> const_iterator;
        typedef _List_iterator<_Tp, _Ref, _Ptr> _Self;

        typedef _Tp value_type;
        typedef _Ptr pointer;
        typedef _Ref reference;
        typedef _List_node<_Tp> _Node;

        _List_iterator(_Node *_x) : _List_iterator_base(_x) {}
        _List_iterator() {}
        _List_iterator(const iterator &_x) : _List_iterator_base(_x._M_node) {}

        // dereference，获取data
        reference operator*() const { return ((_Node *)_M_node)->_M_data; }

        pointer operator->() const { return &(operator*()); }

        _Self &operator++()
        {
            this->_M_incr();
            return *this;
        }
        // 前进一个节点， 后置返回值
        _Self operator++(int)
        {
            _Self _tmp = *this;
            this->_M_incr();
            return _tmp;
        }
        // 后退一个节点， 前置返回对象
        _Self &operator--()
        {
            this->_M_decr();
            return *this;
        }
        _Self operator--(int)
        {
            _Self _tmp = *this;
            this->_M_decr();
            return _tmp;
        }
    };

    inline bidirectional_iterator_tag
    iterator_category(const _List_iterator_base &)
    {
        return bidirectional_iterator_tag();
    }

    template <class _Tp, class _Ref, class _Ptr>
    inline _Tp *
    value_type(const _List_iterator<_Tp, _Ref, _Ptr> &)
    {
        return 0;
    }

    inline ptrdiff_t *
    distance_type(const _List_iterator_base &)
    {
        return 0;
    }

    template <class _Tp, class _Alloc>
    class _List_base
    {
    public:
        typedef _Alloc allocator_type;
        allocator_type get_allocator() const { return allocator_type(); }

        _List_base(const allocator_type &)
        {
            _M_node = _M_get_node();
            _M_node->_M_next = _M_node;
            _M_node->_M_prev = _M_node;
        }
        ~_List_base()
        {
            clear();
            _M_put_node(_M_node);
        }

        void clear();

    protected:
        typedef simple_alloc<_List_node<_Tp>, _Alloc> _Alloc_type;
        _List_node<_Tp> *_M_get_node() { return _Alloc_type::allocate(1); }
        void _M_put_node(_List_node<_Tp> *_p) { _Alloc_type::deallocate(_p, 1); }

    protected:
        _List_node<_Tp> *_M_node;
    };

    template <class _Tp, class _Alloc>
    void _List_base<_Tp, _Alloc>::clear()
    {
        _List_node<_Tp> *_cur = (_List_node<_Tp> *)_M_node->_M_next;
        while (_cur != _M_node)
        {
            _List_node<_Tp> *_tmp = _cur;
            _cur = (_List_node<_Tp> *)_cur->_M_next;
            _Destroy(&_tmp->_M_data);
            _M_put_node(_tmp);
        }
        _M_node->_M_next = _M_node;
        _M_node->_M_prev = _M_node;
    }

    template <class _Tp, class _Alloc = lp::alloc>
    class list : protected _List_base<_Tp, _Alloc>
    {
        // requirements:

        _STL_CLASS_REQUIRES(_Tp, _Assignable);

        typedef _List_base<_Tp, _Alloc> _Base;

    protected:
        typedef void *_Void_pointer;

    public:
        typedef _Tp value_type;
        typedef value_type *pointer;
        typedef const value_type *const_pointer;
        typedef value_type &reference;
        typedef const value_type &const_reference;
        typedef _List_node<_Tp> _Node;
        typedef size_t size_type;
        typedef ptrdiff_t difference_type;

        typedef typename _Base::allocator_type allocator_type;
        allocator_type get_allocator() const { return _Base::get_allocator(); }

    public:
        typedef _List_iterator<_Tp, _Tp &, _Tp *> iterator;
        typedef _List_iterator<_Tp, const _Tp &, const _Tp *> const_iterator;

        typedef reverse_bidirectional_iterator<const_iterator, value_type,
                                               const_reference, difference_type>
            const_reverse_iterator;
        typedef reverse_bidirectional_iterator<iterator, value_type, reference,
                                               difference_type>
            reverse_iterator;

    protected:
        _Node *_M_create_node(const _Tp &_x)
        {
            // 配置节点并返回
            _Node *_p = _M_get_node();
            _STL_TRY
            {
                _Construct(&_p->_M_data, _x);
            }
            // 释放节点
            _STL_UNWIND(_M_put_node(_p));
            return _p;
        }

        _Node *_M_create_node()
        {
            _Node *_p = _M_get_node();
            _STL_TRY
            {
                _Construct(&_p->_M_data);
            }
            _STL_UNWIND(_M_put_node(_p));
            return _p;
        }

    public:
        explicit list(const allocator_type &_a = allocator_type()) : _Base(_a) {}

        iterator begin() { return (_Node *)(_M_node->_M_next); }
        const_iterator begin() const { return (_Node *)(_M_node->_M_next); }

        iterator end() { return _M_node; }
        const_iterator end() const { return _M_node; }

        reverse_iterator rbegin()
        {
            return reverse_iterator(end());
        }
        const_reverse_iterator rbegin() const
        {
            return const_reverse_iterator(end());
        }

        reverse_iterator rend()
        {
            return reverse_iterator(begin());
        }
        const_reverse_iterator rend() const
        {
            return const_reverse_iterator(begin());
        }

        bool empty() const { return _M_node->_M_next == _M_node; }
        size_type size() const
        {
            size_type _result = 0;
            distance(begin(), end(), _result);
            return _result;
        }
        size_type max_size() const { return size_type(-1); }

        reference front() { return *begin(); }
        const_reference front() const { return *begin(); }
        reference back() { return *(--end()); }
        const_reference back() const { return *(--end()); }

        void swap(list<_Tp, _Alloc> &_x) { std:: ::swap(_M_node, _x._M_node); }

        // 在iterator所在位置插入值为x的节点
        iterator insert(iterator _position, const _Tp &_x)
        {
            _Node *_tmp = _M_create_node(_x);
            // 调整指针
            _tmp->_M_next = _position._M_node;
            _tmp->_M_prev = _position._M_node->_M_prev;
            _position._M_node->_M_prev->_M_next = _tmp;
            _position._M_node->_M_prev = _tmp;
            return _tmp;
        }
        iterator insert(iterator _position) { return insert(_position, _Tp()); }
        void insert(iterator _position, const _Tp *_first, const _Tp *_last);
        void insert(iterator _position,
                    const_iterator _first, const_iterator _last);

        void insert(iterator _pos, size_type _n, const _Tp &_x)
        {
            _M_fill_insert(_pos, _n, _x);
        }
        void _M_fill_insert(iterator _pos, size_type _n, const _Tp &_x);

        // 调用insert
        void push_front(const _Tp &_x) { insert(begin(), _x); }
        void push_front() { insert(begin()); }
        void push_back(const _Tp &_x) { insert(end(), _x); }
        void push_back() { insert(end()); }

        // 移除iterator所指节点并destroy
        iterator erase(iterator _position)
        {
            _List_node_base *_next_node = _position._M_node->_M_next;
            _List_node_base *_prev_node = _position._M_node->_M_prev;
            _Node *_n = (_Node *)_position._M_node;
            _prev_node->_M_next = _next_node;
            _next_node->_M_prev = _prev_node;
            _Destroy(&_n->_M_data);
            _M_put_node(_n);
            return iterator((_Node *)_next_node);
        }
        iterator erase(iterator _first, iterator _last);
        void clear() { _Base::clear(); }

        void resize(size_type _new_size, const _Tp &_x);
        void resize(size_type _new_size) { this->resize(_new_size, _Tp()); }

        // 调用erase
        void pop_front() { erase(begin()); }
        void pop_back()
        {
            iterator _tmp = end();
            erase(--_tmp);
        }
        list(size_type _n, const _Tp &_value,
             const allocator_type &_a = allocator_type())
            : _Base(_a)
        {
            insert(begin(), _n, _value);
        }
        explicit list(size_type _n)
            : _Base(allocator_type())
        {
            insert(begin(), _n, _Tp());
        }

        list(const _Tp *_first, const _Tp *_last,
             const allocator_type &_a = allocator_type())
            : _Base(_a)
        {
            this->insert(begin(), _first, _last);
        }
        list(const_iterator _first, const_iterator _last,
             const allocator_type &_a = allocator_type())
            : _Base(_a)
        {
            this->insert(begin(), _first, _last);
        }

        list(const list<_Tp, _Alloc> &_x) : _Base(_x.get_allocator())
        {
            insert(begin(), _x.begin(), _x.end());
        }

        ~list() {}

        list<_Tp, _Alloc> &operator=(const list<_Tp, _Alloc> &_x);

    public:
        // assign(), a generalized assignment member function.  Two
        // versions: one that takes a count, and one that takes a range.
        // The range version is a member template, so we dispatch on whether
        // or not the type is an integer.

        void assign(size_type _n, const _Tp &_val) { _M_fill_assign(_n, _val); }

        void _M_fill_assign(size_type _n, const _Tp &_val);

    protected:
        // 元素搬移 [first, last)搬移到position之前
        void transfer(iterator _position, iterator _first, iterator _last)
        {
            if (_position != _last)
            {
                // Remove [first, last) from its old position.
                _last._M_node->_M_prev->_M_next = _position._M_node;
                _first._M_node->_M_prev->_M_next = _last._M_node;
                _position._M_node->_M_prev->_M_next = _first._M_node;

                // Splice [first, last) into its new position.
                _List_node_base *_tmp = _position._M_node->_M_prev;
                _position._M_node->_M_prev = _last._M_node->_M_prev;
                _last._M_node->_M_prev = _first._M_node->_M_prev;
                _first._M_node->_M_prev = _tmp;
            }
        }

    public:
        // 将 x接合于 position所指位置之前。x必须不同于 *this。
        void splice(iterator _position, list &_x)
        {
            if (!_x.empty())
                this->transfer(_position, _x.begin(), _x.end());
        }
        // 将 i所指元素接合于 position所指位置之前。position和 i可指向同一个 list
        void splice(iterator _position, list &, iterator _i)
        {
            iterator _j = _i;
            ++_j;
            if (_position == _i || _position == _j)
                return;
            this->transfer(_position, _i, _j);
        }
        // 将 [first,last)内的所有元素接合于 position所指位置之前。
        //  position和[first,last)可指向同一个 list，
        // 但 position不能位于[first,last)之内。
        void splice(iterator _position, list &, iterator _first, iterator _last)
        {
            if (_first != _last)
                this->transfer(_position, _first, _last);
        }
        void remove(const _Tp &_value);
        void unique();
        void merge(list &_x);
        void reverse();
        void sort();
    };

    template <class _Tp, class _Alloc>
    inline bool
    operator==(const list<_Tp, _Alloc> &_x, const list<_Tp, _Alloc> &_y)
    {
        typedef typename list<_Tp, _Alloc>::const_iterator const_iterator;
        const_iterator _end1 = _x.end();
        const_iterator _end2 = _y.end();

        const_iterator _i1 = _x.begin();
        const_iterator _i2 = _y.begin();
        while (_i1 != _end1 && _i2 != _end2 && *_i1 == *_i2)
        {
            ++_i1;
            ++_i2;
        }
        return _i1 == _end1 && _i2 == _end2;
    }

    template <class _Tp, class _Alloc>
    inline bool operator<(const list<_Tp, _Alloc> &_x,
                          const list<_Tp, _Alloc> &_y)
    {
        return lexicographical_compare(_x.begin(), _x.end(),
                                       _y.begin(), _y.end());
    }

    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::insert(iterator _position,
                                   const _Tp *_first, const _Tp *_last)
    {
        for (; _first != _last; ++_first)
            insert(_position, *_first);
    }

    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::insert(iterator _position,
                                   const_iterator _first, const_iterator _last)
    {
        for (; _first != _last; ++_first)
            insert(_position, *_first);
    }

    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::_M_fill_insert(iterator _position,
                                           size_type _n, const _Tp &_x)
    {
        for (; _n > 0; --_n)
            insert(_position, _x);
    }

    template <class _Tp, class _Alloc>
    typename list<_Tp, _Alloc>::iterator list<_Tp, _Alloc>::erase(iterator _first,
                                                                  iterator _last)
    {
        while (_first != _last)
            erase(_first++);
        return _last;
    }

    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::resize(size_type _new_size, const _Tp &_x)
    {
        iterator _i = begin();
        size_type _len = 0;
        for (; _i != end() && _len < _new_size; ++_i, ++_len)
            ;
        if (_len == _new_size)
            erase(_i, end());
        else // _i == end()
            insert(end(), _new_size - _len, _x);
    }

    template <class _Tp, class _Alloc>
    list<_Tp, _Alloc> &list<_Tp, _Alloc>::operator=(const list<_Tp, _Alloc> &_x)
    {
        if (this != &_x)
        {
            iterator _first1 = begin();
            iterator _last1 = end();
            const_iterator _first2 = _x.begin();
            const_iterator _last2 = _x.end();
            while (_first1 != _last1 && _first2 != _last2)
                *_first1++ = *_first2++;
            if (_first2 == _last2)
                erase(_first1, _last1);
            else
                insert(_last1, _first2, _last2);
        }
        return *this;
    }

    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::_M_fill_assign(size_type _n, const _Tp &_val)
    {
        iterator _i = begin();
        for (; _i != end() && _n > 0; ++_i, --_n)
            *_i = _val;
        if (_n > 0)
            insert(end(), _n, _val);
        else
            erase(_i, end());
    }

    // 将数值为value的节点全部移除
    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::remove(const _Tp &_value)
    {
        iterator _first = begin();
        iterator _last = end();
        while (_first != _last)
        {
            iterator _next = _first;
            ++_next;
            if (*_first == _value)
                erase(_first);
            _first = _next;
        }
    }

    // 移除数值相同的连续元素，不连续不会移除
    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::unique()
    {
        iterator _first = begin();
        iterator _last = end();
        if (_first == _last)
            return;
        iterator _next = _first;
        while (++_next != _last)
        {
            if (*_first == *_next)
                erase(_next);
            else
                _first = _next;
            _next = _first;
        }
    }

    // 将x合并到*this，前提已排序
    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::merge(list<_Tp, _Alloc> &_x)
    {
        iterator _first1 = begin();
        iterator _last1 = end();
        iterator _first2 = _x.begin();
        iterator _last2 = _x.end();
        while (_first1 != _last1 && _first2 != _last2)
            if (*_first2 < *_first1)
            {
                iterator _next = _first2;
                transfer(_first1, _first2, ++_next);
                _first2 = _next;
            }
            else
                ++_first1;
        if (_first2 != _last2)
            transfer(_last1, _first2, _last2);
    }

    inline void _List_base_reverse(_List_node_base *_p)
    {
        _List_node_base *_tmp = _p;
        do
        {
            std::swap(_tmp->_M_next, _tmp->_M_prev);
            _tmp = _tmp->_M_prev; // Old next node is now prev.
        } while (_tmp != _p);
    }

    // 将*this的内容逆向
    template <class _Tp, class _Alloc>
    inline void list<_Tp, _Alloc>::reverse()
    {
        _List_base_reverse(this->_M_node);
    }

    // list不能使用 STL算法 sort()，必须使用自己的 sort() member function，
    // 因为 STL算法 sort()只接受 RamdonAccessIterator.
    // 本函式采用 quick sort.
    template <class _Tp, class _Alloc>
    void list<_Tp, _Alloc>::sort()
    {
        // 如果是空白串行，或仅有㆒个元素，就不做任何动作。
        // 使用 size() == 0 || size() == 1来判断，虽然也可以，但是比较慢。
        //  Do nothing if the list has length 0 or 1.
        if (_M_node->_M_next != _M_node && _M_node->_M_next->_M_next != _M_node)
        {
            list<_Tp, _Alloc> _carry;
            list<_Tp, _Alloc> _counter[64];
            int _fill = 0;
            while (!empty())
            {
                _carry.splice(_carry.begin(), *this, begin());
                int _i = 0;
                while (_i < _fill && !_counter[_i].empty())
                {
                    _counter[_i].merge(_carry);
                    _carry.swap(_counter[_i++]);
                }
                _carry.swap(_counter[_i]);
                if (_i == _fill)
                    ++_fill;
            }

            for (int _i = 1; _i < _fill; ++_i)
                _counter[_i].merge(_counter[_i - 1]);
            swap(_counter[_fill - 1]);
        }
    }

}; // namespace lp;

#endif /* _SGI_STL_INTERNAL_LIST_H */

// Local Variables:
// mode:C++
// End:
