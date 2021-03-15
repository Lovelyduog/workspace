/*
 * @Author: shenglish
 * @Date: 2021-03-15 21:25:11
 * @LastEditTime: 2021-03-15 22:20:16
 * @LastEditors: Please set LastEditors
 * @Description: learning template
 * @FilePath: /cpp_practice/template.h
 */

#include "stdio.h"
namespace any{

    template<typename T>
    T foo(T *ptr)
    {
        return *ptr;
    }

    template<typename T>
    class vector
    {
    public:
        vector() {data = nullptr; size = 0; capacity = 0;};
        void push_back(T value);
        ~vector() {myfree(); };
    private:

        void myfree();
    private:
        T* data;
        int size;
        int capacity;
    };

    template<typename T>
    void vector<T>::myfree()
    {
        free(data);
        data = nullptr;
    }

    template<typename T>
    void vector<T>::push_back(T value)
    {
        if (size == 0 && capacity == 0)
        {
            data = (T*)malloc(sizeof(T) * 1);
            capacity = 1;
        }
        else if (size == capacity)
        {
            capacity *= 2;
            T *data_t = (T*)malloc(sizeof(T) * capacity);
            for (int i = 0; i < size; ++i)
            {
                data_t[i] = data[i];
            }
            myfree();
            data = data_t;        
        }
        ++size;
        data[size - 1] = value;
    }
}