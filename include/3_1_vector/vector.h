#ifndef LP_VECTOR_H_
#define LP_VECTOR_H_
#include <cstddef>
#include "../1_allocator/lp_memory.h"

namespace lp
{
    template <class T, class Alloc = alloc>
    class vector
    {
    public:
        using value_type = T;
        using pointer = value_type *;
        using iterator = value_type *;
        using reference = value_type &;
        using difference_type = ptrdiff_t;
        using size_type = size_t;

    protected:
        using data_allocator = simple_alloc<value_type, Alloc>;

        iterator start;          // 目前使用空间的头
        iterator finish;         // 目前使用空间的尾
        iterator end_of_storage; // 可用空间的结尾

        void insert_aux(iterator position, const T &x);

        // 将first到last之间的元素拷贝到result指向的位置前面
        // TODO: copy_backward 由算法模块提供
        iterator copy_backward(iterator first, iterator last, iterator result)
        {
            while (first != last)
            {
                *(--result) = *(--last);
            }
            return result;
        }

        void deallocate()
        {
            if (start != nullptr)
            {
                data_allocator::deallocate(start, end_of_storage - start);
            }
        }

        void fill_initialize(size_type n, const T &value)
        {
            iterator result = data_allocator::allocate(n);
            uninitialized_fill_n(result, n, value);
            start = result;
            finish = start + n;
            end_of_storage = finish;
        }

    public:
        iterator begin() { return start; }
        iterator end() { return finish; }
        size_type size() const { return static_cast<size_type>(end() - begin()); }
        size_type capacity() const { return static_cast<size_type>(end_of_storage - begin()); }
        bool empty() const { return begin() == end(); }
        reference operator[](size_type n) { return *(begin() + n); }
        reference front() { return *begin(); }
        reference back() { return *(end() - 1); }

        vector() : start(nullptr), finish(nullptr), end_of_storage(nullptr) {}
        vector(size_type n, const T &value) { fill_initialize(n, value); }
        // 使用explicit防止误导性的隐式转换
        explicit vector(size_type n) { fill_initialize(n, T()); }
        ~vector()
        {
            destory(start, finish);
            deallocate();
        }

        void push_back(const T &x)
        {
            if (finish != end_of_storage)
            {
                construct(finish, x);
                ++finish;
            }
            else
            {
                insert_aux(end(), x);
            }
        }
        void pop_back()
        {
            --finish;
            destory(finish);
        }

        void insert(iterator pos, size_type n, const T &x);

        iterator erase(iterator position)
        {
            if (position + 1 != end())
            {
                copy(position + 1, finish, position);
            }
            --finish;
            destroy(finish);
            return position;
        }
        void resize(size_type new_size, const T &x)
        {
            if (new_size < size())
            {
                erase(begin() + new_size, end());
            }
            else
            {
                insert(end(), new_size - size(), x);
            }
        }
        void resize(size_type new_size) { resize(new_size, T()); }
        void clear() { erase(begin(), end()); }
    };

    template <class T, class Alloc>
    void vector<T, Alloc>::insert_aux(iterator position, const T &x)
    {
        if (finish != end_of_storage)
        {
            construct(finish, *(finish - 1));
            ++finish;
            T x_copy = x;
            copy_backward(position, finish - 2, finish - 1);
            *position = x_copy;
        }
        else
        {
            const size_type old_size = size();
            const size_type new_size = old_size == 0 ? 1 : 2 * old_size;
            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try
            {
                new_finish = uninitialized_copy(begin(), position, new_start);
                construct(new_finish, x);
                new_finish++;
                new_finish = uninitialized_copy(position, finish, new_finish);
            }
            catch (...)
            {
                destory(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }
        }
        // 释放旧空间
        destory(begin(), end());
        deallocate();
        // 更新指针
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + new_size;
    }

    template <class T, class Alloc>
    void vector<T, Alloc>::insert(iterator pos, size_type n, const T &x)
    {
        if (n != 0)
        {
            if (static_cast<size_type>(end_of_storage - finish) >= n)
            {
                // 备用空间大于新增元素个数
                T x_copy = x;
                // 计算插入点之后的现有元素个数
                const size_type elems_after = finish - pos;
                iterator old_finish = finish;
                if (elems_after > n)
                {
                    uninitialized_copy(finish - n, finish, finish);
                    finish += n;
                    copy_backward(position, old_finish - n, old_finish);
                    fill(pos, pos + n, x_copy);
                }
                else
                {
                    uninitialized_fill_n(finish, n - elems_after, x_copy);
                    finish += n - elems_after;
                    uninitialized_copy(pos, old_finish, finish);
                    finish += elems_after;
                    fill(pos, old_finish, x_copy);
                }
            }
        }
        else
        {
            // 备用空间不足，需要分配新空间
            const size_type old_size = size();
            const size_type new_size = old_size + max(old_size, n);
            iterator new_start = data_allocator::allocate(new_size);
            iterator new_finish = new_start;
            try
            {
                new_finish = uninitialized_copy(begin(), pos, new_start);
                new_finish = uninitialized_fill_n(new_finish, n, x);
                new_finish = uninitialized_copy(pos, finish, new_finish);
            }
            catch (...)
            {
                destory(new_start, new_finish);
                data_allocator::deallocate(new_start, new_size);
                throw;
            }
        }
        // 释放旧空间
        destory(begin(), finish);
        deallocate();
        // 更新指针
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + new_size;
    }
};     // namespace lp
#endif // LP_VECTOR_H_