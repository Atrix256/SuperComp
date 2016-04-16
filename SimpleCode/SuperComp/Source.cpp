#include "CKeySet.h"
#include "CSuperInt.h"
#include "Shared.h"
#include "ANF.h"

// change this to change the size of the superpositional integer
typedef CSuperInt<4> TSuperInt;

// turn on for more detailed info
#define SHOW_BITS_AND_ERROR()   0
#define SHOW_VERIFICATION()     0

// turn this on to generate perf data
#define DO_PERF_REPORT()    0
#define PERF_DATA_SAMPLES() 30

typedef std::function<TSuperInt(const TSuperInt&A, const TSuperInt&B)> TestFunc_TSuperInt;
typedef std::function<int(const int&A, const int&B)> TestFunc_Int;
typedef std::function<size_t(const size_t &a, const size_t &b)> TestFunc_Size_T;

double g_PCFreq;

//=================================================================================
template <typename T>
T DoAddition (const T&A, const T&B)
{
    return A + B;
}

//=================================================================================
template <typename T>
T DoSubtraction (const T&A, const T&B)
{
    return A - B;
}

//=================================================================================
template <typename T>
T DoMultiplication (const T&A, const T&B)
{
    return A * B;
}

//=================================================================================
template <typename T>
T DoDivision (const T&A, const T&B)
{
    return A / B;
}

//=================================================================================
template <typename T>
T DoModulus (const T&A, const T&B)
{
    return A % B;
}

//=================================================================================
void ReportPerfData (const char* opName, bool firstTest, bool firstValue, double timeMS)
{
    #if DO_PERF_REPORT()
        char fileName[256];
        sprintf(fileName, "Perf_%s_%i.csv", opName, int(TSuperInt::c_numBits*2));
        FILE *file = fopen(fileName, (firstTest && firstValue) ? "w+t" : "at");
        if (firstTest && firstValue)
            fprintf(file, "Key Lower Bound,Key Generation,Operation,Verification");

        if (firstValue)
            fprintf(file, "\n");

        fprintf(file, "%f,", timeMS);

        fclose(file);
    #endif
}

//=================================================================================
bool DoTest (
    bool allowRightSideZero,
    const char* opSymbol,
    const char* opName,
    TestFunc_TSuperInt testSuperInt,
    TestFunc_Int testInt,
    int testIndex
) {
    printf("\r\n------------------------------\r\nTesting %s (Test %i)\r\n------------------------------\r\n", opName, testIndex+1);
    bool firstTest = testIndex == 0;

    LARGE_INTEGER start, stop;

    // Figure out the smallest key we'll need for this operation
    printf("Calculating Lower Bound Key: ");
    QueryPerformanceCounter(&start);
    TINT minKey = 0;
    {
        std::shared_ptr<CKeySet> exploreKeys = std::make_shared<CKeySet>();
        TSuperInt exploreA(exploreKeys);
        TSuperInt exploreB(exploreKeys);
        exploreA.SetToBinaryMax();
        exploreB.SetToBinaryMax();
        TSuperInt exploreResult = testSuperInt(exploreA, exploreB);
        minKey = (*std::max_element(exploreResult.GetBits().begin(), exploreResult.GetBits().end()));
    }
    QueryPerformanceCounter(&stop);
    printf("%f ms\n", double(stop.QuadPart - start.QuadPart) / g_PCFreq);
    ReportPerfData(opName, firstTest, true, double(stop.QuadPart - start.QuadPart) / g_PCFreq);

    // make the key set that we need, reporting progress
    printf("Making Keys: 9");
    std::shared_ptr<CKeySet> keySet = std::make_shared<CKeySet>();
    QueryPerformanceCounter(&start);
    keySet->Calculate(
        TSuperInt::c_numBits * 2,
        minKey,
        [] (uint8_t percent)
        {
            static uint8_t lastPercent = 0;
            percent = percent * 10 / 100;
            while (lastPercent < percent)
            {
                printf("%c%c", 8, '9' - lastPercent);
                ++lastPercent;
            }
        }
    );
    QueryPerformanceCounter(&stop);
    printf("%c %c%f ms\n", 8, 8, double(stop.QuadPart - start.QuadPart) / g_PCFreq);
    ReportPerfData(opName, firstTest, false, double(stop.QuadPart - start.QuadPart) / g_PCFreq);

    // Do the superpositional operation
    std::cout << "a " << opSymbol << " b in " << TSuperInt::c_numBits << " bits: ";
    TSuperInt A(keySet->GetSuperPositionedBits().begin(), keySet); 
    TSuperInt B(keySet->GetSuperPositionedBits().begin() + keySet->GetSuperPositionedBits().size() / 2, keySet);
    TSuperInt resultsAB(keySet);
    QueryPerformanceCounter(&start);
    resultsAB = testSuperInt(A, B);
    QueryPerformanceCounter(&stop);
    printf("%f ms\n", double(stop.QuadPart - start.QuadPart) / g_PCFreq);
    ReportPerfData(opName, firstTest, false, double(stop.QuadPart - start.QuadPart) / g_PCFreq);

    // report specific bit values and error levels if we should
    #if SHOW_BITS_AND_ERROR()
        ReportBitsAndError(resultsAB);
    #endif

    // Verify results
    printf("Result Verification: ");
    #if SHOW_VERIFICATION()
    printf("\n");
    #endif
    QueryPerformanceCounter(&start);
    bool success = PermuteResults2Inputs(A, B, resultsAB, A.GetKeySet()->GetKeys(),
        [allowRightSideZero, opSymbol, testInt](size_t a, size_t b, size_t keyIndex, const TINT &key, size_t result)
        {
            // don't include zero if we shouldn't
            if (b == 0 && !allowRightSideZero)
                return true;
               
            int intA = TSuperInt::IntFromBinary(a);
            int intB = TSuperInt::IntFromBinary(b);
               
            // show and verify the result
            int actualResult = TSuperInt::IntFromBinary(testInt(intA, intB));
            int computedResult = TSuperInt::IntFromBinary(result);
            #if SHOW_VERIFICATION()
                std::cout << "  [" << keyIndex << "]  " << TSuperInt::IntFromBinary(a) << " " << opSymbol  << " " << TSuperInt::IntFromBinary(b) << " = " << computedResult << "\n";
            #endif
            if (computedResult != actualResult)
            {
                std::cout << "  [" << keyIndex << "] (" << key << ")  " << TSuperInt::IntFromBinary(a) << " " << opSymbol << " " << TSuperInt::IntFromBinary(b) << " = " << computedResult << " (actually " << actualResult << ")\n";
                std::cout << "ERROR! incorrect value detected!\n";
                return false;
            }
            return true;
        }
    );
    QueryPerformanceCounter(&stop);
    printf("%f ms\n", double(stop.QuadPart - start.QuadPart) / g_PCFreq);
    ReportPerfData(opName, firstTest, false, double(stop.QuadPart - start.QuadPart) / g_PCFreq);

    // report circuit complexity
    printf("Circuit Complexity: %0.3f\n", keySet->GetComplexityIndex());

    // return false if the value verification failed
    return success;
}


//=================================================================================
template <size_t NUM_INPUT_BITS, size_t NUM_OUTPUT_BITS>
bool DoTestANF (
    bool allowRightSideZero,
    const char* opSymbol,
    const char* opName,
    TestFunc_Size_T testSizeT,
    int testIndex
) {

    // adapt testSizeT
    auto lambda = [testSizeT, allowRightSideZero](size_t inputValue, size_t numInputBits) -> size_t {
        const size_t bitsA = numInputBits / 2;
        const size_t mask = (1 << bitsA) - 1;

        size_t a = inputValue & mask;
        size_t b = inputValue >> bitsA;

        if (allowRightSideZero || b != 0)
            return testSizeT(a, b);
        else
            return 0;
    };

    // make ANF terms for the function passed in
    auto terms = MakeANFTerms<NUM_INPUT_BITS, NUM_OUTPUT_BITS>(lambda);

    // make int function for the tests
    auto anfTestInt = [&terms] (const int& A, const int& B) -> int {

        // convert from signed integers to our own smaller format signed integer
        size_t a = TSuperInt::BinaryFromInt(A);
        size_t b = TSuperInt::BinaryFromInt(B);

        size_t inputValue = size_t(a) + (size_t(b) << (NUM_INPUT_BITS / 2));

        size_t ret = 0;
        for (size_t outputBitIndex = 0; outputBitIndex < NUM_OUTPUT_BITS; ++outputBitIndex)
        {
            const size_t c_outputBitMask = 1 << outputBitIndex;
            const std::vector<size_t>& bitTerms = terms[outputBitIndex];
            bool xorSum = false;

            for (size_t termIndex = 0; termIndex < bitTerms.size(); ++termIndex)
            {
                size_t term = bitTerms[termIndex];
                if (term == 0)
                {
                    xorSum = 1 ^ xorSum;
                }
                else
                {
                    bool andProduct = true;
                    for (size_t bitIndex = 0; bitIndex < NUM_INPUT_BITS; ++bitIndex)
                    {
                        const size_t bitMask = 1 << bitIndex;
                        if ((term & bitMask) != 0)
                        {
                            if ((inputValue & bitMask) == 0)
                                andProduct = false;
                        }
                    }
                    xorSum = andProduct ^ xorSum;
                }
            }

            if (xorSum)
                ret |= c_outputBitMask;
        }

        return int(ret);
    };

    // make super int function for the tests
    auto anfTestSuperInt = [&terms](const TSuperInt& a, const TSuperInt& b) -> TSuperInt {

        const std::shared_ptr<CKeySet>& keySetPointer = a.GetKeySet();
        const CKeySet& keySet = *keySetPointer;

        CSuperInt<TSuperInt::c_numBits*2> inputValue(a.GetKeySet());
        inputValue.ShiftLeft(NUM_INPUT_BITS/2);
        for (size_t i = 0; i < NUM_INPUT_BITS / 2; ++i)
            inputValue.GetBits()[i] = a.GetBits()[i];

        for (size_t i = 0; i < NUM_INPUT_BITS / 2; ++i)
            inputValue.GetBits()[i+NUM_INPUT_BITS/2] = b.GetBits()[i];

        TSuperInt ret(a.GetKeySet());
        for (size_t outputBitIndex = 0; outputBitIndex < NUM_OUTPUT_BITS; ++outputBitIndex)
        {
            const std::vector<size_t>& bitTerms = terms[outputBitIndex];
            TINT xorSum = 0;

            for (size_t termIndex = 0; termIndex < bitTerms.size(); ++termIndex)
            {
                size_t term = bitTerms[termIndex];
                if (term == 0)
                {
                    xorSum = XOR(1, xorSum, keySet);
                }
                else
                {
                    TINT andProduct = 1;

                    for (size_t bitIndex = 0; bitIndex < NUM_INPUT_BITS; ++bitIndex)
                    {
                        const size_t bitMask = 1 << bitIndex;
                        if ((term & bitMask) != 0)
                            andProduct = AND(andProduct, inputValue.GetBit(bitIndex), keySet);
                    }
                    xorSum = XOR(andProduct, xorSum, keySet);
                }
            }

            ret.GetBit(outputBitIndex) = xorSum;
        }

        return ret;
    };

    // run the tests
    return DoTest(allowRightSideZero, opSymbol, opName, anfTestSuperInt, anfTestInt, testIndex);
}

//=================================================================================
bool DoTests (int testIndex)
{
    /*
    if (!DoTest(true, "+", "Addition", DoAddition<TSuperInt>, DoAddition<int>, testIndex))
        return false;

    if (!DoTest(true, "-", "Subtraction", DoSubtraction<TSuperInt>, DoSubtraction<int>, testIndex))
        return false;

    if (!DoTest(true, "*", "Multiplication", DoMultiplication<TSuperInt>, DoMultiplication<int>, testIndex))
        return false;

    if (!DoTest(false, "/", "Division", DoDivision<TSuperInt>, DoDivision<int>, testIndex))
        return false;

    if (!DoTest(false, "%", "Modulus", DoModulus<TSuperInt>, DoModulus<int>, testIndex))
        return false;
    */

    /*
    if (!DoTestANF<TSuperInt::c_numBits * 2, TSuperInt::c_numBits>(true, "+", "ANF_Addition", DoAddition<size_t>, testIndex))
        return false;

    if (!DoTestANF<TSuperInt::c_numBits * 2, TSuperInt::c_numBits>(true, "-", "ANF_Subtraction", DoSubtraction<size_t>, testIndex))
        return false;

    if (!DoTestANF<TSuperInt::c_numBits * 2, TSuperInt::c_numBits>(true, "/", "ANF_Multiplication", DoMultiplication<size_t>, testIndex))
        return false;
    */

    if (!DoTestANF<TSuperInt::c_numBits * 2, TSuperInt::c_numBits>(false, "/", "ANF_Division", DoDivision<size_t>, testIndex))
        return false;

    /*
    if (!DoTestANF<TSuperInt::c_numBits * 2, TSuperInt::c_numBits>(false, "%", "ANF_Modulus", DoModulus<size_t>, testIndex))
        return false;
    */

    return true;
}

//=================================================================================
int main (int argc, char **argv)
{
    // set up our perf timer
    LARGE_INTEGER timerFreq;
    if (!QueryPerformanceFrequency(&timerFreq))
        return 2;
    g_PCFreq = double(timerFreq.QuadPart) / 1000.0;

    // do tests, wait for enter and return success or failure
    bool success = true;
    #if DO_PERF_REPORT()
    for (int i = 0; i < PERF_DATA_SAMPLES(); ++i)
    #else
    int i = 0;
    #endif
    {
        success &= DoTests(i);
    }
    WaitForEnter();
    return success ? 0 : 1;
}

/*

TODO:
? does reducevalue help or hurt? might make no diff in anf
 * test and see if it changes anything in anf. if not, drop it? even though it might help the other case? how to measure that.
 * need to change note in paper if it makes no difference.
 * could note it shrug.

* Maybe need to find another metric for circuit complexity.  sum of keys doesn't seem to be appropriate as it doesn't tie to execution time.

* run sleepy on this code to see where time is being spent.
 * 5 bit division totally runs fast in ANF so seems like copying / allocating might be the issue?
 * or could really just be circuit complexity

* run perf tests of above using ANF, put data in folder, make graphs.  compare to other add, multiply, divide.

! note for paper: could multithread super int.  each output bit circuit could run in it's own thread.
 * maybe multithread this and use it for timings?
 * maybe option to multithread? not sure if it's exactly honest to report it as the main timing.
  * maybe report timing per output bit, so could see how it would multithread? i dunno

NOTES:

Division complexity:  log10 of sum of keys

       | Regular |   ANF   |
-------|---------|---------|
2 bits |   3.7   |   2.6   |
3 bits |  13.1   |   4.0   |
4 bits |  98.8   |   5.3   |
5 bits |   N/A   |   6.6   |
6 bits |   N/A   |   7.9   |


*/