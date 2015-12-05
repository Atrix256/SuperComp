//=================================================================================
//
//  UnitTestList.h
//
//  Describes unit tests to perform
//
//=================================================================================

/*

UNITTEST(Name, BasicType, SuperType, Operation, AllowRightSideZero)

*/

typedef CSuperInt<3> TSuperInt;
typedef CSuperFixed<2, 2> TSuperFixed;
typedef CFixed<2, 2> TFixed;

UNITTEST(Int_Add, int, TSuperInt, +, true)
UNITTEST(Int_Subtract, int, TSuperInt, -, true)
UNITTEST(Int_Multiply, int, TSuperInt, *, true)
UNITTEST(Int_Divide, int, TSuperInt, /, false)
UNITTEST(Int_Modulus, int, TSuperInt, %, false)

UNITTEST(Fixed_Add, TFixed, TSuperFixed, +, true)
// TODO: uncomment and get working
//UNITTEST(Fixed_Subtract, TFixed, TSuperFixed, -, true)
//UNITTEST(Fixed_Multiply, TFixed, TSuperFixed, *, true)
//UNITTEST(Fixed_Divide, TFixed, TSuperFixed, /, false)

// TODO: Negate() and Abs() for int and fixed point

#undef UNITTEST

// TODO: report timing of unit tests
// TODO: more progress bar reporting? maybe show it on the line below the current operation, then erasing it to print the next operation? or show on same line.  Then erase and replace with timing in seconds for how long it took?
// TODO: how does that work out when writing to a file though? seems ok.  it has some weird blocks for backspace but whatever
// TODO: maybe a define for showing progress bars or not?