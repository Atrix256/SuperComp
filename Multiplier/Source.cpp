#include "Shared/Shared.h"
#include "Shared/CKeySet.h"

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
    std::shared_ptr<CKeySet> keySet = std::make_shared<CKeySet>();
    std::cout << "Loading " << argv[1] << "\n";
    if (!keySet->Read(argv[1]))
    {
        std::cout << "could not load!\n";
        ExitCode_(1);
    }
    std::cout << "file loaded!\n\n";

    /*
    // TODO: TEMP - find out the key size we need for a 4 bit vs 4 bit multiply
    std::shared_ptr<CKeySet> exploreKeys = std::make_shared<CKeySet>();
    CSuperInt exploreA(15, exploreKeys);
    CSuperInt exploreB(15, exploreKeys);
    CSuperInt exploreConstant(1, exploreKeys);
    CSuperInt exploreResult = exploreA * exploreB;
    TINT maxValue = 0;
    for (const TINT &v : exploreResult.GetBits())
    {
        if (v > maxValue)
            maxValue = v;
        std::cout << v << "\n";
    }
    std::cout << "\n" << maxValue;
    */

    // Do our superpositional math
    CSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet);
    CSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet->GetSuperPositionedBits().end(), keySet);
    CSuperInt resultsAB(keySet);
    CSuperInt constant(8, keySet);
    resultsAB = A * B * constant;
    std::cout << "Multiplied " << A.NumBits() << " bits and " << B.NumBits() << " bits and 8, to get a " << resultsAB.NumBits() << " bit result\n\n";

    // show superpositional result and error (max and % of each key)
    ReportBitsAndError(resultsAB);

    // Permute through results, make sure truth tables add up
    printf("Result Verification:\n");
    bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet().GetKeys(),
        [](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result)
    {
        // show and verify the result
        std::cout << "  [" << keyIndex << "] (" << key << ")  " << a << " * " << b << " * 8 = " << result << "\n";
        if (result != a * b * 8)
        {
            std::cout << "ERROR! incorrect value detected!";
            return false;
        }
        return true;
    }
    );

    if (!success)
    {
        Assert_(success);
        ExitCode_(1);
    }

    ExitCode_(0);
}

/*
TODO:
* is there a way to calculate the key value you'd need to pass through a given circuit? yes! below
* send all 1s through circuit to find the maximum required key.  Can then make that+1 the smallest key for the circuit.
 * paper notes: this works because every logical operation grows the value (yes, even not!), so sending 1s will expose the largest error.
 * paper notes: this isn't garaunteed to be the smallest set of keys that could pass through the circuit though.
  * both unused circuit paths (an overflow being ANDED by zero)
  * as well as the fact that the largest key required doesn't line up with the key 0. diff keys may require diff sizes.
 ! this seems like it might not be right.  for 4 bits and 4 bits it's telling me 127733759, but the key 10000001 for 8 bits works just fine.
 ? if it isn't right, why does the smaller key work? maybe there's something more to that... like it overflows an even number of times so we are ok??
  * maybe look w/ debugging again and see what's going on with the max key.
  * i think this has something to do with modulus against LCM not happening, so this isn't the real number, the modulus of some other number is the real number.
  * not real sure how to calculate the right key then...

? why does multiplying by a constant add a 0 on the high side? need to fix that, it'll mess with signed numbers
 * multiply 3 by 1 and see the results are 011. multiplication order matters
* signed math (multiplication / addition) with twos complement
 * iterate through the superpositions with negative values as well
* make a subtractor project?
* assert in CSuperInt that the key set pointer is the same value when doing math against multiple CSuperInts?
 * could also make it a static of CSuperInt perhaps, but that isn't so great.
* the logging assumes the debug key is a regular int, need to treat it as a TINT.
* with keys that large, need to make sure we treat all residues as TINTs
* assert that the key set is the same address when multiplying or adding?

* paper notes:
 * you can use non superpositional int values.  they mod any key will be the same value!
 * you can multiply any superpositional int by a power of 2 without it creating more error due to this.

! NEXT:
 * divide / modulus: http://courses.cs.vt.edu/~cs1104/BuildingBlocks/divide.030.html
 * figure out signed add and then make an adder subtractor
  * does CSuperInt need a CSuperUINT version? or just always do signed? probably always signed.
  * uint add is cheaper but multiply is moreso, so not the bottleneck
  * altgough multiply probably wants unsigned add, and then we use XOR of input sign bits to get proper sign bit of result.
 * evaluate a polynomial, like 3x^2+5x+3? (this would have both add and multiply by a constant)
  * is there any way to easily find what key would make the result be zero? like binary search key space or something? probably not, but could be interesting if so, for solving equations.
  * maybe have the thing evaluate circuit first to figure out the key size
 * a little nuts, but you could do dual numbers homomorphically too for automatic differentiation...
 * something more time consuming or impressive.  Like shor's algorithm or who knows what else.  CORDIC math perhaps to calculate sine since it's branchless?
 * RECRYPT: how to do this?
  * could decrypt all permutations then maybe re-calculate numbers that fit those bounds and continue
  * problem: could be lots of permutations and take a while. what would the benefit be?

! note, could just put "first key" in key file, and then have the user generate the next # coprime values for the rest of the keys
? should we put the paper on arxiv before submitting?
*/