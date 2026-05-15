module;

#include <unordered_map>
#include <algorithm>
#include <string>
#include <string_view>
#include <optional>
#include <charconv>
#include <vector>
#include <memory>
#include <utility>
#include <span>

export module cmdline;

template <typename T> requires std::is_arithmetic_v<T>
auto parse_string(std::string_view s) -> std::optional<T> {
    T value;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), value);
    if (ec == std::errc())
        return value;
    return std::nullopt;
}


#ifdef _WIN32
    #if defined(_MSC_VER)
        #pragma comment(lib, "shell32.lib")
    #endif
    extern "C" {
        __declspec(dllimport) wchar_t* __stdcall GetCommandLineW();
        __declspec(dllimport) wchar_t** __stdcall CommandLineToArgvW(const wchar_t* lpCmdLine, int* pNumArgs);
        __declspec(dllimport) void* __stdcall LocalFree(void* hMem);
        __declspec(dllimport) int __stdcall WideCharToMultiByte(
            unsigned CodePage, unsigned long dwFlags, const wchar_t* lpWideCharStr, int cchWideChar, 
            char* lpMultiByteStr, int cbMultiByte, const char* lpDefaultChar, int* lpUsedDefaultChar
        );
    }
#endif


export namespace cmdline {
    struct value_t {
        std::optional<std::string_view> str_val = std::nullopt;

        explicit operator bool() const { return str_val.has_value(); }
        auto operator==(std::string_view val) const -> bool { return str_val.value_or("") == val; }
        auto operator|(std::string_view default_value) const { return std::string(str_val.has_value() ? str_val.value() : default_value); }
        auto operator|(value_t default_value) const -> value_t { return str_val.has_value() ? *this : default_value; }

        template <typename T> requires std::is_arithmetic_v<T>
        auto operator|(T default_value) const -> T {
            return str_val.and_then(parse_string<T>).value_or(default_value);
        }
    };

    struct parser_t {
        std::vector<std::string> pos;
        std::vector<std::pair<std::string, std::string>> opts;

        parser_t(std::span<std::string> args) {
            for (int i = 1; i < std::ssize(args); i++) {
                std::string& arg = args[i];

                if (arg.starts_with("-")) {
                    if (auto eq = arg.find('='); eq != std::string::npos)
                        opts.emplace_back(arg.substr(0, eq), arg.substr(eq + 1));

                    else if (i + 1 < std::ssize(args) && !args[i + 1].starts_with("-"))
                        opts.emplace_back(arg, args[++i]);

                    else opts.emplace_back(arg, "");
                }
                
                else pos.push_back(arg);
            }
        }
        
        auto operator[](unsigned i) const -> value_t {
            return i < pos.size() ? value_t(pos[i]) : value_t();
        }
        
        auto operator[](std::string_view name) const -> value_t {
            if (auto it = std::ranges::find(opts, name, &decltype(opts)::value_type::first); it != opts.end())
                return value_t(it->second);
            return value_t();
        }
    };

    auto parse(int argc, char* argv[]) -> parser_t {
        #ifdef _WIN32
            constexpr unsigned CP_UTF8 = 65001;

            auto wargv = std::unique_ptr<wchar_t*[], decltype([](wchar_t** p) { LocalFree(p); })>(CommandLineToArgvW(GetCommandLineW(), &argc));
            auto args = std::vector<std::string>(argc);

            for(int i = 0; i < argc; i++) {
                int len = WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, nullptr, 0, nullptr, nullptr);
                args[i].resize_and_overwrite(len, [&](char* buf, size_t len) {
                    return WideCharToMultiByte(CP_UTF8, 0, wargv[i], -1, buf, len, nullptr, nullptr) - 1;
                });
            }
        #else
            auto args = std::vector<std::string>(argv, argv + argc);
        #endif

        return parser_t(args);
    }
}