#define NOMINMAX
#include <Windows.h> // for IsDebuggerPresent() and DebugBreak()

#define ExitCode_(x) {if (IsDebuggerPresent()) {WaitForEnter();} return x;}

#define Assert_(x) if (!(x)) { printf("Assert Failed : " #x); DebugBreak(); }