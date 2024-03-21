/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-17 11:18:50
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-17 11:35:12
 * @FilePath: /MyServer/TinyMuduo/base/FileUtils.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#pragma once

#include <boost/noncopyable.hpp>
#include <sys/types.h> 
#include <string>

namespace fileutils
{
    constexpr int kBufferSize=64*1024;
    class ReadSmallFile:public boost::noncopyable
    {
    public:
        explicit ReadSmallFile(const char* fileName);
        explicit ReadSmallFile(const std::string& fileName);
        ~ReadSmallFile();

        int readToBuffer(int& size);

        const auto getBuffer()const{return m_buffer;}

    private:
        int m_fd;   //文件描述符
        int m_error{0};    //期间的错误
        char m_buffer[kBufferSize]{0};  //用来读的buffer
    };

    class AppendFile:public boost::noncopyable
    {};
}

