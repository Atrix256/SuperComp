//=================================================================================
//
//  UnitTests.h
//
//  Expands UnitTestList.h into actual work
//
//=================================================================================

#pragma once

#include "Shared\CSuperFixed.h"
#include "Shared\CFixed.h"

// TODO: convert unit test code to use SuperType and BasicType all the way.
// TODO: make it show fixed point as float output
// TODO: instead of saying "in 4 bits" for fixed, could have it say eg 2.2
// TODO: instead of int being the type for superint, maybe need to use CFixed for that with 0 fractional bits?

#define SHOW_VERIFICATION() 1
#define SHOW_BITSANDERROR() 0

#if SHOW_VERIFICATION()
    #define VERIFICATION(x) x
#else
    #define VERIFICATION(x)
#endif

#if SHOW_BITSANDERROR()
    #define BITSANDERROR(x) x
#else
    #define BITSANDERROR(x)
#endif

// make the templated operation to support each unit test
#define UNITTEST(Name, BasicType, SuperType, Operation, AllowRightSideZero) \
    template <typename T> \
    T UnitTestFunction_##Name (T& a, T& b) \
    { \
        return a Operation b; \
    }
#include "UnitTestList.h"

// make the actual unit test
#define UNITTEST(Name, BasicType, SuperType, Operation, AllowRightSideZero) \
    bool DoUnitTest_##Name () \
    { \
        printf("UnitTest: " #Name "\n"); \
        /* Figure out the smallest key we'll need for this operation */ \
        TINT minKey = 0; \
        { \
            std::shared_ptr<CKeySet> exploreKeys = std::make_shared<CKeySet>(); \
            SuperType exploreA(exploreKeys); \
            SuperType exploreB(exploreKeys); \
            exploreA.SetToBinaryMax(); \
            exploreB.SetToBinaryMax(); \
            SuperType exploreResult = UnitTestFunction_##Name(exploreA, exploreB); \
            minKey = (*std::max_element(exploreResult.GetBits().begin(), exploreResult.GetBits().end())); \
        } \
        \
        /* make the key set that we need, reporting progress */ \
        printf("Making Keys: "); \
        std::shared_ptr<CKeySet> keySet = std::make_shared<CKeySet>(); \
        keySet->CalculateCached(SuperType::c_numBits * 2, minKey, \
            [] (uint8_t percent) \
            { \
                static uint8_t lastPercent = 0; \
                percent = percent * 10 / 100; \
                while (lastPercent < percent) \
                { \
                    printf("%c", '9' - lastPercent); \
                    ++lastPercent; \
                } \
            } \
        ); \
        printf("\n"); \
        \
        /* Do our superpositional math */ \
        std::cout << "a" << " " #Operation " " << "b in " << SuperType::c_numBits << " bits\n"; \
        SuperType A(keySet->GetSuperPositionedBits().begin(), keySet); \
        SuperType B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet); \
        SuperType resultsAB(keySet); \
        resultsAB = UnitTestFunction_##Name(A,B); \
        /* show superpositional result and error (max and % of each key) */ \
        BITSANDERROR(ReportBitsAndError(resultsAB)); \
        \
        /* Verify result permutations */ \
        printf("Result Verification...\n"); \
        bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet()->GetKeys(), \
            [](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result) \
            { \
                /* don't include zero if we shouldn't */ \
                if (b == 0 && !AllowRightSideZero) \
                    return true; \
                \
                int intA = TSuperInt::IntFromBinary(a); \
                int intB = TSuperInt::IntFromBinary(b); \
                \
                /* show and verify the result */ \
                int actualResult = TSuperInt::IntFromBinary(UnitTestFunction_##Name(intA, intB)); \
                int computedResult = TSuperInt::IntFromBinary(result); \
                VERIFICATION(std::cout << "  [" << keyIndex << "]  " << TSuperInt::IntFromBinary(a) << " " #Operation " " << TSuperInt::IntFromBinary(b) << " = " << computedResult << "\n"); \
                if (computedResult != actualResult) \
                { \
                    std::cout << "  [" << keyIndex << "] (" << key << ")  " << TSuperInt::IntFromBinary(a) << " " #Operation " " << TSuperInt::IntFromBinary(b) << " = " << computedResult << " (actually " << actualResult << ")\n"; \
                    std::cout << "ERROR! incorrect value detected!\n"; \
                    return false; \
                } \
                return true; \
            } \
        ); \
        printf("\n"); \
        return success; \
    }
#include "UnitTestList.h"

// The function to do all the unit tests
void DoUnitTests ()
{
    #define UNITTEST(Name, BasicType, SuperType, Operation, AllowRightSideZero) \
        if (!DoUnitTest_##Name()) \
            return;
    #include "UnitTestList.h"
}
