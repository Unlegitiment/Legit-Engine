#include <vcruntime.h>
#include <cstdlib>
#include <cstdint>
extern "C" __declspec(dllimport)
void* GetStdHandle(unsigned long);

extern "C" __declspec(dllimport)
int WriteFile(void*, const void*, unsigned long, unsigned long*, void*);

extern "C" __declspec(dllimport)
void ExitProcess(unsigned int);

extern "C" int main() {
    int* str = (int*)malloc(sizeof(10) * 4);
    const char msg[] = "we bypassed the CRT for no reason\n";

    void* handle = GetStdHandle((unsigned long)-11); // STD_OUTPUT_HANDLE
    unsigned long written;

    WriteFile(handle, msg, sizeof(msg) - 1, &written, 0);
    return 0;
}

extern "C" void _start() {
    int r = main();
    ExitProcess(r);
}