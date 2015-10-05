#include "Shared/Shared.h"
#include "Shared/CKeySet.h"

//=================================================================================
int main(int argc, char **argv)
{
    // verify parameters
    std::cout << "--Adder--\nUses an input key file to add bits superpositionally and show results.\n\n";
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

    // Do our superpositional math
    CSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet);
    CSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet->GetSuperPositionedBits().end(), keySet);
    CSuperInt resultsAB(keySet);
    resultsAB = A + B;
    std::cout << "Added " << A.NumBits() << " bits and " << B.NumBits() << " bits to get a " << resultsAB.NumBits() << " bit result\n\n";

    // show superpositional result and error (max and % of each key)
    ReportBitsAndError(resultsAB);

    // Permute through results, make sure truth tables add up
    printf("Result Verification:\n");
    bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet().GetKeys(),
        [](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result)
        {
            // show and verify the result
            std::cout << "  [" << keyIndex << "] (" << key << ")  " << a << " + " << b << " = " << result << "\n";
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
        Assert_(success);
        ExitCode_(1);
    }

    ExitCode_(0);
}