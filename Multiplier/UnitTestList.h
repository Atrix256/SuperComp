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

// TODO: Negate() and Abs() for int.
// TODO: Negate() and Abs() for fixed point.

// TODO: need a fixed point class to be able to compare results with "reality"
//UNITTEST(Fixed_Add, int, TSuperFixed, +, true)
//UNITTEST(Fixed_Subtract, int, TSuperFixed, -, true)

#undef UNITTEST

// TODO: need a way to make the unit tests verbose, to show truth table and errors etc. Also key sizes?
// TODO: report timing of unit tests
// TODO: more progress bar reporting? maybe show it on the line below the current operation, then erasing it to print the next operation? or show on same line.  Then erase and replace with timing in seconds for how long it took?
// TODO: how does that work out when writing to a file though?