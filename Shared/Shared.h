#include <vector>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include "CSuperInt.h"

#define NOMINMAX
#include <Windows.h> // for IsDebuggerPresent()

#define ExitCode_(x) {if (IsDebuggerPresent()) {WaitForEnter();} return x;}

void WaitForEnter ();

//=================================================================================
template <typename T>
bool WriteBitsKeys (const char *fileName, const std::vector<T> &superPositionedBits, const std::vector<T> &keys)
{
    std::ofstream file;
    file.open(fileName, std::ios::out | std::ios::trunc);

    if (!file.is_open())
        return false;

    // write the number of bits
    file << superPositionedBits.size() << "\n";

    // write the bits
    for (const T &v : superPositionedBits)
        file << v << "\n";

    // write the keys
    for (const T &v : keys)
        file << v << "\n";

    file.close();
    return true;
}

//=================================================================================
template <typename T>
bool ReadBitsKeys (const char *fileName, std::vector<T> &superPositionedBits, std::vector<T> &keys)
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
        superPositionedBits.resize(numBits);
        keys.resize(1 << numBits);

        // read the bits
        for (T &v : superPositionedBits)
            file >> v;

        // read the keys
        for (T &v : keys)
            file >> v;
    }
    while (0);

    bool ret = !file.fail();
    file.close();
    return ret;
}

//=================================================================================
template <typename T>
void ReportBitsAndError (const std::vector<T> &superPositionedBits, const std::vector<T> &keys)
{
    for (size_t i = 0, c = superPositionedBits.size(); i < c; ++i)
    {
        std::cout << "--Bit " << i << "--\n" << superPositionedBits[i] << "\n";

        float maxError = 0.0f;
        for (int keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
        {
            float error = 100.0f * float(superPositionedBits[i] % keys[keyIndex]) / float(keys[keyIndex]);
            if (error > maxError)
                maxError = error;
        }
        std::cout << "Highest Error = " << std::setprecision(2) << maxError << "%\n\n";
    }
}

//=================================================================================
template <typename L>
bool PermuteResults2Inputs (const CSuperInt &A, const CSuperInt &B, const L& lambda)
{
    bool ret = true;
    size_t numABits = A.NumBits();
    for (size_t b = 0, bc = (1 << B.NumBits()) - 1; b <= bc; ++b)
    {
        for (size_t a = 0, ac = (1 << A.NumBits()) - 1; a <= ac; ++a)
        {
            // get the index of our key for this specific set of inputs
            size_t keyIndex = (b << numABits) | a;

            // call the lambda!
            ret = ret && lambda(a, b, keyIndex);
        }
    }
    return ret;
}
