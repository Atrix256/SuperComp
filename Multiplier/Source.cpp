#include "Shared/Shared.h"
#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::cpp_int TINT;

//=================================================================================
void DoMultiply (const std::vector<TINT>& A, const std::vector<TINT>& B, std::vector<TINT>& results)
{
    results.resize(A.size() + B.size());
    std::fill(results.begin(), results.end(), 0);
    // TODO: make this work!
}

//=================================================================================
int main(int argc, char **argv)
{
    // verify parameters
    std::cout << "--Multiplier--\n\nUses an input key file to multiply bits superpositionally and show results.\nRequires an even number of bits.\n\n";
    if (argc < 2)
    {
        std::cout << "Usage: <inputFile>\n";;
        ExitCode_(1);
    }

    // read in the bits and keys if we can
    std::vector<TINT> superPositionedBits;
    std::vector<TINT> keys;
    std::cout << "Loading " << argv[1] << "\n";
    if (ReadBitsKeys(argv[1], superPositionedBits, keys))
    {
        // we need an even number of bits to do adding!
        if (superPositionedBits.size() % 2 == 0)
        {
            std::cout << "file loaded!\n\n";
            const size_t numInputBits = superPositionedBits.size() / 2;
            std::cout << "Multiplying " << numInputBits << " bits and " << numInputBits << " bits to get a " << (numInputBits * 2) << " bit result\n";
            std::cout << "Note that you could truncate the high bits if you wanted to\n";

            // Calculate the LCM of our keys so we can make results mod this LCM and keep numbers smaller
            TINT keysLCM = 1;
            for (const TINT& v : keys)
                keysLCM *= v;

            // do our multi bit multiplication! N bits * N bits = up to N*2 bit result.
            // Note that we can initialize our carry bit to the value 0.  we don't
            // need to initialize it to a superpositioned bit value!
            TINT carryBit = 0;
            std::vector<TINT> a, b, results;
            a.resize(numInputBits);
            b.resize(numInputBits);
            for (size_t i = 0; i < numInputBits; ++i)
            {
                a[i] = superPositionedBits[i];
                b[i] = superPositionedBits[i + numInputBits];
            }
            DoMultiply(a, b, results);

            // show superpositional result and error (max and % of each key)
            std::cout << "\nSuperpositional Multiplier Result:\n\n";
            for (size_t i = 0, c = results.size(); i < c; ++i)
            {
                std::cout << "--Bit " << i << "--\n\n" << results[i] << "\n\n";

                float maxError = 0.0f;
                for (int keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
                {
                    float error = 100.0f * float(results[i] % keys[keyIndex]) / float(keys[keyIndex]);
                    if (error > maxError)
                        maxError = error;
                }
                std::cout << "Highest Error = " << std::setprecision(2) << maxError << "%\n\n";
            }

            // Permute through results, make sure truth tables add up
            printf("Result Verification:\n");
            const size_t maxValue = (1 << numInputBits) - 1;
            for (size_t a = 0; a <= maxValue; ++a)
            {
                for (size_t b = 0; b <= maxValue; ++b)
                {
                    // get the index of our key for this specific set of inputs
                    size_t keyIndex = (a << numInputBits) | b;

                    // decode result for this specific key index
                    size_t result = 0;
                    for (size_t i = 0, c = results.size(); i < c; ++i)
                        result = result | (size_t((results[i] % keys[keyIndex]) % 2) << i);

                    // show the result
                    std::cout << "  [" << keyIndex << "]  " << a << " * " << b << " = " << result << "\n";

                    // verify the result
                    if (result != a * b)
                    {
                        std::cout << "ERROR! incorrect value detected!";
                        // TODO: assert or something?
                        ExitCode_(1);
                    }
                }
            }
        }
        else
        {
            std::cout << "file did not have an even number of bits!\n";
        }
    }
    else
    {
        std::cout << "could not load " << argv[1] << "\n";
        ExitCode_(1);
    }

    ExitCode_(0);
}

/*
TODO:
* currently re-working adder, then come back to this
* make this work
* https://en.wikipedia.org/wiki/Binary_multiplier
* put the add / multiply code in shared
 * eventually will abstract a type with operator overloads
* remove adder / multiplier even bit count restriction
* instead of calling it "TINT", could call it "CSuperBit" or something.

! NEXT:
 * divide / modulus: http://courses.cs.vt.edu/~cs1104/BuildingBlocks/divide.030.html
 * something more time consuming or impressive.  Like shor's algorithm or who knows what else.  CORDIC math perhaps to calculate sine since it's branchless?
 * can we do bootstrapping with superpositional keys? do we have to do one layer deeper of HE or something?
*/