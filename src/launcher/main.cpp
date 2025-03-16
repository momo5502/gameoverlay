#include <utils/win.hpp>
#include <cstdlib>
#include <stdexcept>

#include "injector.hpp"

namespace
{
    void run()
    {
        injector i{};
        (void)i;
    }
}

int main(int, char**)
{
    try
    {
        run();
        return 0;
    }
    catch (const std::exception& e)
    {
#ifdef _WIN32
        OutputDebugStringA(e.what());
#else
        puts(e.what());
#endif
    }

    return -1;
}

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
    return main(__argc, __argv);
}
#endif
