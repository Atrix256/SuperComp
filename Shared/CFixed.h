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

// TODO: honor CSUPERFIXED_EXTENDPRECISION_MULTIPLY() and CSUPERFIXED_EXTENDPRECISION_DIVIDE()

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
class CFixed
{
public:
    CFixed(float value = 0.0f)
    {
        static_assert(BITS_INTEGER + BITS_FRACTION > 0, "size must be greater than 0");
        SetFloat(value);
    }

    void SetFloat(float value)
    {
        m_int = (int)((unsigned int)(value * c_floatToInt)&c_bitMask);
    }

    float GetFloat()
    {
        bool isNegative = (m_int & c_negativeTestBit) != 0;
        unsigned int magnitude = m_int;
        if (isNegative)
            magnitude = ((~magnitude) + 1) & c_bitMask;

        float ret = magnitude * c_intToFloat;
        if (isNegative)
            ret *= -1.0f;

        return ret;

        //return m_int * c_intToFloat;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator + (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = (m_int + other.m_int) & c_bitMask;
        return result;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator - (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = (m_int - other.m_int) & c_bitMask;
        return result;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator * (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = (m_int * other.m_int) & c_bitMask;
        result.m_int = (result.m_int >> BITS_FRACTION) & c_bitMask;
        return result;
    }

    CFixed<BITS_INTEGER, BITS_FRACTION> operator / (const CFixed<BITS_INTEGER, BITS_FRACTION>& other)
    {
        CFixed<BITS_INTEGER, BITS_FRACTION> temp;
        temp.m_int = (m_int << BITS_FRACTION) & c_bitMask;
        CFixed<BITS_INTEGER, BITS_FRACTION> result;
        result.m_int = (temp.m_int / other.m_int) & c_bitMask;
        return result;
    }

private:
    unsigned int m_int;

    static const float c_floatToInt;
    static const float c_intToFloat;

    static const unsigned int c_bitMask = (1 << (BITS_INTEGER + BITS_FRACTION)) - 1;
    static const unsigned int c_negativeTestBit = (1 << (BITS_INTEGER + BITS_FRACTION - 1));
};

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt = std::pow(2.0f, (float)BITS_FRACTION);

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CFixed<BITS_INTEGER, BITS_FRACTION>::c_intToFloat = 1.0f / CFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt;
