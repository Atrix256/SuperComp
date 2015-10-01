//=================================================================================
//
//  CSuperInt
//
//  A superpositional integer, made up of a collection of superpositional bits
//
//=================================================================================

#pragma once

#include <vector>
#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::cpp_int TINT;

//=================================================================================
class CSuperInt
{
public:
    // initialize to zero with LCM
    CSuperInt(const TINT &keysLCM);

    // initialize to zero with keys to calculate LCM
    CSuperInt (const std::vector<TINT> &keys);

    // initialize to a range of a vector of superpositional bits, and keys to
    // calculate LCM
    CSuperInt (
        std::vector<TINT>::const_iterator &bitsBegin,
        std::vector<TINT>::const_iterator &bitsEnd,
        const std::vector<TINT> keys
    );

    // Math Operators
    CSuperInt operator+ (const CSuperInt &other) const;

    // Other
    const TINT& GetBit (size_t i) const;

    size_t NumBits() const {
        return m_bits.size();
    }

    // TEMP!
    const std::vector<TINT>& GetBits() const {
        return m_bits;
    }

private:
    void SetKeysLCM(const std::vector<TINT> keys);

    std::vector<TINT>   m_bits;     // the superpositional bits
    TINT                m_keysLCM;  // the LCM of the keys, to keep numbers smaller


    static TINT         s_zeroBit;
};