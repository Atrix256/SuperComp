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
* using 8 bits or higher gives wrong results at key 127 (high bit is 0 when it should be 1) with 11, 51 and 1001 as minkey! need to look into why.
 * make it spit out binary math it's doing, and check it by hand to see where it went wrong

! NEXT:
 * write up notes and put in repository, in a mergable file format (txt?)
 * divide / modulus: http://courses.cs.vt.edu/~cs1104/BuildingBlocks/divide.030.html
 * figure out signed add and then make an adder subtractor
  * does CSuperInt need a CSuperUINT version? or just always do signed? probably always signed.
  * uint add is cheaper but multiply is moreso, so not the bottleneck
  * altgough multiply probably wants unsigned add, and then we use XOR of input sign bits to get proper sign bit of result.
 * evaluate a polynomial, like 3x^2+5x+3?
  * is there any way to easily find what key would make the result be zero? like binary search key space or something? probably not, but could be interesting if so, for solving equations.
 * a little nuts, but you could do dual numbers homomorphically too for automatic differentiation...
 * something more time consuming or impressive.  Like shor's algorithm or who knows what else.  CORDIC math perhaps to calculate sine since it's branchless?
 * can we do bootstrapping with superpositional keys? do we have to do one layer deeper of HE or something?
*/