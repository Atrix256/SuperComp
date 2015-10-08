//=================================================================================
//
//  CKeySet
//
//  Holds a set of keys, as well as their LCM. Used by CSuperInt
//
//=================================================================================

#pragma once

#include "CSuperInt.h"

class CKeySet
{
public:
    CKeySet() : m_reduce(false) {}

    bool Read (const char *fileName);
    bool Write (const char *fileName) const;

    void Calculate (int numBits, size_t minKey);

    const std::vector<TINT> &GetSuperPositionedBits () const { return m_superPositionedBits; }
    const std::vector<TINT> &GetKeys () const { return m_keys; }

    void ReduceValue (TINT& v) const { if (m_reduce) { v = v % m_keysLCM; } }

private:
    void MakeKey (size_t keyIndex, size_t minKey);
    bool KeyIsCoprime (size_t keyIndex, TINT& value) const;
    TINT CalculateBit (size_t bitIndex, const std::vector<TINT> &coefficients) const;

private:
    std::vector<TINT>   m_superPositionedBits;
    std::vector<TINT>   m_keys;
    TINT                m_keysLCM;
    bool                m_reduce;
};