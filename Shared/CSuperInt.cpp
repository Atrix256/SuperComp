//=================================================================================
//
//  CSuperInt
//
//  A superpositional integer, made up of a collection of superpositional bits
//
//=================================================================================

#include "CSuperInt.h"
#include "Shared.h"
#include <algorithm>
#include "CKeySet.h"

TINT CSuperInt::s_zeroBit = 0;

//=================================================================================
static TINT XOR (const TINT &A, const TINT &B)
{
    return A + B;
}

//=================================================================================
static TINT AND (const TINT &A, const TINT &B)
{
    return A * B;
}

//=================================================================================
static TINT NOT (const TINT &A)
{
    return XOR(A, TINT(1));
}

//=================================================================================
static TINT FullAdder (const TINT &A, const TINT &B, TINT &carryBit)
{
    // homomorphically add the encrypted bits A and B
    // return the single bit sum, and put the carry bit into carryBit
    // From http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder
    TINT sumBit = XOR(XOR(A, B), carryBit);
    carryBit = XOR(AND(A, B), AND(carryBit, XOR(A, B)));
    return sumBit;
}

//=================================================================================
CSuperInt::CSuperInt(const std::shared_ptr<CKeySet> &keySet) : m_keySet(keySet)
{

}

//=================================================================================
CSuperInt::CSuperInt (
    std::vector<TINT>::const_iterator &bitsBegin,
    std::vector<TINT>::const_iterator &bitsEnd,
    const std::shared_ptr<CKeySet> &keySet
) : m_keySet(keySet) {
    size_t size = bitsEnd - bitsBegin;
    m_bits.resize(size);

    for (size_t i = 0; i < size; ++i)
        m_bits[i] = bitsBegin[i];
}

//=================================================================================
size_t CSuperInt::Decode (const TINT& key) const
{
    // decode result for this specific key index
    const std::vector<TINT> &bits = GetBits();
    size_t result = 0;
    for (size_t i = 0, c = bits.size(); i < c; ++i)
    {
        TINT value = (bits[i] % key) % 2;
        result = result | (value.convert_to<size_t>() << i);
    }
    return result;
}

//=================================================================================
size_t CSuperInt::MaxError (const TINT& key) const
{
    // decode result for this specific key
    const std::vector<TINT> &bits = GetBits();
    float maxError = 0.0f;
    for (size_t i = 0, c = bits.size(); i < c; ++i)
    {
        TINT residue = bits[i] % key;
        float error = residue.convert_to<float>() / key.convert_to<float>();
        if (error > maxError)
            maxError = error;
    }
    return size_t(maxError * 100.0f);
}

//=================================================================================
CSuperInt CSuperInt::operator + (const CSuperInt &other) const
{
    // handle the case of zero bits in one of the values
    if (m_bits.size() == 0)
        return other;
    else if (other.m_bits.size() == 0)
        return *this;

    // we initialize the carry bit to 0, not a superpositional value
    TINT carryBit = 0;

    // do the adding and return the result
    CSuperInt result(m_keySet);
    size_t biggerSize = std::max(m_bits.size(), other.m_bits.size());
    result.m_bits.resize(biggerSize + 1);
    for (size_t i = 0; i < biggerSize; ++i)
    {
        const TINT &a = GetBit(i);
        const TINT &b = other.GetBit(i);
        result.m_bits[i] = FullAdder(a, b, carryBit) % m_keySet->GetKeysLCM();
        carryBit = carryBit % m_keySet->GetKeysLCM();
    }
    *(result.m_bits.rbegin()) = carryBit;

    return result;
}

//=================================================================================
CSuperInt CSuperInt::operator * (const CSuperInt &other) const
{
    // do multiplication like this:
    // https://en.wikipedia.org/wiki/Binary_multiplier#Multiplication_basics
    CSuperInt result(m_keySet);
    for (size_t i = 0, c = m_bits.size(); i < c; ++i)
    {
        CSuperInt row = other;
        for (TINT &v : row.m_bits)
            v = AND(v, m_bits[i]);

        row.ShiftLeft(i);
        result = result + row;
    }
    return result;
}

//=================================================================================
void CSuperInt::ShiftLeft (size_t amount)
{
    while (amount > 0)
    {
        m_bits.insert(m_bits.begin(), 0);
        amount--;
    }
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
* assert that the key set pointer is the same value when doing math against multiple CSuperInts?
 * could also make it a static of CSuperInt perhaps, but that isn't so great.
* make a define to turn on or off checking of each basic operation (AND / XOR) for each key.
 * assert if there's a failure, specifying which key(s) hit the problems?
 * so the minkey can be increased
*/