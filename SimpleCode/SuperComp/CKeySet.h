//=================================================================================
//
//  CKeySet
//
//  Holds a set of keys, as well as their LCM. Used by CSuperInt
//
//=================================================================================

#pragma once

#include "TINT.h"
#include <functional>

class CKeySet
{
public:
    CKeySet() : m_reduce(false) {}

    bool Read (const char *fileName);
    bool Write (const char *fileName) const;

    void CalculateCached (int numBits, const TINT& minKey, const std::function<void (uint8_t percent)>& progressCallback = [] (uint8_t percent) {} );
    void Calculate (int numBits, const TINT& minKey, const std::function<void (uint8_t percent)>& progressCallback = [] (uint8_t percent) {} );

    const std::vector<TINT> &GetSuperPositionedBits () const { return m_superPositionedBits; }
    const std::vector<TINT> &GetKeys () const { return m_keys; }

    void ReduceValue (TINT& v) const { /*if (m_reduce) { ReduceValueExplicit(v); }*/ }

    void ReduceValueExplicit (TINT& v) const { v = v % m_keysLCM; }

    float GetComplexityIndex () const;

private:
    void MakeKey (size_t keyIndex);
    bool KeyIsCoprime (size_t keyIndex, TINT& value) const;
    TINT CalculateBit (size_t bitIndex, const std::vector<TINT> &coefficients) const;

private:
    std::vector<TINT>   m_superPositionedBits;
    std::vector<TINT>   m_keys;
    TINT                m_keysLCM;
    bool                m_reduce;
};