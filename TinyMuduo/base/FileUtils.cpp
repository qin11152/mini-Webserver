/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-17 11:18:58
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-17 11:34:47
 * @FilePath: /MyServer/TinyMuduo/base/FileUtils.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "FileUtils.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>


namespace fileutils
{
    ReadSmallFile::ReadSmallFile(const char *fileName)
    :m_fd(::open(fileName, O_RDONLY | O_CLOEXEC))
    {
        m_buffer[0]='\0';
        if(m_fd<0)
        {
            m_error=errno;
        }
    }
    ReadSmallFile::~ReadSmallFile()
    {
        if(m_fd>=0)
        {
            ::close(m_fd);
        }
    }
}