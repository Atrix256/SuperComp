#include "Shared/Shared.h"
#include "Shared/CKeySet.h"
#include "Shared/CSuperFixed.h" // TODO: temp

typedef CSuperInt<3> TSuperInt;
typedef CSuperFixed<3, 2> TSuperFixed;

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
    // TODO: temp fixed point tests
    {
        std::shared_ptr<CKeySet> keys = std::make_shared<CKeySet>();
        TSuperFixed a(1.5f, keys);
        TSuperFixed b(-0.5f, keys);

        TSuperFixed aplusb = a + b;
        TSuperFixed aminusb = a - b;
        TSuperFixed atimesb = a * b;
        TSuperFixed adivb = a / b;

        float fa = a.DecodeFloat(2);
        float fb = b.DecodeFloat(2);

        float faplusb = aplusb.DecodeFloat(2);
        float faminusb = aminusb.DecodeFloat(2);
        float fatimesb = atimesb.DecodeFloat(2);
        float fadivb = adivb.DecodeFloat(2);

        float actualatimesb = fa*fb;
        float actualadivb = fa/fb;

        // TODO: make sure division works. may not want to extend precision though since it takes so long. maybe have a # define for extending precision in both division and multiplication?

        int ijkl = 0;
    }

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

* maybe a define for increased precision? or a different function call or soemthing? compare contrast in paper perhaps. for CSuperFixed

* should we also do (or offer) TINT in increased precision for * and /?
 * maybe not, but could note it as a possibility in the paper?

* it's easy to bust the size of fixed point by accident. how should we deal with that? like a <2,2> when you add 1.5 and 0.5 you got -2.0, which is wrong.

* when you start doing fixed point multiply and divide it gets even worse. should we use higher precision temporarily when doing fixed point multiplication and division?
 * yeah, i think we should.  in CSuperFixed * and /, do the math in a larger number of bits (i think maybe it should be BITS_INTEGER+BITS_FRACTION+BITS_FRACTION) then copy the bits back in.
 * or, re-look up how many bits the result of an N bit * N bit number should be.  similar for division?

* do a polynomial demo of some kind. bezier curve? storageless shuffler? calculate sine for an 8 bit number? all three?

* unit tests for fixed point operations too


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