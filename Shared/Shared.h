#include <vector>
#include <stdint.h>
#include <iostream>
#include <algorithm>
#include "CSuperInt.h"

#define NOMINMAX
#include <Windows.h> // for IsDebuggerPresent() and DebugBreak()

#define ExitCode_(x) {if (IsDebuggerPresent()) {WaitForEnter();} return x;}

#define Assert_(x) if (!(x)) { printf("Assert Failed : " #x); DebugBreak(); }

void WaitForEnter ();

void ReportBitsAndError(const CSuperInt &superInt);

//=================================================================================
template <typename L>
bool PermuteResults2Inputs (const CSuperInt &A, const CSuperInt &B, const CSuperInt &superResult, const std::vector<TINT> &keys, const L& lambda)
{
    bool ret = true;
    size_t numABits = A.NumBits();
    for (size_t b = 0, bc = (1 << B.NumBits()) - 1; b <= bc; ++b)
    {
        for (size_t a = 0, ac = (1 << A.NumBits()) - 1; a <= ac; ++a)
        {
            // get the index of our key for this specific set of inputs
            size_t keyIndex = (b << numABits) | a;

            // decode result for this specific key
            size_t result = superResult.Decode(keys[keyIndex]);

            // call the lambda!
            ret = ret && lambda(a, b, keyIndex, keys[keyIndex], result);
        }
    }
    return ret;
}
