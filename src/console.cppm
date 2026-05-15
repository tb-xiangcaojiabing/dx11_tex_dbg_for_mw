module;

#include <format>
#include <cstdio>

export module console;

export namespace console::ansi {
    constexpr std::string_view bold = "\x1b[1m";
    constexpr std::string_view reset = "\033[0m";
    constexpr std::string_view clean_line = "\r\033[K";
    
    #define rgb(R, G, B) "\x1b[38;2;" #R ";" #G ";" #B "m"
    constexpr std::string_view white = rgb(233, 233, 233);
    constexpr std::string_view green = rgb(96, 200, 135);
    constexpr std::string_view blue = rgb(76, 186, 250);
    constexpr std::string_view orange = rgb(241, 149, 88);
    constexpr std::string_view red = rgb(247, 101, 104);
    #undef rgb
}

export namespace console {

#ifdef _WIN32
    #if defined(_MSC_VER)
        #pragma comment(lib, "shell32.lib")
    #endif
    extern "C" {
        __declspec(dllimport) void* __stdcall GetStdHandle(unsigned nStdHandle);
        __declspec(dllimport) int __stdcall GetConsoleMode(void* hConsoleHandle, unsigned* lpMode);
        __declspec(dllimport) int __stdcall SetConsoleMode(void* hConsoleHandle, unsigned dwMode);
        __declspec(dllimport) int __stdcall SetConsoleCP(unsigned wCodePageID);
        __declspec(dllimport) int __stdcall SetConsoleOutputCP(unsigned wCodePageID);
    }
#endif

    auto init() noexcept -> bool {
        #ifdef _WIN32
            constexpr unsigned CP_UTF8 = 65001;
            constexpr unsigned STD_OUTPUT_HANDLE = (unsigned)-11;
            const void* INVALID_HANDLE_VALUE = (void*)(uintptr_t)-1;
            constexpr unsigned ENABLE_VIRTUAL_TERMINAL_PROCESSING = 0x0004;

            void* hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
            if(hOutput == INVALID_HANDLE_VALUE)
                return false;

            unsigned dwMode = 0;
            if (!GetConsoleMode(hOutput, &dwMode))
                return false;

            if (!SetConsoleMode(hOutput, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
                return false;

            SetConsoleCP(CP_UTF8);
            SetConsoleOutputCP(CP_UTF8);
        #endif
        return true;
    }

    template <class... Args>
    void print(const std::format_string<Args...> fmt, Args&&... args) {
        fputs(std::format(fmt, std::forward<Args>(args)...).c_str(), stdout);
    }

    template <class... Args>
    void println(const std::format_string<Args...> fmt, Args&&... args) {
        puts(std::format(fmt, std::forward<Args>(args)...).c_str());
    }

    template <class... Args>
    void info(const std::format_string<Args...> fmt, Args&&... args) {
        console::println("{}[Info]{} {}", ansi::green, ansi::reset, std::format(fmt, std::forward<Args>(args)...));
    }
}
