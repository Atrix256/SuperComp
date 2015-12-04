//=================================================================================
//
//  CFixed
//
//  A fixed point reference implementation to use for comparison in CSuperFixed
//  unit tests.
//
//=================================================================================

#pragma once

#include "CSuperFixed.h"

// TODO: make sure and honor CSUPERFIXED_EXTENDPRECISION_MULTIPLY() and CSUPERFIXED_EXTENDPRECISION_DIVIDE()
// TODO: implement this!

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
class CFixed
{
public:
    CFixed (float value = 0.0f)
    {
        static_assert(BITS_INTEGER + BITS_FRACTION > 0, "size must be greater than 0");
        SetFloat(value);
    }

    void SetFloat (float value)
    {
        m_int = (int)((unsigned int)(value * c_floatToInt)&c_bitMask);
    }

    float GetFloat ()
    {
        return m_int * c_intToFloat;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator + (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = m_int + other.m_int;
        result.m_int = (result.m_int & c_bitMask);
        return result;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator - (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = m_int - other.m_int;
        result.m_int = (result.m_int & c_bitMask);
        return result;
    }

private:
    int m_int;

    static const float c_floatToInt;
    static const float c_intToFloat;

    static const unsigned int c_bitMask = (1 << (BITS_INTEGER + BITS_FRACTION)) - 1;
};

// TODO: maybe need to do math in unsigned and just deal with sign as needed?

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt = std::pow(2.0f, (float)BITS_FRACTION);

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CFixed<BITS_INTEGER, BITS_FRACTION>::c_intToFloat = 1.0f / CFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt;
