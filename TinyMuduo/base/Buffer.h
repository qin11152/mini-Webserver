/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2023-06-14 14:33:06
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2023-06-15 10:30:05
 * @FilePath: /MyServer/TinyMuduo/base/Buffer.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */

#pragma once

#include <vector>
#include <algorithm>
#include <string>
#include <string.h>

namespace base
{
    constexpr char kCRLF[] ="\r\n";
    class Buffer
    {
    public:
        //buffer前部分的大小
        static constexpr size_t kCheapPrepend=8;
        //存储空间初始大小
        static constexpr size_t kInitialSize=1024;

        /// @brief 初始化的时候可以传入想要的buffer长度
        /// @param initialSize 
        explicit Buffer(size_t initialSize=kInitialSize)
        :m_buffer(initialSize)
        {}

        void swap(Buffer& rhs)
        {
            //交换地址
            m_buffer.swap(rhs.m_buffer);
            //交换内容
            std::swap(m_iReaderIndex,rhs.m_iReaderIndex);
            std::swap(m_iWriterIndex,rhs.m_iWriterIndex);
        }


        /// @brief 返回还有多少字节可读
        /// @return 可读字节数
        auto readableBytes()const
        {
            return m_iWriterIndex - m_iReaderIndex;
        }

        /// @brief 返回还有多少可写
        /// @return 
        auto writableBytes()const
        {
            return m_buffer.size()-m_iWriterIndex;
        }

        /// @brief 返回前边的空余
        /// @return 
        auto prependableBytes()const
        {
            return m_iReaderIndex;
        }

        /// @brief 返回指向可读位置的字符串指针
        /// @return 
        const char* peek()const{
            return begin()+m_iReaderIndex;
        }

        /// @brief 查找是否含有crlf，如果有，就返回位置，如果没有就返回空
        /// @return 
        const char* findCRLF()const
        {
            const char* crlf=std::search(peek(),beginWrite(),kCRLF,kCRLF+2);
            //找到了开始写的地方就是说明不含有查找的字符串
            return crlf==beginWrite()?nullptr:crlf;
        }
        
        /// @brief 指定位置查找crlf
        /// @param start 开始的位置
        /// @return 
        const char* findCRLF(const char* start)const
        {
            const char* crlf=std::search(start,beginWrite(),kCRLF,kCRLF+2);
            return crlf==beginWrite()?nullptr:crlf;
        }

        const char* findEOL()const
        {
            const void* eol=memchr(peek(),'\n',readableBytes());
            return static_cast<const char*>(eol);
        }

        const char* findEOL(const char* start) const
        {
            const void* eol = memchr(start, '\n', beginWrite() - start);
            return static_cast<const char*>(eol);
        }

        /// @brief 向后移动
        /// @param len 移动的长度
        void retrieve(size_t len)
        {
            //小于可读去的字节数，就把可读的+这个长度
            if(len<readableBytes())
            {
                m_iReaderIndex+=len;
            }
            //都读完了，就调用all
            else
            {
                retrieveAll();
            }
        }

        /// @brief 移动一定范围，尾部是end，头部还是可读的位置
        /// @param end 检索的终点
        void retrieveUtil(const char* end)
        {
            retrieve(end-peek());
        }

        /// @brief 移动int64长度的
        void retrieveInt64()
        {
            retrieve(sizeof(int64_t));
        }

        /// @brief 移动int32长度的
        void retrieveInt32()
        {
            retrieve(sizeof(int32_t));
        }
        
        /// @brief 移动int16长度的
        void retrieveInt16()
        {
            retrieve(sizeof(int16_t));
        }
        
        /// @brief 移动int8长度的
        void retrieveInt8()
        {
            retrieve(sizeof(int8_t));
        }

        /// @brief 清空了，复位read和write的index
        void retrieveAll()
        {
            m_iReaderIndex=kCheapPrepend;
            m_iWriterIndex=kCheapPrepend;
        }

        /// @brief 移动到最后并返回一个字符串（字符串就是buffer中所有的内容）
        /// @return buffer的字符串
        std::string retrieveAllAsString()
        {
            return retrieveAsString(readableBytes());
        }

        /// @brief 移动一定长度，并以返回范围内的字符串
        /// @param len 检索的长度
        /// @return 检索范围没的字符串
        std::string retrieveAsString(size_t len)
        {
            std::string result(peek(),len);
            retrieve(len);
            return result;
        }

        /// @brief 从尾部添加内容
        /// @param str 需要添加的字符串
        void append(const std::string& str)
        {
            append(str.data(),str.size());
        }

        /// @brief 尾部添加内容
        /// @param data 字符串指针
        /// @param len 长度
        void append(const char* data,size_t len)
        {
            ensureWritableBytes(len);
            std::copy(data,data+len,beginWrite());
            hasWritten(len);
        }

        void append(const void* data,size_t len)
        {
            append(static_cast<const char*>(data),len);
        }

        /// @brief 确保能写入len长度的内容，不够就得扩容
        /// @param len 学要写入的长度
        void ensureWritableBytes(size_t len)
        {
            if(m_iWriterIndex<len)
            {
                makeSpace(len);
            }
        }

        /// @brief 返回当前buffer没写的起点
        /// @return 可写起点的字符串指针
        char* beginWrite()
        {
            return begin()+m_iWriterIndex;
        }

        /// @brief 返回当前buffer没写的起点
        /// @return 可写起点的字符串指针
        const char* beginWrite()const
        {
            return begin()+m_iWriterIndex;
        }

        /// @brief 写之后更新buffer写的起点
        /// @param len 新写入的长度
        void hasWritten(size_t len)
        {
            m_iWriterIndex+=len;
        }

        void unwrite(size_t len)
        {
            m_iWriterIndex-=len;
        }

        void appendInt64(int64_t x)
        {
            int64_t be64=htobe64(x);
            append(&be64,sizeof(be64));
        }

        void appendInt32(int32_t x)
        {
            int32_t be32=htobe32(x);
            append(&be32,sizeof(be32));
        }

        void appendInt64(int16_t x)
        {
            int64_t be16=htobe16(x);
            append(&be16,sizeof(be16));
        }

        void appendInt8(int8_t x)
        {
            append(&x,sizeof(x));
        }

        int64_t readInt64()
        {
            int64_t result=peekInt64();
            retrieveInt64();
            return result;
        }

        int32_t readInt32()
        {
            int64_t result=peekInt32();
            retrieveInt32();
            return result;
        }


        int64_t readInt16()
        {
            int64_t result=peekInt16();
            retrieveInt16();
            return result;
        }

        int32_t readInt8()
        {
            int64_t result=peekInt8();
            retrieveInt8();
            return result;
        }

        int64_t peekInt64()const
        {
            int64_t be64=0;
            memcpy(&be64,peek(),sizeof(be64));
            return be64toh(be64);
        }

        int32_t peekInt32()const
        {
            int64_t be32=0;
            memcpy(&be32,peek(),sizeof(be32));
            return be32toh(be32);
        }

        int16_t peekInt16()const
        {
            int16_t be16=0;
            memcpy(&be16,peek(),sizeof(be16));
            return be16toh(be16);
        }

        int8_t peekInt8()const
        {
            int64_t be8=0;
            memcpy(&be8,peek(),sizeof(be8));
            return be8;
        }

        void prependInt64(int64_t x)
        {
            int64_t be64=htobe64(x);
            prepend(&be64,sizeof(be64));
        }

        void prependInt32(int32_t x)
        {
            int32_t be32=htobe32(x);
            prepend(&be32,sizeof(be32));
        }

        void prependInt64(int16_t x)
        {
            int16_t be16=htobe16(x);
            prepend(&be16,sizeof(be16));
        }

        void prependInt8(int8_t x)
        {
            prepend(&x,sizeof(x));
        }

        void prepend(const void* data,size_t len)
        {
            m_iReaderIndex-=len;
            const char* d=static_cast<const char*>(data);
            std::copy(d,d+len,begin()+m_iReaderIndex);
        }

        void shrink(size_t reserve)
        {
            Buffer other;
            other.ensureWritableBytes(readableBytes()+reserve);
            other.append(std::string(peek(),readableBytes()));
            swap(other);
        }

        auto internalCapacity()const
        {
            return m_buffer.capacity();
        }

        ssize_t readFd(int fd,int* savedError);
    private:
        char* begin()
        {
            return &*m_buffer.begin();
        }

        const char* begin()const
        {
            return &*m_buffer.begin();
        }

        void makeSpace(size_t len)
        {
            if(writableBytes()+prependableBytes()<len+kCheapPrepend)
            {
                m_buffer.resize(m_iWriterIndex+len);
            }
            else
            {
                auto readable=readableBytes();
                std::copy(begin()+m_iReaderIndex,begin()+m_iWriterIndex,begin()+kCheapPrepend);
                m_iReaderIndex=kCheapPrepend;
                m_iWriterIndex=m_iReaderIndex+readable;
            }
        }

    private:
        //buffer用来存储内容的区域
        std::vector<char> m_buffer;
        //可读字节从哪开始
        size_t m_iReaderIndex{kCheapPrepend};
        //可写字节从哪开始，也就是最后一个可读的位置+1
        size_t m_iWriterIndex{kCheapPrepend};
    };
}

