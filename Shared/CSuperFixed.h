//=================================================================================
//
//  CSuperFixed
//
//  A superpositional fixed point number, using CSuperInt internally
//
//=================================================================================

#pragma once

#include "CSuperInt.h"

// defines to extend precision of intermediate values when performing operations
#define CSUPERFIXED_EXTENDPRECISION_MULTIPLY()  1
#define CSUPERFIXED_EXTENDPRECISION_DIVIDE()    0

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
class CSuperFixed
{
public:
    // initialize to zero
    CSuperFixed (const std::shared_ptr<CKeySet> &keySet)
        : m_int(keySet)
    {
        static_assert(BITS_INTEGER + BITS_FRACTION > 0, "size must be greater than 0");
    }

    // initialize to a non superpositional value
    CSuperFixed (float value, const std::shared_ptr<CKeySet> &keySet)
        : m_int(keySet)
    {
        static_assert(BITS_INTEGER + BITS_FRACTION > 0, "size must be greater than 0");
        SetFloat(value);
    }

    // initialize to a range of a vector of superpositional bits
    CSuperFixed (
        std::vector<TINT>::const_iterator &bitsBegin,
        const std::shared_ptr<CKeySet> &keySet)
        : m_int(bitsBegin, keySet)
    {
        static_assert(BITS_INTEGER + BITS_FRACTION > 0, "size must be greater than 0");
    }

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
        #if CSUPERFIXED_EXTENDPRECISION_MULTIPLY()
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
        #else
            CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
            result.m_int = m_int * other.m_int;
            result.m_int.SignedShiftRight(BITS_FRACTION);
            return result;
        #endif
    }

    CSuperFixed<BITS_INTEGER, BITS_FRACTION> operator / (const CSuperFixed<BITS_INTEGER, BITS_FRACTION>& other) const
    {
        #if CSUPERFIXED_EXTENDPRECISION_DIVIDE()
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
        #else
            CSuperInt<BITS_INTEGER + BITS_FRACTION> temp(m_int);
            temp.ShiftLeft(BITS_FRACTION);
            CSuperFixed<BITS_INTEGER, BITS_FRACTION> result(m_int.GetKeySet());
            result.m_int = temp / other.m_int;
            return result;
        #endif
    }

    // Flip sign
    void Negate ()
    {
        m_int.Negate();
    }

    void NegateConditional (const TINT& condition)
    {
        m_int.NegateConditional(condition);
    }

    // absolute value.
    void Abs ()
    {
        m_int.Abs();
    }

    // returns a superpositional value for whether or not this number is negative
    const TINT& IsNegative() const { return m_int.IsNegative() }

public:
    static const size_t c_numBits = BITS_INTEGER + BITS_FRACTION;
    static const size_t c_numIntegerBits = BITS_INTEGER;
    static const size_t c_numFractionBits = BITS_FRACTION;

private:
    CSuperInt<BITS_INTEGER + BITS_FRACTION> m_int;

    static const float c_floatToInt;
    static const float c_intToFloat;
};

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt = std::pow(2.0f, (float)BITS_FRACTION);

template <size_t BITS_INTEGER, size_t BITS_FRACTION>
const float CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_intToFloat = 1.0f / CSuperFixed<BITS_INTEGER, BITS_FRACTION>::c_floatToInt;
