//=================================================================================
//
//  CSuperInt
//
//  A superpositional integer, made up of a collection of superpositional bits.
//  Uses two's complement.
//
//=================================================================================

#pragma once

#include <vector>
#include <array>
#include <memory>
#include "CKeySet.h"
#include "TINT.h"

#define NOMINMAX
#include <Windows.h> // for IsDebuggerPresent() and DebugBreak()

#define ExitCode_(x) {if (IsDebuggerPresent()) {WaitForEnter();} return x;}

#define Assert_(x) if (!(x)) { printf("Assert Failed : " #x); DebugBreak(); }

//=================================================================================
template <size_t NUMBITS, bool USELOOKUPTABLES>
class CSuperInt
{
public:
    // initialize to zero
    CSuperInt (const std::shared_ptr<CKeySet> &keySet)
        : m_keySet(keySet)
    {
        static_assert(NUMBITS > 0, "NUMBITS must be greater than 0");
        std::fill(m_bits.begin(), m_bits.end(), 0);
    }

    // initialize to a non superpositional value
    CSuperInt (int value, const std::shared_ptr<CKeySet> &keySet)
        : m_keySet(keySet)
    {
        static_assert(NUMBITS > 0, "NUMBITS must be greater than 0");
        SetInt(value);
    }

    // initialize to a range of a vector of superpositional bits
    CSuperInt (
        std::vector<TINT>::const_iterator &bitsBegin,
        const std::shared_ptr<CKeySet> &keySet)
        : m_keySet(keySet)
    {
        static_assert(NUMBITS > 0, "NUMBITS must be greater than 0");
        for (size_t i = 0; i < NUMBITS; ++i)
            m_bits[i] = bitsBegin[i];
    }

    // decode value into binary for the given key
    size_t DecodeBinary (const TINT& key) const
    {
        size_t result = 0;
        for (size_t i = 0; i < NUMBITS; ++i)
        {
            TINT value = (m_bits[i] % key) % 2;
            result = result | (value.convert_to<size_t>() << i);
        }
        return result;
    }

    int DecodeInt (const TINT& key) const
    {
        return IntFromBinary(DecodeBinary(key));
    }

    // Helper functions
    void SetInt (int value)
    {
        Assert_(value >= c_minValue);
        Assert_(value <= c_maxValue);
        size_t binary = (size_t)value;
        for (size_t i = 0; i < NUMBITS; ++i, binary = binary >> 1)
            m_bits[i] = (binary & 1) ? 1 : 0;
    }

    void SetToBinaryMax()
    {
        for (size_t i = 0; i < NUMBITS; ++i)
            m_bits[i] = 1;
    }

    // shifting is way less expensive than other operations since it doesn't use logic circuits
    // it just moves around the internal representation of the bits
    void ShiftLeft (size_t amount)
    {
        if (amount == 0)
            return;

        for (size_t index = NUMBITS - 1; index >= amount; --index)
            m_bits[index] = m_bits[index - amount];

        for (size_t index = 0; index < amount; ++index)
            m_bits[index] = 0;
    }

    void SignedShiftRight (size_t amount)
    {
        if (amount == 0)
            return;

        const TINT& signBit = IsNegative();

        for (size_t index = 0; index < NUMBITS - amount; ++index)
            m_bits[index] = m_bits[index + amount];

        for (size_t index = NUMBITS - amount; index < NUMBITS; ++index)
            m_bits[index] = signBit;
    }

    // multiply by -1. Negates the bits, then add 1
    void Negate ()
    {
        const CKeySet& keySet = *m_keySet;
        for (TINT& bit : m_bits)
            bit = NOT(bit, keySet);

        CSuperInt<NUMBITS, false> one(1, m_keySet);
        *this = *this + one;
    }

    void NegateConditional (const TINT& condition)
    {
        // To negate in two's complement, we flip the bits and then add 1.
        // This effectively multiplies by -1.

        // make the adder here in case the condition is part of this int
        // as is the case of when doing abs
        CSuperInt<NUMBITS, false> add(m_keySet);
        add.GetBit(0) = condition;

        // Step 1 - negate by XORing every bit against the condition bit.
        // AKA negate bits conditionally.
        for (TINT& v : m_bits)
            v = XOR(v, condition, *m_keySet);

        // Step 2 - add a number where the lowest bit is the condition bit.
        // AKA add 1 conditionally.
        CSuperInt<NUMBITS, false> tempTest(m_keySet);
        *this = *this + add;
    }

    // absolute value.  Negate if the number is negative
    void Abs ()
    {
        NegateConditional(IsNegative());
    }

    // returns a superpositional value for whether or not this number is negative
    const TINT& IsNegative() const { return *m_bits.rbegin(); }

    // Internals Access
    size_t MaxError(const TINT& key) const
    {
        float maxError = 0.0f;
        for (size_t i = 0; i < NUMBITS; ++i)
        {
            TINT residue = GetBit(i) % key;
            float error = residue.convert_to<float>() / key.convert_to<float>();
            if (error > maxError)
                maxError = error;
        }
        return size_t(maxError * 100.0f);
    }

    const TINT& GetBit(size_t i) const { return m_bits[i]; }
    TINT& GetBit(size_t i) { return m_bits[i]; }

    const std::array<TINT, NUMBITS>& GetBits () const {
        return m_bits;
    }
    std::array<TINT, NUMBITS>& GetBits() {
        return m_bits;
    }

    const std::shared_ptr<CKeySet> &GetKeySet() const {
        return m_keySet;
    }

// static interface
public:
    static int IntFromBinary (size_t n)
    {
        n = n & c_mask;
        if (n & c_negativeTestMask)
            return -(int)((n ^ c_mask) + 1);
        else
            return int(n);
    }

// public constants
public:
    static const int c_minValue = -((1 << (NUMBITS - 1)));
    static const int c_maxValue = (-c_minValue) - 1;
    static const size_t c_negativeTestMask = 1 << (NUMBITS - 1);
    static const size_t c_mask = (1 << NUMBITS) - 1;
    static const size_t c_numBits = NUMBITS;

// private members
private:
    std::array<TINT, NUMBITS>   m_bits;     // the superpositional bits
    std::shared_ptr<CKeySet>    m_keySet;   // the keys to decode the bits

    static const TINT           s_zeroBit;
};

template <size_t NUMBITS, bool USELOOKUPTABLES>
const TINT CSuperInt<NUMBITS, USELOOKUPTABLES>::s_zeroBit = 0;

//=================================================================================
// HE operations
//=================================================================================
inline TINT XOR (const TINT &A, const TINT &B, const CKeySet &keySet)
{
    TINT result = A + B;
    keySet.ReduceValue(TINT(result));
    return result;
}

//=================================================================================
inline TINT AND(const TINT &A, const TINT &B, const CKeySet &keySet)
{
    TINT result = A * B;
    keySet.ReduceValue(result);
    return result;
}

//=================================================================================
inline TINT NOT(const TINT &A, const CKeySet &keySet)
{
    return XOR(A, TINT(1), keySet);
}

//=================================================================================
// Math operations for non lookup table math
//=================================================================================
inline TINT FullAdder(const TINT &A, const TINT &B, TINT &carryBit, const CKeySet &keySet)
{
    // homomorphically add the encrypted bits A and B
    // return the single bit sum, and put the carry bit into carryBit
    // From http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder
    TINT sumBit = XOR(XOR(A, B, keySet), carryBit, keySet);
    carryBit = XOR(AND(A, B, keySet), AND(carryBit, XOR(A, B, keySet), keySet), keySet);
    return sumBit;
}

//=================================================================================
template <size_t NUMBITS>
CSuperInt<NUMBITS, false> operator + (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    // we initialize the carry bit to 0, not a superpositional value
    TINT carryBit = 0;

    // do the adding and return the result
    const std::shared_ptr<CKeySet>& keySetPointer = a.GetKeySet();
    const CKeySet& keySet = *keySetPointer;
    CSuperInt<NUMBITS, false> result(keySetPointer);
    for (size_t i = 0; i < NUMBITS; ++i)
        result.GetBit(i) = FullAdder(a.GetBit(i), b.GetBit(i), carryBit, keySet);
    return result;
}

//=================================================================================
template <size_t NUMBITS>
CSuperInt<NUMBITS, false> operator - (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    CSuperInt<NUMBITS, false> negativeB(b);
    negativeB.Negate();
    return a + negativeB;
}

//=================================================================================
template <size_t NUMBITS>
CSuperInt<NUMBITS, false> operator * (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    // do multiplication like this:
    // https://en.wikipedia.org/wiki/Binary_multiplier#Multiplication_basics
    const std::shared_ptr<CKeySet>& keySetPointer = a.GetKeySet();
    const CKeySet& keySet = *keySetPointer;
    CSuperInt<NUMBITS, false> result(keySetPointer);
    for (size_t i = 0; i < NUMBITS; ++i)
    {
        CSuperInt<NUMBITS, false> row = b;
        for (TINT &v : row.GetBits())
            v = AND(v, a.GetBit(i), keySet);

        row.ShiftLeft(i);
        result = result + row;
    }
    return result;
}

//=================================================================================
template <size_t NUMBITS>
CSuperInt<NUMBITS, false> operator / (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    CSuperInt<NUMBITS, false> Q(a.GetKeySet());
    CSuperInt<NUMBITS, false> R(a.GetKeySet());

    Divide(a, b, Q, R);
    return Q;
}

//=================================================================================
template <size_t NUMBITS>
CSuperInt<NUMBITS, false> operator % (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    CSuperInt<NUMBITS, false> Q(a.GetKeySet());
    CSuperInt<NUMBITS, false> R(a.GetKeySet());

    Divide(a, b, Q, R);
    return R;
}

//=================================================================================
template <size_t NUMBITS>
TINT operator < (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    CSuperInt<NUMBITS, false> result(a.GetKeySet());
    result = a - b;
    return result.IsNegative();
}

//=================================================================================
template <size_t NUMBITS>
TINT operator <= (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    return NOT(a > B, *a.GetKeySet());
}

//=================================================================================
template <size_t NUMBITS>
TINT operator > (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    CSuperInt<NUMBITS> result(a.GetKeySet());
    result = b - a;
    return result.IsNegative();
}

//=================================================================================
template <size_t NUMBITS>
TINT operator >= (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    return NOT(a < b, *a.GetKeySet());
}

//=================================================================================
template <size_t NUMBITS>
TINT operator == (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    const CKeySet& keySet = *a.GetKeySet();
    TINT ANotLtB = NOT(a < b, keySet);
    TINT BNotLtA = NOT(b < a, keySet);
    return AND(ANotLtB, BNotLtA, keySet);
}

//=================================================================================
template <size_t NUMBITS>
TINT operator != (const CSuperInt<NUMBITS, false> &a, const CSuperInt<NUMBITS, false> &b)
{
    const CKeySet& keySet = *a.GetKeySet();
    TINT ALtB = a < b;
    TINT BLtA = b < a;
    return OR(ALtB, BLtA, keySet);
}

//=================================================================================
template <size_t NUMBITS>
void Divide (const CSuperInt<NUMBITS, false> &Nin, const CSuperInt<NUMBITS, false> &Din, CSuperInt<NUMBITS, false> &Q, CSuperInt<NUMBITS, false> &R)
{
    // Unsigned integer division algorithm from here: 
    // https://en.wikipedia.org/wiki/Division_algorithm#Integer_division_.28unsigned.29_with_remainder
    // making it work for signed integers here:
    // http://www.cs.utah.edu/~rajeev/cs3810/slides/3810-08.pdf

    Q.SetInt(0);
    R.SetInt(0);

    // Make N and D positive, making sure to remember what their sign used to be
    CSuperInt<NUMBITS, false> N(Nin);
    CSuperInt<NUMBITS, false> D(Din);
    TINT NWasNegative = N.IsNegative();
    TINT DWasNegative = D.IsNegative();
    N.Abs();
    D.Abs();

    const CKeySet& keySet = *N.GetKeySet();
    for (size_t index = NUMBITS; index > 0; --index)
    {
        size_t i = index - 1;
        R.ShiftLeft(1);
        R.GetBit(0) = N.GetBit(i);

        // if R >= D
        // R = R - D
        // Q[i] = 1
        {
            // if R >= D
            TINT RgteD = R >= D;

            // R = R - D
            CSuperInt<NUMBITS, false> branchlessMultiplier(N.GetKeySet());
            branchlessMultiplier.GetBit(0) = RgteD;
            CSuperInt<NUMBITS, false> subtractD(D);
            subtractD = D * branchlessMultiplier;
            R = R - subtractD;

            // Q[i] = 1
            Q.GetBit(i) = RgteD;
        }
    }

    // Make dividedend and remainder have same sign
    TINT RIsNegative = R.IsNegative();
    TINT NRNegativeMismatch = XOR(NWasNegative, RIsNegative, keySet);
    R.NegateConditional(NRNegativeMismatch);

    // Make quotient negative if signs disagree
    TINT quotientNegative = XOR(NWasNegative, DWasNegative, keySet);
    Q.NegateConditional(quotientNegative);
}
