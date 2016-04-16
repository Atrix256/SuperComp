//=================================================================================
//
//  CKeySet
//
//  Holds a set of keys, as well as their LCM. Used by CSuperInt
//
//=================================================================================

#include "CKeySet.h"
#include <fstream>
#include <sstream>

//=================================================================================
static TINT ExtendedEuclidianAlgorithm (TINT smaller, TINT larger, TINT &s, TINT &t)
{
    // make sure A <= B before starting
    bool swapped = false;
    if (larger < smaller)
    {
        swapped = true;
        std::swap(smaller, larger);
    }

    // set up our storage for the loop.  We only need the last two values so will
    // just use a 2 entry circular buffer for each data item
    std::array<TINT, 2> remainders = { larger, smaller };
    std::array<TINT, 2> ss = { 1, 0 };
    std::array<TINT, 2> ts = { 0, 1 };
    size_t indexNeg2 = 0;
    size_t indexNeg1 = 1;

    // loop
    while (1)
    {
        // calculate our new quotient and remainder
        TINT newQuotient = remainders[indexNeg2] / remainders[indexNeg1];
        TINT newRemainder = remainders[indexNeg2] - newQuotient * remainders[indexNeg1];

        // if our remainder is zero we are done.
        if (newRemainder == 0)
        {
            // return our s and t values as well as the quotient as the GCD
            s = ss[indexNeg1];
            t = ts[indexNeg1];
            if (swapped)
                std::swap(s, t);

            // if t < 0, add the modulus divisor to it, to make it positive
            if (t < 0)
                t += smaller;
            return remainders[indexNeg1];
        }

        // calculate this round's s and t
        TINT newS = ss[indexNeg2] - newQuotient * ss[indexNeg1];
        TINT newT = ts[indexNeg2] - newQuotient * ts[indexNeg1];

        // store our values for the next iteration
        remainders[indexNeg2] = newRemainder;
        ss[indexNeg2] = newS;
        ts[indexNeg2] = newT;

        // move to the next iteration
        std::swap(indexNeg1, indexNeg2);
    }
}

//=================================================================================
bool CKeySet::Read (const char *fileName)
{
    std::ifstream file;
    file.open(fileName);

    if (!file.is_open())
        return false;

    do
    {
        // read the number of bits
        size_t numBits;
        file >> numBits;
        if (file.fail())
            break;

        // resize the data arrays
        m_superPositionedBits.resize(numBits);
        m_keys.resize(1 << numBits);

        // read the bits
        for (TINT &v : m_superPositionedBits)
            file >> v;

        // read the keys
        for (TINT &v : m_keys)
            file >> v;
    }
    while (0);

    bool ret = !file.fail();
    file.close();

    // calculate keys LCM
    m_keysLCM = 1;
    for (const TINT& v : m_keys)
        m_keysLCM *= v;

    // we will reduce numbers since we have an LCM
    m_reduce = true;

    return ret;
}

//=================================================================================
bool CKeySet::Write (const char *fileName) const
{
    std::ofstream file;
    file.open(fileName, std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return false;

    // write the number of bits
    file << m_superPositionedBits.size() << "\n";

    // write the bits
    for (const TINT &v : m_superPositionedBits)
        file << v << "\n";

    // write the keys
    for (const TINT &v : m_keys)
        file << v << "\n";

    file.close();
    return true;
}

//=================================================================================
void CKeySet::CalculateCached (int numBits, const TINT& minKey, const std::function<void (uint8_t percent)>& progressCallback)
{
    // if we can read the keys from the cache do that and return
    std::stringstream fileName;
    fileName << "keys_" << numBits << "_" << minKey << ".txt";
    if (Read(fileName.str().c_str()))
    {
        progressCallback(100);
        return;
    }

    // Do the calculation
    Calculate(numBits, minKey, progressCallback);

    // write these keys to the cache
    Write(fileName.str().c_str());
}

//=================================================================================
void CKeySet::Calculate (int numBits, const TINT& minKey, const std::function<void (uint8_t percent)>& progressCallback)
{
    // size our arrays
    m_superPositionedBits.resize(size_t(numBits));
    m_keys.resize(size_t(1) << m_superPositionedBits.size());

    // set our keys to co prime numbers that aren't super tiny.  The smallest key
    // determines how much error we can tolerate building up, just like FHE over integers.
    uint8_t lastPercent = 255;
    m_keys[0] = minKey;
    TINT isOdd = m_keys[0] % 2;
    if (isOdd == 0)
        m_keys[0]++;
    m_keysLCM = m_keys[0];
    for (size_t i = 1, c = m_keys.size(); i < c; ++i)
    {
        MakeKey(i);
        m_keysLCM *= m_keys[i];
        uint8_t percent = uint8_t(i * 33 / c);
        if (lastPercent != percent)
        {
            progressCallback(percent);
            lastPercent = percent;
        }
    }

    // calculate our co-efficients for each term, for the chinese remainder theorem.
    // We do it here because it can be re-used for each x value we are solving for
    // with these keys.
    const size_t c_numKeys = m_keys.size();
    std::vector<TINT> coefficients;
    coefficients.resize(c_numKeys);
    fill(coefficients.begin(), coefficients.end(), 1);
    for (size_t i = 0; i < c_numKeys; ++i)
    {
        for (size_t j = 0; j < c_numKeys; ++j)
        {
            if (i != j)
                coefficients[i] *= m_keys[j];
        }
        uint8_t percent = uint8_t(i * 33 / c_numKeys + 33);
        if (lastPercent != percent)
        {
            progressCallback(percent);
            lastPercent = percent;
        }
    }

    // calculate each x value
    for (size_t i = 0, c = m_superPositionedBits.size(); i < c; ++i)
    {
        m_superPositionedBits[i] = CalculateBit(i, coefficients);
        uint8_t percent = uint8_t(i * 33 / c + 66);
        if (lastPercent != percent)
        {
            progressCallback(percent);
            lastPercent = percent;
        }
    }

    progressCallback(100);

    // we will reduce numbers since we have an LCM
    m_reduce = true;
}

//=================================================================================
float CKeySet::GetComplexityIndex () const
{
    // return the log10 of the sum of the keys
    TINT keySum = 0;
    for (const TINT& v : m_keys)
        keySum = keySum + v;
    return float(log10(keySum.convert_to<double>()));
}

//=================================================================================
void CKeySet::MakeKey (size_t keyIndex)
{
    // make sure our keys are odd
    TINT nextNumber = m_keys[keyIndex - 1] + 2;
    while (1)
    {
        if (KeyIsCoprime(keyIndex, nextNumber))
        {
            m_keys[keyIndex] = nextNumber;
            return;
        }

        nextNumber += 2;
    }
}

//=================================================================================
bool CKeySet::KeyIsCoprime (size_t keyIndex, TINT& value) const
{
    for (size_t index = 0; index < keyIndex; ++index)
    {
        TINT s, t;
        TINT gcd = ExtendedEuclidianAlgorithm(value, m_keys[index], s, t);
        if (gcd != 1)
            return false;
    }

    return true;
}

//=================================================================================
TINT CKeySet::CalculateBit (size_t bitIndex, const std::vector<TINT> &coefficients) const
{
    TINT ret = 0;
    const TINT bitMask = 1 << bitIndex;

    // now figure out how much to multiply each coefficient by to make it have the specified modulus residue (remainder)
    for (size_t i = 0, c = m_keys.size(); i < c; ++i)
    {
        // we either want this term to be 0 or 1 mod the key.  if zero, we can multiply by zero, and
        // not add anything into the bit value!
        if ((i & bitMask) == 0)
            continue;

        TINT s, t;
        ExtendedEuclidianAlgorithm(coefficients[i], m_keys[i], s, t);
        ret = (ret + ((coefficients[i] * t) % m_keysLCM)) % m_keysLCM;
    }

    return ret;
}