#include "CKeySet.h"
#include "CSuperInt.h"
#include "Shared.h"

// change this to change the size of the superpositional integer
typedef CSuperInt<4,false> TSuperInt;

// turn on for more detailed info
#define SHOW_BITS_AND_ERROR()   0
#define SHOW_VERIFICATION()     0

// turn this on to generate perf data
#define DO_PERF_REPORT()    0
#define PERF_DATA_SAMPLES() 30

typedef TSuperInt (*TestFunc_TSuperInt) (const TSuperInt& A, const TSuperInt& B);
typedef int (*TestFunc_Int) (const int &a, const int &b);

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
    return A / B;
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

    // return false if the value verification failed
    return success;
}

//=================================================================================
bool DoTests (int testIndex)
{
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
* make supercomp have a boolean to fall back to using ANF.
* make variadic (template?) XOR / AND? not sure if needed.
* run perf tests of above using ANF, put data in folder, make graphs.  compare to other add, multiply, divide.

? should we also try unums?
*/