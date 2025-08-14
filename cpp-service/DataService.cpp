#include <spdlog/spdlog.h>
#include <string>
#include <cstdint>

int main(int argc, char* argv[]) {
    std::string level = "info";
    std::uint64_t trace_id = 0;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--log-level" && i + 1 < argc) {
            level = argv[++i];
        } else if (arg == "--trace-id" && i + 1 < argc) {
            trace_id = std::stoull(argv[++i]);
        }
    }

    spdlog::set_level(spdlog::level::from_str(level));
    spdlog::info("[DS1001][trace={}]: Data Service running", trace_id);
    return 0;
}
