#include "Shared/Shared.h"
#include "Shared/CSuperInt.h"

//=================================================================================
int main(int argc, char **argv)
{
    // verify parameters
    std::cout << "--Adder--\n\nUses an input key file to add bits superpositionally and show results.\n\n";
    if (argc < 2)
    {
        std::cout << "Usage: <inputFile>\n";;
        ExitCode_(1);
    }

    // read in the bits and keys if we can
    std::vector<TINT> superPositionedBits;
    std::vector<TINT> keys;
    std::cout << "Loading " << argv[1] << "\n";
    if (!ReadBitsKeys(argv[1], superPositionedBits, keys))
    {
        std::cout << "could not load!\n";
        ExitCode_(1);
    }
    std::cout << "file loaded!\n\n";

    // Do our superpositional math
    CSuperInt A(superPositionedBits.begin(), superPositionedBits.begin() + superPositionedBits.size() / 2, keys);
    CSuperInt B(superPositionedBits.begin() + superPositionedBits.size() / 2, superPositionedBits.end(), keys);
    CSuperInt resultsAB(keys);
    resultsAB = A + B;
    std::cout << "Added " << A.NumBits() << " bits and " << B.NumBits() << " bits to get a " << resultsAB.NumBits() << " bit result\n";

    // show superpositional result and error (max and % of each key)
    const std::vector<TINT> &adderResults = resultsAB.GetBits();
    std::cout << "\nSuperpositional Adder Result:\n\n";
    ReportBitsAndError(adderResults, keys);

    // Permute through results, make sure truth tables add up
    printf("Result Verification:\n");
    bool success = PermuteResults2Inputs(A, B, //resultsAB,
        [&adderResults, &keys] (size_t a, size_t b, size_t keyIndex)// , size_t result)
        {
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
                return false;
            }
            return true;
        }
    );

    if (!success)
    {
        // TODO: assert or something?
        ExitCode_(1);
    }

    ExitCode_(0);
}

/*
TOOD:
* can the PermuteResults2Inputs function decode adderResults for us?
? can "ReportBitsAndError" take a CSuperInt instead?

* clean this code up
* test with all the keys, even and odd, small and large
* finish multiplier
*/