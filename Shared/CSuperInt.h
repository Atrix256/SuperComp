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
#include <memory>

//typedef int64_t TINT;
//typedef boost::multiprecision::int128_t TINT;
typedef boost::multiprecision::cpp_int TINT;

//=================================================================================
class CKeySet;

//=================================================================================
class CSuperInt
{
public:
    // initialize to zero
    CSuperInt (const std::shared_ptr<CKeySet> &keySet);

    // initialize to a range of a vector of superpositional bits, and keys to
    // calculate LCM
    CSuperInt (
        std::vector<TINT>::const_iterator &bitsBegin,
        std::vector<TINT>::const_iterator &bitsEnd,
        const std::shared_ptr<CKeySet> &keySet
    );

    // Encoding / Decoding
    size_t Decode (const TINT& key) const;
    size_t MaxError (const TINT& key) const;

    // Math Operators
    CSuperInt operator+ (const CSuperInt &other) const;
    CSuperInt operator* (const CSuperInt &other) const;

    void ShiftLeft (size_t amount);

    // Internals Access
    const TINT& GetBit (size_t i) const;

    size_t NumBits() const {
        return m_bits.size();
    }

    const std::vector<TINT>& GetBits () const {
        return m_bits;
    }

    const CKeySet &GetKeySet () const {
        return *m_keySet;
    }

private:

    std::vector<TINT>           m_bits;     // the superpositional bits
    std::shared_ptr<CKeySet>    m_keySet;   // the keys to decode the bits

    static TINT                 s_zeroBit;
};