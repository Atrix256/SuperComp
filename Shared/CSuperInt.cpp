#include "CSuperInt.h"
#include "Shared.h"
#include <algorithm>

TINT CSuperInt::s_zeroBit = 0;

//=================================================================================
template <typename T>
static T XOR(T A, T B)
{
    return A + B;
}

//=================================================================================
template <typename T>
static T AND(T A, T B)
{
    return A * B;
}

//=================================================================================
template <typename T>
static T FullAdder (T A, T B, T &carryBit)
{
    // homomorphically add the encrypted bits A and B
    // return the single bit sum, and put the carry bit into carryBit
    // From http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder
    T sumBit = XOR(XOR(A, B), carryBit);
    carryBit = XOR(AND(A, B), AND(carryBit, XOR(A, B)));
    return sumBit;
}

//=================================================================================
CSuperInt::CSuperInt (const TINT &keysLCM) : m_keysLCM(keysLCM)
{

}

//=================================================================================
CSuperInt::CSuperInt (const std::vector<TINT> &keys)
{
    SetKeysLCM(keys);
}

//=================================================================================
CSuperInt::CSuperInt (
    std::vector<TINT>::const_iterator &bitsBegin,
    std::vector<TINT>::const_iterator &bitsEnd,
    const std::vector<TINT> keys
) {
    size_t size = bitsEnd - bitsBegin;
    m_bits.resize(size);

    for (size_t i = 0; i < size; ++i)
        m_bits[i] = bitsBegin[i];

    SetKeysLCM(keys);
}

//=================================================================================
CSuperInt CSuperInt::operator + (const CSuperInt &other) const
{
    // we initialize the carry bit to 0, not a superpositional value
    TINT carryBit = 0;

    // do the adding and return the result
    CSuperInt result(m_keysLCM);
    size_t biggerSize = std::max(m_bits.size(), other.m_bits.size());
    result.m_bits.resize(biggerSize + 1);
    for (size_t i = 0; i < biggerSize; ++i)
    {
        const TINT &a = GetBit(i);
        const TINT &b = other.GetBit(i);
        result.m_bits[i] = FullAdder(a, b, carryBit) % m_keysLCM;
        carryBit = carryBit % m_keysLCM;
    }
    *(result.m_bits.rbegin()) = carryBit;

    return result;
}

//=================================================================================
void CSuperInt::SetKeysLCM (const std::vector<TINT> keys)
{
    m_keysLCM = 1;
    for (const TINT& v : keys)
        m_keysLCM *= v;
}

//=================================================================================
const TINT& CSuperInt::GetBit (size_t i) const
{
    if (i < m_bits.size())
        return m_bits[i];

    return s_zeroBit;
}

/*
TODO:
* assert that the m_keysLCM are the same value when doing math against multiple CSuperInts?
 * could also make it a static of CSuperInt perhaps, but that isn't so great.
*/