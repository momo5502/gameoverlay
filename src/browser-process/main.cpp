#include <utils/win.hpp>

#include <web_ui.hpp>

int main(int, char**)
{
    try
    {
        return gameoverlay::web_ui::run_process();
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
