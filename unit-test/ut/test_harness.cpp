#include "test_harness.h"

#include <string>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>


// MSVC defines this in winsock2.h!?
typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

int gettimeofday(struct timeval* tp, struct timezone* tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((uint64_t)file_time.dwLowDateTime);
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}

namespace common {
    namespace test {
        int errCount = 0;
        namespace {
            struct Test {
                const char* base;
                const char* name;
                void(*func)();
            };
            std::vector<Test>* tests;
        }

        bool RegisterTest(const char* base, const char* name, void(*func)()) {
            if (tests == NULL) {
                tests = new std::vector<Test>;
            }
            Test t;
            t.base = base;
            t.name = name;
            t.func = func;
            tests->push_back(t);
            return true;
        }

        int RunAllTests(const char* matcher) {
            int num = 0;
            if (tests != NULL) {
                for (size_t i = 0; i < tests->size(); i++) {
                    const Test& t = (*tests)[i];
                    if (matcher != NULL) {
                        std::string name = t.base;
                        name.push_back('.');
                        name.append(t.name);
                        if (strstr(name.c_str(), matcher) == NULL) {
                            continue;
                        }
                    }
                    fprintf(stderr, "\033[0;32m[ RUN      ] ==== Test %s.%s\n", t.base, t.name);
                    fprintf(stderr, "\033[0m");
                    (*t.func)();
                    ++num;
                }
            }
            fprintf(stderr, "\033[0;32m[ PASS     ] ==== PASSED %d tests\n", num);
            fprintf(stderr, "\033[0;31m[ NOPASS   ] ==== ERROR %d tests\n", errCount);
            fprintf(stderr, "\033[0m\n");
            return 0;
        }

        std::string TmpDir() {
            return "/tmp";
        }

        int RandomSeed() {
            return 301;
        }
        TestPerfomence::TestPerfomence() {
            startMs_ = NowMs();
        }
        TestPerfomence::TestPerfomence(int size) {
            startMs_ = NowMs();
            fprintf(stderr,
                "\033[0;32m[ RUN      ] ==== start to run %lu cases.\n",
                size);
        }
        TestPerfomence::~TestPerfomence() {
            long endMs = NowMs();
            fprintf(stderr,
                "\033[0;32m[ RUN      ] ==== start at %lu, stop at %lu, cost:[%lu]\n",
                startMs_, endMs, endMs - startMs_);
        }
        long TestPerfomence::NowMs() {
            struct timeval timeNow;
            gettimeofday(&timeNow, NULL);
            return (timeNow.tv_sec) * 1000 + timeNow.tv_usec / 1000;
        }

    }
}  // namespace common

using namespace common::test;
int main(int argc, char** argv) {
    common::test::RunAllTests(NULL);
    return 0;
}
