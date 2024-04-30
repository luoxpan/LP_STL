#include "1_allocator/lp_memory.h"
#include <iostream>
#include <string>
class TestClass
{
public:
    int id;
    char *name;
    long double data[100];

public:
    TestClass(int id, char *name)
    {
        this->id = id;
        this->name = new char[strlen(name) + 1];
        strcpy(this->name, name);
        for (int i = 0; i < 100; i++)
        {
            data[i] = i;
        }
    }
    ~TestClass() {}
};

int main()
{
    std::cout << "Testing LP memory management..." << std::endl;

    // 测试allocate和deallocate函数
    TestClass *testClass = lp::simple_alloc<TestClass, lp::alloc>::allocate(30);

    if (testClass == nullptr)
    {
        std::cout << "Allocation failed!" << std::endl;
    }
    else
    {
        for (int i = 0; i < 3; i++)
        {
            testClass[i].id = i;
            testClass[i].name = new char[10];
            strcpy(testClass[i].name, "test");
        }
        std::cout << "The id of the first object is: " << testClass[1].id << std::endl;
        std::cout << "The name of the first object is: " << testClass[1].name << std::endl;
        std::cout << "The id of the second object is: " << testClass[2].id << std::endl;
        std::cout << "The name of the second object is: " << testClass[2].name << std::endl;
    }

    lp::simple_alloc<TestClass, lp::alloc>::deallocate(testClass, 30);
    std::cout << "Deallocation done!" << std::endl;

    // 测试uninitialized_fill函数
    int *arr1 = lp::simple_alloc<int, lp::alloc>::allocate(5);
    lp::uninitialized_fill(arr1, arr1 + 5, 1);

    std::cout << "The array after uninitialized_fill is: ";
    for (int i = 0; i < 5; ++i)
        std::cout << arr1[i] << " ";
    std::cout << std::endl;

    // 测试uninitialized_fill_n函数
    int *arr2 = lp::simple_alloc<int, lp::alloc>::allocate(5);
    lp::uninitialized_fill_n(arr2, 5, 2);

    std::cout << "The array after uninitialized_fill_n is: ";
    for (int i = 0; i < 5; ++i)
        std::cout << arr2[i] << " ";
    std::cout << std::endl;

    // 测试uninitialized_copy函数
    int *arr3 = lp::simple_alloc<int, lp::alloc>::allocate(5);
    lp::uninitialized_copy(arr1, arr1 + 5, arr3);

    std::cout << "The array after uninitialized_copy is: ";
    for (int i = 0; i < 5; ++i)
        std::cout << arr3[i] << " ";
    std::cout << std::endl;

    lp::simple_alloc<int, lp::alloc>::deallocate(arr1, 5);
    std::cout << "arr1 deallocated done!" << std::endl;
    lp::simple_alloc<int, lp::alloc>::deallocate(arr2, 5);
    std::cout << "arr2 deallocated done!" << std::endl;
    lp::simple_alloc<int, lp::alloc>::deallocate(arr3, 5);
    std::cout << "arr3 deallocated done!" << std::endl;
    return 0;
}