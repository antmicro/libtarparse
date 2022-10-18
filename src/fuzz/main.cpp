#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <tarparse/parse.hpp>

int main()
{
    struct Parser : public tarparse::TarParser<Parser> {
        void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* data, size_t len)
        {
        }
    } parser {};

    static uint8_t buffer[512] {};
    while(true) {
        const auto count = std::fread(buffer, sizeof(uint8_t), sizeof(buffer), stdin);
        if(count == 0) {
            break;
        }

        (void)parser.update(buffer, count);
    }

    return 0;
}
