#include <cstdlib>
#include <stdexcept>

#include "injector.hpp"
#include "snapshot.hpp"

#include <thread>
#include <unordered_set>

namespace
{
    void run(const std::string_view target)
    {
        injector i{};

        std::unordered_set<DWORD> injected{};

        const auto file = utils::nt::library::get_by_address(run).get_folder() / "overlay.dll";

        for (int l = 0; l < 10000000; ++l)
        {
            for (const auto& process : snapshot::get_processes())
            {
                if (std::filesystem::path(process.szExeFile).filename() == target &&
                    !injected.contains(process.th32ProcessID))
                {
                    injected.insert(process.th32ProcessID);

                    printf("Injecting into: %d\n", process.th32ProcessID);
                    const bool res = i.inject(process.th32ProcessID, file);
                    printf("Result: %s\n", res ? "sucess" : "failure");
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2)
        {
            return 1;
        }

        run(argv[1]);
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
