#ifndef LP_UNINITIALIZED_H
#define LP_UNINITIALIZED_H
#include <type_traits>
#include <new>
#include <cstring>
namespace lp
{
    // region:uninitialized_copy
    template <class InputIterator, class ForwardIterator>
    ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, std::true_type);

    template <class InputIterator, class ForwardIterator>
    ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, std::false_type);
    template <class InputIterator, class ForwardIterator>

    inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
    {
        // std::iterator_traits 替换为 lp::iterator_traits
        using ValueType = typename std::iterator_traits<InputIterator>::value_type;
        return _uninitialized_copy(first, last, result, std::is_trivially_copyable<ValueType>());
    }

    template <class InputIterator, class ForwardIterator>
    ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, std::true_type)
    {
        // TODO: std::copy改为lp::copy
        return std::copy(first, last, result);
    }

    template <class InputIterator, class ForwardIterator>
    ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, std::false_type)
    {
        ForwardIterator cur = result;
        try
        {
            for (; first != last; ++first, ++cur)
            {
                construct(&*cur, *first);
            }
        }
        catch (...)
        {
            // 如果在构造过程中抛出异常，销毁已经构造的对象
            while (cur != result)
            {
                --cur;
                destroy(&*cur);
            }
            throw;
        }
        return cur;
    }

    // 对char*和wchar_t*等指针类型，使用memmove进行优化
    inline char *uninitialized_copy(const char *first, const char *last, char *result)
    {
        memmove(result, first, last - first);
        return result + (last - first);
    }

    inline wchar_t *uninitialized_copy(const wchar_t *first, const wchar_t *last, wchar_t *result)
    {
        memmove(result, first, (last - first) * sizeof(wchar_t));
        return result + (last - first);
    }
    // endregion uninitialized_copy

    // region:uninitialized_fill
    template <class ForwardIterator, class T>
    void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &value, std::false_type);

    template <class ForwardIterator, class T>
    void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &value, std::true_type);

    template <class ForwardIterator, class T>
    inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &value)
    {
        // 根据 T 的类型，决定是使用 std::true_type 还是 std::false_type
        // 然后使用这个类型来调用 _uninitialized_fill 函数
        _uninitialized_fill(first, last, value, std::is_trivial<T>());
    }

    template <class ForwardIterator, class T>
    inline void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &value, std::true_type)
    {
        // TODO: std::fill改为lp::fill
        std::fill(first, last, value);
    }

    template <class ForwardIterator, class T>
    void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T &value, std::false_type)
    {
        ForwardIterator cur = first;
        try
        {
            for (; cur != last; ++cur)
            {
                construct(&*cur, value);
            }
        }
        catch (...)
        {
            // 如果在构造过程中抛出异常，销毁已经构造的对象
            while (cur != first)
            {
                --cur;
                destroy(&*cur);
            }
            throw; // 重新抛出异常
        }
    }
    // endregion uninitialized_fill
    // region:uninitialized_fill_n
    // 先声明,实现在后面.
    template <class ForwardIterator, class Size, class T>
    inline ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T &value, std::true_type);

    template <class ForwardIterator, class Size, class T>
    inline ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T &value, std::false_type);

    template <class ForwardIterator, class Size, class T>
    inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T &value)
    {
        // 然后使用这个类型来调用 _uninitialized_fill 函数
        return _uninitialized_fill_n(first, n, value, std::is_trivial<T>());
    }

    template <class ForwardIterator, class Size, class T>
    inline ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T &value, std::true_type)
    {
        // 对Trivial类型可以直接使用内存填充操作，如通过使用标准库算法std::fill
        // TODO: std::fill_n改为lp::fill_n
        return std::fill_n(first, n, value);
    }

    template <class ForwardIterator, class Size, class T>
    ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T &value, std::false_type)
    {
        ForwardIterator cur = first;
        try
        {
            for (; n > 0; --n, ++cur)
            {
                construct(&*cur, value);
            }
        }
        catch (...)
        {
            // 如果在构造过程中抛出异常，销毁已经构造的对象
            while (cur != first)
            {
                --cur;
                destroy(&*cur);
            }
            throw; // 重新抛出异常
        }
        return cur;
    }
    // endregion uninitialized_fill_n
} // namespace LP
#endif // LP_UNINITIALIZED_H