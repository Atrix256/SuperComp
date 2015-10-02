#include "Shared/Shared.h"

//=================================================================================
int main(int argc, char **argv)
{
    // verify parameters
    std::cout << "--Multiplier--\nUses an input key file to multiply bits superpositionally and show results.\n\n";
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
    resultsAB = A * B;
    std::cout << "Multiplied " << A.NumBits() << " bits and " << B.NumBits() << " bits to get a " << resultsAB.NumBits() << " bit result\n\n";

    // show superpositional result and error (max and % of each key)
    ReportBitsAndError(resultsAB, keys);

    // Permute through results, make sure truth tables add up
    printf("Result Verification:\n");
    bool success = PermuteResults2Inputs(A, B, resultsAB, keys,
        [](size_t a, size_t b, size_t keyIndex, size_t result)
    {
        // show and verify the result
        std::cout << "  [" << keyIndex << "]  " << a << " * " << b << " = " << result << "\n";
        if (result != a * b)
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
TODO:
* using 8 bits or higher gives wrong results. need to look into why. error?
* how to do signed add / multiply?
 * multiply is easy... xor sign bit.
 * add = ?? can't do two's compliment, unless we also want to change multiply. Also our ints are unbounded. high bit = ???
* make SMALLESTKEYVALUE be a command line parameter for key generator


! NEXT:
 * divide / modulus: http://courses.cs.vt.edu/~cs1104/BuildingBlocks/divide.030.html
 * something more time consuming or impressive.  Like shor's algorithm or who knows what else.  CORDIC math perhaps to calculate sine since it's branchless?
 * can we do bootstrapping with superpositional keys? do we have to do one layer deeper of HE or something?
*/