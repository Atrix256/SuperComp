//=================================================================================
//
//  CSuperFixed
//
//  A superpositional fixed point number, using CSuperInt internally
//
//=================================================================================

#pragma once

#include "CSuperInt.h"

// fixed point math details here: http://x86asm.net/articles/fixed-point-arithmetic-and-tricks/
// TODO: remove the above comment when done!

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
class CSuperFixed
{
public:
    // initialize to zero
    CSuperFixed (const std::shared_ptr<CKeySet> &keySet)
        : m_int(keySet)
    {
        // TODO: is there any restrictions on size of either parts of the fixed point number?
        //static_assert(NUMBITS > 0, "NUMBITS must be greater than 0");
    }

    // initialize to a non superpositional value
    CSuperFixed (float value, const std::shared_ptr<CKeySet> &keySet)
        : m_int(keySet)
    {
        SetFloat(value);
    }

    // initialize to a range of a vector of superpositional bits
    CSuperFixed (
        std::vector<TINT>::const_iterator &bitsBegin,
        const std::shared_ptr<CKeySet> &keySet)
        : m_int(bitsBegin, keySet)
    { }

    void SetFloat (float value)
    {
        m_int.SetInt((int)(value * c_floatToInt));
    }

    float DecodeFloat (const TINT& key) const
    {
        return ((float)m_int.DecodeInt(key)) * c_intToFloat;
    }

    //=================================================================================
    // Math operations
    //=================================================================================
    CSuperFixed<BITS_INTEGER, BITS_FRACTION> operator + (const CSuperFixed<BITS_INTEGER, BITS_FRACTION>& other) const
    {
        CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
        result.m_int = m_int + other.m_int;
        return result;
    }

    CSuperFixed<BITS_INTEGER, BITS_FRACTION> operator - (const CSuperFixed<BITS_INTEGER, BITS_FRACTION>& other) const
    {
        CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
        result.m_int = m_int - other.m_int;
        return result;
    }

    CSuperFixed<BITS_INTEGER, BITS_FRACTION> operator * (const CSuperFixed<BITS_INTEGER, BITS_FRACTION>& other) const
    {
        const size_t c_intermediaryBits = (BITS_INTEGER + BITS_FRACTION) * 2 - 1;

        // copy values into larger intermediary sized integer
        CSuperInt<c_intermediaryBits> a(m_int.GetKeySet());
        CSuperInt<c_intermediaryBits> b(m_int.GetKeySet());
        for (size_t i = 0; i < (BITS_INTEGER + BITS_FRACTION); ++i)
        {
            a.GetBit(i) = m_int.GetBit(i);
            b.GetBit(i) = other.m_int.GetBit(i);
        }

        // sign extend the larger intermediary numbers
        const TINT& aNeg = m_int.IsNegative();
        const TINT& bNeg = other.m_int.IsNegative();
        for (size_t i = (BITS_INTEGER + BITS_FRACTION); i < c_intermediaryBits; ++i)
        {
            a.GetBit(i) = aNeg;
            b.GetBit(i) = bNeg;
        }

        // do the math in higher bit intermediary format
        CSuperInt<c_intermediaryBits> c = a * b;
        c.SignedShiftRight(BITS_FRACTION);

        // copy values back into normal sized value and return it
        CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
        for (size_t i = 0; i < (BITS_INTEGER + BITS_FRACTION); ++i)
            result.m_int.GetBit(i) = c.GetBit(i);
        return result;
    }

    CSuperFixed<BITS_INTEGER, BITS_FRACTION> operator / (const CSuperFixed<BITS_INTEGER, BITS_FRACTION>& other) const
    {
        const size_t c_intermediaryBits = (BITS_INTEGER + BITS_FRACTION + BITS_FRACTION);

        // copy values into larger intermediary sized integer
        CSuperInt<c_intermediaryBits> a(m_int.GetKeySet());
        CSuperInt<c_intermediaryBits> b(m_int.GetKeySet());
        for (size_t i = 0; i < (BITS_INTEGER + BITS_FRACTION); ++i)
        {
            a.GetBit(i) = m_int.GetBit(i);
            b.GetBit(i) = other.m_int.GetBit(i);
        }

        // sign extend the larger intermediary numbers
        const TINT& aNeg = m_int.IsNegative();
        const TINT& bNeg = other.m_int.IsNegative();
        for (size_t i = (BITS_INTEGER + BITS_FRACTION); i < c_intermediaryBits; ++i)
        {
            a.GetBit(i) = aNeg;
            b.GetBit(i) = bNeg;
        }

        // do the math in higher bit intermediary format
        a.ShiftLeft(BITS_FRACTION);
        CSuperInt<c_intermediaryBits> c = a / b;

        // copy values back into normal sized value and return it
        CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
        for (size_t i = 0; i < (BITS_INTEGER + BITS_FRACTION); ++i)
            result.m_int.GetBit(i) = c.GetBit(i);
        return result;


        /*
        // TODO: do the math in a larger integer.  (BITS_INTEGER + BITS_FRACTION + BITS_FRACTION) bits
        // TODO: then copy bits to the normal sized integer, truncating some of them.
        // TODO: maybe a define for increased precision? or a different function call or soemthing? compare contrast in paper perhaps.
        const size_t c_intermediaryBits = (BITS_INTEGER + BITS_FRACTION + BITS_FRACTION);

        CSuperInt<BITS_INTEGER + BITS_FRACTION> temp(m_int);
        temp.ShiftLeft(BITS_FRACTION);
        CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
        result.m_int = temp / other.m_int;
        return result;
        */
    }

private:
    CSuperInt<BITS_INTEGER + BITS_FRACTION> m_int;

    static const float c_floatToInt;
    static const float c_intToFloat;
};

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt = std::pow(2.0f, (float)BITS_FRACTION);

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_intToFloat = 1.0f / CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt;
