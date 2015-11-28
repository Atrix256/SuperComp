#include "Shared/Shared.h"
#include "Shared/CKeySet.h"

typedef CSuperInt<4> TSuperInt;

//=================================================================================
template <typename T>
T DoFunction (T& a, T& b)
{
    return a + b;
}

//=================================================================================
int main(int argc, char **argv)
{
    // Figure out the smallest key we'll need for this operation
    TINT minKey = 0;
    {
        std::shared_ptr<CKeySet> exploreKeys = std::make_shared<CKeySet>();
        TSuperInt exploreA(-1, exploreKeys);
        TSuperInt exploreB(-1, exploreKeys);
        TSuperInt exploreResult = DoFunction(exploreA, exploreB);

        minKey = (*std::max_element(exploreResult.GetBits().begin(), exploreResult.GetBits().end())).convert_to<size_t>();
    }

    // make the key set that we need, reporting progress
    printf("Making Keys [                    ]");
    for (int i = 0; i < 21; ++i)
        printf("%c", 8);
    std::shared_ptr<CKeySet> keySet = std::make_shared<CKeySet>();
    keySet->CalculateCached(TSuperInt::c_numBits * 2, minKey,
        [] (uint8_t percent)
        {
            static uint8_t lastPercent = 0;
            percent = percent * 20 / 100;
            while (lastPercent < percent)
            {
                ++lastPercent;
                printf("*");
            }
        }
    );
    printf("\n");

    // Do our superpositional math
    std::cout << "Adding two numbers in " << TSuperInt::c_numBits << " bits\n\n";
    TSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet);
    TSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet);
    TSuperInt resultsAB(keySet);
    resultsAB = DoFunction(A,B);

    // show superpositional result and error (max and % of each key)
    ReportBitsAndError(resultsAB);

    // Permute through results, make sure truth tables add up
    printf("Result Verification...\n");
    bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet()->GetKeys(),
        [](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result)
        {
            // show and verify the result
            int actualResult = TSuperInt::IntFromBinary(DoFunction(a, b));
            int computedResult = TSuperInt::IntFromBinary(result);
            if (computedResult != actualResult)
            {
                std::cout << "  [" << keyIndex << "] (" << key << ")  " << TSuperInt::IntFromBinary(a) << " + " << TSuperInt::IntFromBinary(b) << " = " << computedResult << " (actually " << actualResult << ")\n";
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
