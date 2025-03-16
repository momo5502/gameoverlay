#include <cstdlib>
#include <stdexcept>

#include "injector.hpp"
#include "snapshot.hpp"

namespace
{
    void run()
    {
        injector i{};

        const auto file = utils::nt::library::get_by_address(run).get_folder() / "overlay.dll";

        for (const auto& process : snapshot::get_processes())
        {
            if (std::filesystem::path(process.szExeFile).filename() == "bird.exe")
            {
                printf("Injecting into: %X\n", process.th32ProcessID);
                const bool res = i.inject(process.th32ProcessID, file);
                printf("Result: %s\n", res ? "sucess" : "failure");
            }
        }
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
