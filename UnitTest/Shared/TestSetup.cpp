#include "stdafx.h"

#include "TestSetup.h"


namespace NativeJIT
{
    //
    // TestFixture
    //

    TestFixture::TestFixture()
        : TestFixture(c_defaultCodeAllocatorCapacity, c_defaultGeneralAllocatorCapacity)
    {
    }


    TestFixture::TestFixture(unsigned codeAllocatorCapacity,
                             unsigned generalAllocatorCapacity)
        : m_codeAllocator(codeAllocatorCapacity),
          m_generalAllocator(generalAllocatorCapacity),
          m_code(m_codeAllocator, codeAllocatorCapacity, m_generalAllocator),
          m_testCaseAllocator(generalAllocatorCapacity)
    {
    }


    std::unique_ptr<TestCaseSetup> TestFixture::GetSetup()
    {
        return std::make_unique<TestCaseSetup>(m_code, m_testCaseAllocator);
    }


    //
    // TestCaseSetup
    //

    TestCaseSetup::TestCaseSetup(FunctionBuffer& code, Allocator& generalAllocator)
        : m_code(code),
          m_allocator(generalAllocator)
    {
    }


    TestCaseSetup::~TestCaseSetup()
    {
        m_allocator.Reset();
        m_code.Reset();
    }


    FunctionBuffer& TestCaseSetup::GetCode()
    {
        return m_code;
    }


    Allocators::IAllocator& TestCaseSetup::GetAllocator()
    {
        return m_allocator;
    }
}
