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
            TSuperInt exploreA(-1, exploreKeys); \
            TSuperInt exploreB(-1, exploreKeys); \
            TSuperInt exploreResult = UnitTestFunction_##Name(exploreA, exploreB); \
            minKey = (*std::max_element(exploreResult.GetBits().begin(), exploreResult.GetBits().end())); \
        } \
        \
        /* make the key set that we need, reporting progress */ \
        printf("Making Keys [                    ]"); \
        for (int i = 0; i < 21; ++i) \
            printf("%c", 8); \
        std::shared_ptr<CKeySet> keySet = std::make_shared<CKeySet>(); \
        keySet->CalculateCached(SuperType::c_numBits * 2, minKey, \
            [] (uint8_t percent) \
            { \
                static uint8_t lastPercent = 0; \
                percent = percent * 20 / 100; \
                while (lastPercent < percent) \
                { \
                    ++lastPercent; \
                    printf("*"); \
                } \
            } \
        ); \
        printf("\n"); \
        \
        /* Do our superpositional math */ \
        std::cout << "a" << " " #Operation " " << "b in " << SuperType::c_numBits << " bits\n"; \
        TSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet); \
        TSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet); \
        TSuperInt resultsAB(keySet); \
        resultsAB = UnitTestFunction_##Name(A,B); \
        /* show superpositional result and error (max and % of each key) */ \
        /* ReportBitsAndError(resultsAB); */ \
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
                /* std::cout << "  [" << keyIndex << "]  " << TSuperInt::IntFromBinary(a) << FunctionOperator() << TSuperInt::IntFromBinary(b) << " = " << computedResult << "\n"; */ \
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
