#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <tarparse/parse.hpp>

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
    struct Parser : public tarparse::TarParser<Parser> {
        uint8_t sum = 0;

        void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* data, size_t len)
        {
            //  Touch all incoming bytes to catch any potential errors
            for(auto i = 0u; i < len; ++i) {
                sum = sum ^ data[i];
            }
        }
    } parser {};

    (void)parser.update(data, size);
    return 0;
}
