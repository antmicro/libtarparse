//
// Copyright (c) 2023 Antmicro
// SPDX-License-Identifier: Apache-2.0
//

#include "catch.hpp"
#include "fileio.hpp"
#include <cstdio>
#include <iostream>
#include <optional>
#include <tarparse/parse.hpp>
#include <unordered_map>

TEST_CASE("TarParser")
{
    auto maybe_file = fileio::load_file("test-data/test.tar");
    REQUIRE(maybe_file.has_value());

    auto maybe_corrupted_file = fileio::load_file("test-data/non-terminated-filename.tar");
    REQUIRE(maybe_corrupted_file.has_value());

    std::vector<uint8_t> file = *maybe_file;
    std::vector<uint8_t> corrupted_file = *maybe_corrupted_file;

    SECTION("extracting file from archive")
    {

        struct FileExtractor : public tarparse::TarParser<FileExtractor> {
            std::unordered_map<std::string, bool> seen_files {};
            std::unordered_map<std::string, std::vector<uint8_t>> seen_file_contents {};

            void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len)
            {
                const auto str = std::string { meta.name };
                seen_files[str] = true;
                std::printf("'%s': incoming %d bytes\n", meta.name, len);
                std::copy(filedata, filedata + len, std::back_inserter(seen_file_contents[str]));
            }
        };
        FileExtractor extractor {};
        REQUIRE(extractor.update(file.data(), file.size()) == tarparse::ParserError::Ok);

        SECTION("all files were seen by the parser")
        {
            REQUIRE(extractor.seen_files["version"]);
            REQUIRE(extractor.seen_files["manifest"]);
            REQUIRE(extractor.seen_files["header.tar"]);
            REQUIRE(extractor.seen_files["data/0000.tar"]);
            //  This file is contained inside the header.tar file. If it was seen by the parser,
            //  then the ustar stream somehow got out of sync.
            REQUIRE(!extractor.seen_files["header-info"]);
        }

        SECTION("file contents match")
        {
            auto check_same = [](std::vector<uint8_t> const& vec, std::string file) -> bool {
                auto maybe_file = fileio::load_file(file);
                assert(maybe_file.has_value());
                std::vector<uint8_t> actual_contents = *maybe_file;
                REQUIRE(vec.size() == actual_contents.size());

                return std::equal(vec.begin(), vec.end(), actual_contents.begin(), actual_contents.end());
            };

            REQUIRE(check_same(extractor.seen_file_contents["version"], "test-data/test-tar-extracted/version"));
            REQUIRE(check_same(extractor.seen_file_contents["manifest"], "test-data/test-tar-extracted/manifest"));
            REQUIRE(check_same(extractor.seen_file_contents["header.tar"], "test-data/test-tar-extracted/header.tar"));
            REQUIRE(check_same(extractor.seen_file_contents["data/0000.tar"], "test-data/test-tar-extracted/data/0000.tar"));
        }
    }

    SECTION("invalid tar header")
    {
        struct Dummy : public tarparse::TarParser<Dummy> {
            void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len)
            {
            }
        };
        Dummy parser {};

        REQUIRE(parser.update(corrupted_file.data(), corrupted_file.size()) != tarparse::ParserError::Ok);
    }
}
