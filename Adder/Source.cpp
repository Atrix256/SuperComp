#include "Shared/Shared.h"
#include <boost/multiprecision/cpp_int.hpp>

typedef boost::multiprecision::cpp_int TINT;

//=================================================================================
int main(int argc, char **argv)
{
    // read in the bits and keys if we can
    std::vector<TINT> superPositionedBits;
    std::vector<TINT> keys;
    std::cout << "Loading results.txt\n";
    if (ReadBitsKeys("results.txt", superPositionedBits, keys))
    {
        // we need an even number of bits to do adding!
        if (superPositionedBits.size() % 2 == 0)
        {
            std::cout << "file loaded!\n\n";
            const size_t numBitsAdding = superPositionedBits.size() / 2;
            std::cout << "Adding " << numBitsAdding << " bits and " << numBitsAdding << " bits to get a " << numBitsAdding + 1 << " bit result\n";
            std::cout << "Note that you could truncate the high bit if you wanted to\n";

            // Calculate the LCM of our keys so we can make results mod this LCM and keep numbers smaller
            TINT keysLCM = 1;
            for (const TINT& v : keys)
                keysLCM *= v;

            // do our multi bit addition! N bits + N bits = up to N+1 bit result.
            // Note that we can initialize our carry bit to the value 0.  we don't
            // need to initialize it to a superpositioned bit value!
            TINT carryBit = 0;
            std::vector<TINT> adderResults;
            adderResults.resize(numBitsAdding+1);
            for (size_t i = 0; i < numBitsAdding; ++i)
            {
                adderResults[i] = FullAdder(superPositionedBits[i], superPositionedBits[i + numBitsAdding], carryBit) % keysLCM;
                carryBit = carryBit % keysLCM;
            }
            adderResults[numBitsAdding] = carryBit;

            // show superpositional result and error (max and % of each key)
            std::cout << "\nSuperpositional Adder Result:\n\n";
            for (size_t i = 0, c = adderResults.size(); i < c; ++i)
            {
                std::cout << "--Bit " << i << "--\n\n" << adderResults[i] << "\n\n";

                float maxError = 0.0f;
                for (int keyIndex = 0, keyCount = keys.size(); keyIndex < keyCount; ++keyIndex)
                {
                    float error = 100.0f * float(adderResults[i] % keys[keyIndex]) / float(keys[keyIndex]);
                    if (error > maxError)
                        maxError = error;
                }
                std::cout << "Highest Error = " << std::setprecision(2) << maxError << "%\n\n";
            }

            // Permute through results, make sure truth tables add up
            printf("Result Verification:\n");
            const size_t maxValue = (1 << numBitsAdding) - 1;
            for (size_t a = 0; a <= maxValue; ++a)
            {
                for (size_t b = 0; b <= maxValue; ++b)
                {
                    // get the index of our key for this specific set of inputs
                    size_t keyIndex = (a << numBitsAdding) | b;

                    // decode result for this specific key index
                    size_t result = 0;
                    for (size_t i = 0, c = adderResults.size(); i < c; ++i)
                        result = result | (size_t((adderResults[i] % keys[keyIndex]) % 2) << i);

                    // show the result
                    std::cout << "  [" << keyIndex << "]  " << a << " + " << b << " = " << result << "\n";

                    // verify the result
                    if (result != a + b)
                    {
                        std::cout << "ERROR! incorrect value detected!";
                        // TODO: assert or something
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
        std::cout << "could not load file!\n";
    }

    WaitForEnter();
    return 0;
}

/*
TODO:

* make keygen take command line params for number of bits (and prime or not / min value?) instead of asking user for input.

* make this program take command line params for the input file?

! NEXT: something more time consuming or impressive.  Like shor's algorithm or who knows what else.  CORDIC math perhaps to calculate sine since it's branchless?

! NEXT: multiplier? working towards full homomoprhism!
 * https://en.wikipedia.org/wiki/Binary_multiplier


*/