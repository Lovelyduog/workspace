/*
 * @Author: your name
 * @Date: 2021-03-13 23:54:16
 * @LastEditTime: 2021-03-15 22:31:35
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /vscode_test/main.cpp
 */
#include <iostream>
#include <vector>
#include "template.h"

int main()
{
    int a = 1;
    int b = any::foo(&a) + 1;
    std::vector<int> vec;
    int   nnn;
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    vec.push_back(1);
    std::cout << "hh" << b << std::endl;
    return 0;
}