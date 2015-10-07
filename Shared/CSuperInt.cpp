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
#include "Settings.h"

TINT CSuperInt::s_zeroBit = 0;

//=================================================================================
static TINT XOR (const TINT &A, const TINT &B, const CKeySet &keySet)
{
    #if CHECK_BITOPS()
        int residueA = TINT((A % CHECK_BITOPS_KEY())).convert_to<int>();
        int residueB = TINT((B % CHECK_BITOPS_KEY())).convert_to<int>();
        bool decodedA = !!(residueA % 2);
        bool decodedB = !!(residueB % 2);
        bool desiredResult = decodedA ^ decodedB;
    #endif

    TINT result = A + B;
    keySet.ReduceValue(result);

    #if CHECK_BITOPS()
        int residueResult = TINT((result % CHECK_BITOPS_KEY())).convert_to<int>();
        bool decodedResult = !!(residueResult % 2);
        #if CHECK_BITOPS_LOG()
            std::cout << s_bitOpsLogIndent << "XOR: " << residueA << " + " << residueB << " = " << residueResult << "\n";
        #endif
        Assert_(desiredResult == decodedResult);
    #endif

    return result;
}

//=================================================================================
static TINT AND(const TINT &A, const TINT &B, const CKeySet &keySet)
{
    #if CHECK_BITOPS()
        int residueA = TINT((A % CHECK_BITOPS_KEY())).convert_to<int>();
        int residueB = TINT((B % CHECK_BITOPS_KEY())).convert_to<int>();
        bool decodedA = !!(residueA % 2);
        bool decodedB = !!(residueB % 2);
        bool desiredResult = decodedA && decodedB;
    #endif

    TINT result = A * B;
    keySet.ReduceValue(result);

    #if CHECK_BITOPS()
        int residueResult = TINT((result % CHECK_BITOPS_KEY())).convert_to<int>();
        bool decodedResult = !!(residueResult % 2);
        #if CHECK_BITOPS_LOG()
            std::cout << s_bitOpsLogIndent<< "AND: " << residueA << " * " << residueB << " = " << residueResult << "\n";
        #endif
        Assert_(desiredResult == decodedResult);
    #endif

    return result;
}

//=================================================================================
static TINT NOT (const TINT &A, const CKeySet &keySet)
{
    CHECK_BITOPS_LOG_INDENT("Not");
    return XOR(A, TINT(1), keySet);
}

//=================================================================================
static TINT FullAdder(const TINT &A, const TINT &B, TINT &carryBit, const CKeySet &keySet)
{
    CHECK_BITOPS_LOG_INDENT("FullAdder");
    // homomorphically add the encrypted bits A and B
    // return the single bit sum, and put the carry bit into carryBit
    // From http://en.wikipedia.org/w/index.php?title=Adder_(electronics)&oldid=381607326#Full_adder
    TINT sumBit;

    {
        CHECK_BITOPS_LOG_INDENT("SumBit");
        sumBit = XOR(XOR(A, B, keySet), carryBit, keySet);
        #if CHECK_BITOPS_LOG()
            std::cout << s_bitOpsLogIndent << "SumBit = " << TINT(sumBit % CHECK_BITOPS_KEY()).convert_to<int>() << "\n";
        #endif
    }
    {
        CHECK_BITOPS_LOG_INDENT("CarryBit");
        carryBit = XOR(AND(A, B, keySet), AND(carryBit, XOR(A, B, keySet), keySet), keySet);
        #if CHECK_BITOPS_LOG()
            std::cout << s_bitOpsLogIndent << "CarryBit = " << TINT(carryBit % CHECK_BITOPS_KEY()).convert_to<int>() << "\n";
        #endif
    }
    return sumBit;
}

//=================================================================================
CSuperInt::CSuperInt (const std::shared_ptr<CKeySet> &keySet) : m_keySet(keySet)
{

}

//=================================================================================
CSuperInt::CSuperInt (int value, const std::shared_ptr<CKeySet> & keySet) : m_keySet(keySet)
{
    // we want the raw bit representation
    unsigned int uvalue = (unsigned int)value;

    // make the non superpositional binary representation of this value
    while (uvalue)
    {
        m_bits.push_back((uvalue & 1) ? 1 : 0);
        uvalue = uvalue >> 1;
    }
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
    CHECK_BITOPS_LOG_INDENT("Add");

    #if CHECK_BITOPS_LOG()
        std::cout << s_bitOpsLogIndent << "[ ";
        for (const TINT &v : m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "] + [ ";
        for (const TINT &v : other.m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "]\n";
    #endif

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
        #if CHECK_BITOPS_LOG()
            char scopeName[32];
            sprintf(scopeName, "bit %i", i);
            CHECK_BITOPS_LOG_INDENT(scopeName);
        #endif
        const TINT &a = GetBit(i);
        const TINT &b = other.GetBit(i);
        result.m_bits[i] = FullAdder(a, b, carryBit, *m_keySet);
    }
    *(result.m_bits.rbegin()) = carryBit;

    #if CHECK_BITOPS_LOG()
        std::cout << s_bitOpsLogIndent << "Results = [ ";
        for (const TINT &v : result.m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "]\n";
    #endif

    return result;
}

//=================================================================================
CSuperInt CSuperInt::operator * (const CSuperInt &other) const
{
    CHECK_BITOPS_LOG_INDENT("Multiply");

    #if CHECK_BITOPS_LOG()
        std::cout << s_bitOpsLogIndent << "[ ";
        for (const TINT &v : m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "] * [ ";
        for (const TINT &v : other.m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "]\n";
    #endif

    // do multiplication like this:
    // https://en.wikipedia.org/wiki/Binary_multiplier#Multiplication_basics
    CSuperInt result(m_keySet);
    for (size_t i = 0, c = m_bits.size(); i < c; ++i)
    {
        CSuperInt row = other;
        {
            #if CHECK_BITOPS_LOG()
                char scopeName[32];
                sprintf(scopeName, "Making Row %i", i);
                CHECK_BITOPS_LOG_INDENT(scopeName);
            #endif
            for (TINT &v : row.m_bits)
                v = AND(v, m_bits[i], *m_keySet);
        }

        row.ShiftLeft(i);

        {
            CHECK_BITOPS_LOG_INDENT("AddingRow");
            result = result + row;
        }
    }

    #if CHECK_BITOPS_LOG()
        std::cout << s_bitOpsLogIndent << "Results = [ ";
        for (const TINT &v : result.m_bits)
            std::cout << TINT(v % CHECK_BITOPS_KEY()).convert_to<int>() << " ";
        std::cout << "]\n";
    #endif

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
