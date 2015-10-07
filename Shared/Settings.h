//=================================================================================
//
//  Settings.h
//
//  Global Settings
//
//=================================================================================

#include <string>
#include <iostream>

// if CHECK_BITOPS() is 1, it will check CSuperInt bit ops and alert you of overflows
// when using key CHECK_BITOPS_KEY().  If CHECK_BITOPS_LOG() is 1, it will print
// out the details of the logic circuits to help show details of what things are doing
// and where error is creeping in.
#define CHECK_BITOPS() 0
#define CHECK_BITOPS_LOG() 0
#define CHECK_BITOPS_KEY() 733

#if CHECK_BITOPS_LOG()
#define CHECK_BITOPS_LOG_INDENT(x) SCheckBitopsLogIndenter s (x);
#else
    #define CHECK_BITOPS_LOG_INDENT(x)
#endif

extern std::string s_bitOpsLogIndent;

struct SCheckBitopsLogIndenter
{
    SCheckBitopsLogIndenter (const char *scopeName) { std::cout << s_bitOpsLogIndent << scopeName << "\n";s_bitOpsLogIndent += "  "; }
    ~SCheckBitopsLogIndenter () { s_bitOpsLogIndent.pop_back(); s_bitOpsLogIndent.pop_back(); }
};