#include "Shared/Shared.h"
#include "Shared/CKeySet.h"
#include "Shared/CFixed.h" // TODO: temp

#include "UnitTests.h"

//=================================================================================
int main(int argc, char **argv)
{
    // TODO: temp fixed point tests
    {
        const float c_valueA = 1.5f;
        const float c_valueB = 0.5f;

        std::shared_ptr<CKeySet> keys = std::make_shared<CKeySet>();
        TSuperFixed a(c_valueA, keys);
        TSuperFixed b(c_valueB, keys);

        TSuperFixed aplusb = a + b;
        TSuperFixed aminusb = a - b;
        TSuperFixed atimesb = a * b;
        TSuperFixed adivb = a / b;

        float faplusb = aplusb.DecodeFloat(2);
        float faminusb = aminusb.DecodeFloat(2);
        float fatimesb = atimesb.DecodeFloat(2);
        float fadivb = adivb.DecodeFloat(2);

        TFixed fixeda(c_valueA);
        TFixed fixedb(c_valueB);

        TFixed fixedaplusb = fixeda + fixedb;
        TFixed fixedaminusb = fixeda - fixedb;

        float ffixedaplusb = fixedaplusb.GetFloat();
        float ffixedaminusb = fixedaminusb.GetFloat();

        // TODO: fix CFixed storing and recalling negative values

        /*
        float fa = a.DecodeFloat(2);
        float fb = b.DecodeFloat(2);

        float actualatimesb = fa*fb;
        float actualadivb = fa/fb;
        */

        int ijkl = 0;
    }

    DoUnitTests();

    ExitCode_(0);
}


/*

TODO:

* rename multiplier project to unittests?
* get rid of adder?
* do we still need the key generator project?
* do we still need the shared project? maybe not...

* see what else needs to be removed from code (like macros.h might have stuff?)

* try GMP backend for boost numbers and tom_int (http://www.boost.org/doc/libs/1_56_0/libs/multiprecision/doc/html/index.html)
 * see if those guys are any faster than cpp_int
 * see if there's still a problem with dividing 1.5 by -0.5 in CSuperFixed<3,2> with increased division precision

* try doing fixed size integers to get rid of allocations.
 * could look into something that makes operations reflective and outputs a cpp file that is the flat operations done with the right number of bits in each step.

* it's easy to bust the size of fixed point by accident. how should we deal with that? like a <2,2> when you add 1.5 and 0.5 you got -2.0, which is wrong.

* do a polynomial demo of some kind. bezier curve? storageless shuffler? calculate sine for an 8 bit number? all three?

* unit tests for fixed point operations and integer operations

* unit tests for non superpositional values? or maybe having one with constants is enough?

* make it easier to switch to int64_t somehow (conversion functions are the stumbling blocks)
 * or, can we at least visualize the big ints in watch window somehow?
* organize code a bit better.  Like put the HE stuff in it's own header - include basic HE math routines?

* very large keys have problems having a file written out for them.
* might need to make a better caching system?
* like.. an index file that describes key files and points at them.

* can we somehow make this boilerplate code go away, for generating keys and verification and such?
* going to use it for more complex functions as well, besides adder and multiplier
* make we can make a class with an pure virtual interface to describe these tests?

* can keyset use array? i think so
* might want -= and += operators? maybe ++ and -- too?
* can we make if statements easier? maybe we could have something that takes a lambda for when it's true or something?
* profile code eventually and figure out where the time is going, and try to optimize


* make unit tests to test all operations.
* refer to TINT as SuperBit? or typedef it as the same so you can use whichever is more appropriate situationally? phantom type it?
* does it make sense to return a superbit without a keyset? maybe we need a new class CSuperBit which can hold a single bit, and the key set?
* verify that the key sets are the same between two ints, not just size. (assert that pointer is the same perhaps? doesn't catch the case of when values are same but pointer is different though)
* review if we are always using size_t and unsigned int appropriately. maybe stick to one?

*/