#include "stdafx.h"

#include <stdexcept>
#include <Windows.h>

#include "NativeJIT/ExecutionBuffer.h"
#include "Temporary/Assert.h"


namespace NativeJIT
{
namespace Windows
{
    //*************************************************************************
    //
    // ExecutionBuffer
    //
    //*************************************************************************

    // Rounds x up to the nearest size, where size is a power of 2.
    size_t RoundUp(size_t  x, size_t powerOfTwo)
    {
        return (x + powerOfTwo - 1) & (~(powerOfTwo - 1));
    }


    // http://stackoverflow.com/questions/570257/jit-compilation-and-dep
    ExecutionBuffer::ExecutionBuffer(size_t bufferSize)
        : m_buffer(nullptr),
          m_bytesAllocated(0)
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo(&systemInfo);

        m_bufferSize = RoundUp(bufferSize, systemInfo.dwPageSize);

        // Allocate m_bufferSize bytes plus one extra page that will act as 
        // a write-guard to detect buffer overruns.
        m_buffer = (unsigned char*)VirtualAlloc(NULL, m_bufferSize + systemInfo.dwPageSize, 
                                                MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        if (m_buffer == NULL)
        {
            throw std::runtime_error("CodeBuffer: out of memory.");
        }


        // Set protection on the guard page.
        DWORD oldProtection;
        if (!VirtualProtect(m_buffer + m_bufferSize, systemInfo.dwPageSize, 
                            PAGE_NOACCESS, &oldProtection))
        {
            throw std::runtime_error("CodeBuffer: failed to set protection on guard page.");
        }


        //m_bufferEnd = m_bufferStart + size;
        //m_current = m_bufferStart;
    }


    ExecutionBuffer::~ExecutionBuffer()
    {
        if (m_buffer != nullptr)
        {
            if (VirtualFree(m_buffer, 0, MEM_RELEASE) == 0)
            {
                throw std::runtime_error("CodeBuffer: VirtualFree failed.");
            }
        }
    }


    //
    // IAllocator methods
    //

    // Allocates a block of a specified byte size.
    void* ExecutionBuffer::Allocate(size_t size)
    {
        Assert(m_bytesAllocated + size <= m_bufferSize, "Out of memory");

        void* result = static_cast<void*>(m_buffer + m_bytesAllocated);
        m_bytesAllocated += size;

        return result;
    }


    // Frees a block.
    void ExecutionBuffer::Deallocate(void* /*block*/)
    {
        //Assert(static_cast<char*>(block) > m_buffer.get() &&
        //       static_cast<char*>(block) < m_buffer.get() + m_bytesAllocated,
        //       "Attempting to deallocate memory not owned by this allocator.");

        // Intentional NOP
    }


    // Returns the maximum legal allocation size in bytes.
    size_t ExecutionBuffer::MaxSize() const
    {
        return m_bufferSize;
    }


    // Frees all blocks that have been allocated since construction or the
    // last call to Reset().
    void ExecutionBuffer::Reset()
    {
        m_bytesAllocated = 0;
        DebugInitialize();
    }


    void ExecutionBuffer::DebugInitialize()
    {
        memset(m_buffer, 0xcc, m_bufferSize);
    }
}
}