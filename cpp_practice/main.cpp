/*
 * @Author: your name
 * @Date: 2021-03-13 23:54:16
 * @LastEditTime: 2021-03-14 13:45:54
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /vscode_test/main.cpp
 */
#include <iostream>
#include "test_class.h"
#include <vector>
int fun()
{
    int a;
    int c;
    std::vector<int> vec;
    for(int i = 0; i < 10 ; ++i)
    {
        vec.push_back(i);
    }
}

int main()
{
    TestClass a;
    fun();
    std::cout << "hh" << std::endl;
    return 0;
}