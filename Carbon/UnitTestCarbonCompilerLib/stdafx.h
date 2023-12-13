// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef __clang__ 
    #ifndef USE_MAIN
        #define USE_MAIN
    #endif
#endif

#ifndef USE_MAIN

    #define FRAMEWORK Microsoft::VisualStudio::CppUnitTestFramework

    // Including SDKDDKVer.h defines the highest available Windows platform.

    // If you wish to build your application for a previous Windows platform, include WinSDKVer.h and
    // set the _WIN32_WINNT macro to the platform you wish to support before including SDKDDKVer.h.

    #include <SDKDDKVer.h>
    // Headers for CppUnitTest
    #include "CppUnitTest.h"

#else

    extern int __global_test_count;
    extern int __global_pass_count;

    #define FRAMEWORK BasicTest
    #define TEST_CLASS(N) class N
    #define TEST_METHOD(N) void N()
    #define TEST_ALL_METHOD() void TestAll()
    #define RUN_TEST_METHOD(N) {\
        printf(" - %s ", #N); __global_test_count+=1;\
        try {N(); printf("\x1b[32mpassed \x1b[0m\n"); __global_pass_count+=1; } \
        catch (std::logic_error error) { printf("\x1b[31mfailed\x1b[0m %s\n", error.what()); }\
    }
    #define RUN_TEST_CLASS(N) {printf("%s\n",#N);N().TestAll();}
    #define RUN_TEST_SUMMARY() BasicTest::TestSummary();

    #include <stdexcept>
    #include <stdlib.h>
    #include <string.h>
    #include <stdio.h>


    namespace BasicTest
    {
        namespace Assert
        {
            inline void Fail(const char* message) { throw std::logic_error(message); }

            inline void Fail(const wchar_t* message)
            {
                int n = 1024;
                char *mbstring = new char[n];
                wcstombs(mbstring,message,n);
                Fail(mbstring);
            }

            inline void AreEqual(int a, int b) {
                if (a != b) {
                    Fail("Not equal");
                }
            }

            inline void IsFalse(bool a, const wchar_t* message = L"Shoud be false") {
                if (a) Fail(message);
            }
        }

        inline int TestSummary() {
            if (__global_pass_count == 0 || __global_pass_count != __global_test_count) {
                printf("%d out of %d tests failed\n", __global_test_count - __global_pass_count, __global_test_count);
                return 1;
            }
            else {
                printf("all %d tests passed\n", __global_pass_count);
                return 0;
            }
        }
    }

#endif 



#include "ErrorMessageFormatter.h"

// TODO: reference additional headers your program requires here
