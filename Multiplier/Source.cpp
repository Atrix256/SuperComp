#include "Shared/Shared.h"
#include "Shared/CKeySet.h"

typedef CSuperInt<3> TSuperInt;

//=================================================================================
template <typename T>
T DoFunction (T& a, T& b)
{
    return a / b;
}

//=================================================================================
const char* FunctionOperator ()
{
    return " / ";
}

//=================================================================================
bool RightSideCanBeZero ()
{
    return false;
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

        minKey = (*std::max_element(exploreResult.GetBits().begin(), exploreResult.GetBits().end()));
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
    std::cout << "a" << FunctionOperator() << "b in " << TSuperInt::c_numBits << " bits\n\n";
    TSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet);
    TSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet);
    TSuperInt resultsAB(keySet);
    resultsAB = DoFunction(A,B);

    // show superpositional result and error (max and % of each key)
    //ReportBitsAndError(resultsAB);

    // Permute through results, make sure truth tables add up
    printf("Result Verification...\n");
    bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet()->GetKeys(),
        [](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result)
        {
            // don't test dividing by zero!
            if (b == 0 && !RightSideCanBeZero())
                return true;

            int intA = TSuperInt::IntFromBinary(a);
            int intB = TSuperInt::IntFromBinary(b);

            // show and verify the result
            int actualResult = TSuperInt::IntFromBinary(DoFunction(intA, intB));
            int computedResult = TSuperInt::IntFromBinary(result);
            std::cout << "  [" << keyIndex << "]  " << TSuperInt::IntFromBinary(a) << FunctionOperator() << TSuperInt::IntFromBinary(b) << " = " << computedResult << "\n";
            if (computedResult != actualResult)
            {
                std::cout << "  [" << keyIndex << "] (" << key << ")  " << TSuperInt::IntFromBinary(a) << FunctionOperator() << TSuperInt::IntFromBinary(b) << " = " << computedResult << " (actually " << actualResult << ")\n";
                std::cout << "ERROR! incorrect value detected!\n";
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
11/26/15
* clean up code to get ready for checking in
* do a polynomial demo of some kind. bezier curve? storageless shuffler? calculate sine for an 8 bit number? all three?

* make a fixed point abstraction class.
* make it easier to switch to int64_t somehow (conversion functions are the stumbling blocks)
 * or, can we at least visualize the big ints in watch window somehow?
* organize code a bit better.  Like put the HE stuff in it's own header - include basic HE math routines?

* very large keys have problems having a file written out for them.
* might need to make a better caching system?
* like.. an index file.

* can we somehow make this boilerplate code go away, for generating keys and verification and such?
* going to use it for more complex functions as well, besides adder and multiplier
* make we can make a class with an pure virtual interface to describe these tests?

* can keyset use array? i think so
* might want -= and += operators? maybe ++ and -- too?
* can we make if statements easier? maybe we could have something that takes a lambda for when it's true or something?
* profile code eventually and figure out where the time is going, and try to optimize
* make a fixed point class!


* make unit tests to test all operations.
// TODO: refer to TINT as SuperBit? or typedef it as the same so you can use whichever is more appropriate situationally>?
// TODO: does it make sense to return a superbit without a keyset? maybe we need a new class CSuperBit which can hold a single bit?
// TODO: verify that the key sets are the same between two ints, not just size. (assert that pointer is the same perhaps? doesn't catch the case of when values are same but pointer is different though)
// TODO: review if we are always using size_t and unsigned int appropriately. maybe stick to one?

*/