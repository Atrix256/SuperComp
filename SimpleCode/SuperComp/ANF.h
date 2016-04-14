//=================================================================================
//
//  ANF.h
//
//  Converts a function (lookup table) to ANF terms
//
//=================================================================================

#pragma once

#include <array>
#include <vector>

//=================================================================================
// Internal Functionality
//=================================================================================

template <size_t NUM_INPUT_BITS>
bool LookupEntryPassesMask (size_t entry, size_t mask)
{
    for (size_t i = 0; i < NUM_INPUT_BITS; ++i)
    {
        const size_t bitMask = 1 << i;
        const bool allowOnes = (mask & bitMask) != 0;
        const bool bitPassesMask = allowOnes || (entry & bitMask) == 0;
        if (!bitPassesMask)
            return false;
    }
    return true;
}

template <size_t NUM_INPUT_BITS>
bool ANFHasTerm (const std::array<size_t, 1 << NUM_INPUT_BITS> &lookupTable, size_t outputBitIndex, size_t termMask)
{
    const size_t c_inputValueCount = 1 << NUM_INPUT_BITS;

    int onesCount = 0;
    for (size_t i = 0; i < c_inputValueCount; ++i)
    {
        if (LookupEntryPassesMask<NUM_INPUT_BITS>(i, termMask) && ((lookupTable[i] >> outputBitIndex) & 1) != 0)
            onesCount++;
    }

    return (onesCount & 1) != 0;
}

template <size_t NUM_INPUT_BITS>
void MakeANFTruthTable (const std::array<size_t, 1 << NUM_INPUT_BITS> &lookupTable, std::vector<size_t>& terms, size_t outputBitIndex)
{
    const size_t c_inputValueCount = 1 << NUM_INPUT_BITS;

    // find each ANF term
    for (size_t termMask = 0; termMask < c_inputValueCount; ++termMask)
    {
        if (ANFHasTerm<NUM_INPUT_BITS>(lookupTable, outputBitIndex, termMask))
            terms.push_back(termMask);
    }
}

//=================================================================================
// Entry Point
//=================================================================================

template <size_t NUM_INPUT_BITS, size_t NUM_OUTPUT_BITS, typename LAMBDA>
std::array<std::vector<size_t>, NUM_OUTPUT_BITS> MakeANFTerms (const LAMBDA& lambda)
{
    // make lookup table
    const size_t c_outputValueMask = (1 << NUM_OUTPUT_BITS) - 1;
    const size_t c_inputValueCount = 1 << NUM_INPUT_BITS;
    std::array<size_t, c_inputValueCount> lookupTable;
    for (size_t inputValue = 0; inputValue < c_inputValueCount; ++inputValue)
        lookupTable[inputValue] = lambda(inputValue, NUM_INPUT_BITS) & c_outputValueMask;

    // make the anf for each truth table (each output bit of the lookup table)
    std::array<std::vector<size_t>, NUM_OUTPUT_BITS> terms;
    for (size_t outputBitIndex = 0; outputBitIndex < NUM_OUTPUT_BITS; ++outputBitIndex)
        MakeANFTruthTable<NUM_INPUT_BITS>(lookupTable, terms[outputBitIndex], outputBitIndex);

    // return the terms we calculated
    return terms;
}