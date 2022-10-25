//
// Copyright (c) 2022 Antmicro
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "internal/numparse.hpp"
#include "internal/util.hpp"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

namespace tarparse {
    //  This represents the metadata sent via the callback to the user.
    struct FileMeta {
        char name[100];
        size_t size;
    };

    //  Errors encountered during tar extraction
    enum class ParserError {
        Ok,
        InvalidHeader,
    };

    /*
     *  Simple streamed tar extractor.
     *  Example usage:
     *  ```
     *      struct FileExtractor : public TarParser<FileExtractor> {
     *          void on_file_contents(TarFileMeta const& meta, uint8_t const* filedata, size_t len) {
     *          }
     *      };
     *      FileExtractor ex {};
     *      ex.update(buf, buflen);
     *  ```
     *  The derived class method ```on_file_contents``` will be called as file data is extracted from the archive.
     *  The expected signature is as shown above.
     *  Things to keep in mind:
     *      - Only ustar format is supported
     *      - Garbage blocks (without the proper ustar magic and version) are ignored.
     *      - The block buffer is stored directly in the class, don't make this on the stack!
     */
    template <typename Derived>
    class TarParser {
    public:
        constexpr TarParser() = default;
        ~TarParser() = default;

        /**
         *  \brief      Feed the parser with archive data.
         *
         *  \param buffer       Buffer containing the next chunk of archive data
         *  \param bufferlen    Length of the buffer
         *
         *  \returns    ```TarParserError::Ok``` on success
         *              Error code on failure
         **/
        [[nodiscard]] constexpr ParserError update(uint8_t const* buffer, size_t bufferlen)
        {
            //  Consume the entire buffer
            while(bufferlen != 0) {
                const auto how_many = tarparse::internal::min(bufferlen, bytes_left());
                memcpy(current(), buffer, how_many);
                advance(how_many);
                buffer += how_many;
                bufferlen -= how_many;

                if(block_ready()) {
                    if(const auto rc = parse_block(); rc != ParserError::Ok) {
                        return rc;
                    }
                    m_cursor = 0;
                }
            }
            return ParserError::Ok;
        }

        /**
         *  \brief       Reset the parser state
         *
         *  Resets the parser state. The following call to update() will behave as if
         *  a completely new TarParser object was created.
         *
         */
        constexpr void reset()
        {
            m_cursor = 0;
            m_state = ParserState::ExpectingFileHeader;
            m_metadata.size = 0;
        }

    private:
        static constexpr const size_t block_size = 512;
        static constexpr const char ustar_magic_string[] = "ustar";
        static constexpr const char ustar_version_string[] = "00";

        enum class ParserState {
            ExpectingFileHeader,
            ExpectingFileContent
        };

        struct TarBlock {
            char file_name[100];
            char file_mode[8];
            char uid[8];
            char gid[8];
            char file_size[12];
            char last_modified[12];
            char checksum[8];
            char file_type[1];
            char file_link_name[100];
            char ustar_magic[6];
            char ustar_version[2];
            char owner_username[32];
            char owner_groupname[32];
            char dev_major[8];
            char dev_minor[8];
            char file_prefix[155];
            char pad[12];
        };
        static_assert(sizeof(TarBlock) == block_size, "TarBlock structure does not have correct size!");

        union {
            TarBlock block;
            uint8_t bytes[block_size];
        } m_data;
        size_t m_cursor { 0 };
        ParserState m_state { ParserState::ExpectingFileHeader };
        FileMeta m_metadata {};
        size_t m_consumed { 0 };

        //  Assumes the cursor never goes out of bounds
        constexpr size_t bytes_left() const
        {
            return block_size - m_cursor;
        }

        //  Pointer to the next byte to write in the block buffer
        constexpr uint8_t* current()
        {
            return m_data.bytes + m_cursor;
        }

        //  Advance the cursor by n bytes
        constexpr void advance(size_t n)
        {
            //  Safeguard
            if(m_cursor + n > block_size) {
                m_cursor = block_size;
                return;
            }

            m_cursor += n;
        }

        //  Do we have enough data to parse the block?
        constexpr bool block_ready() const
        {
            return bytes_left() == 0;
        }

        constexpr bool verify_is_ustar_format() const
        {
            //  Check for the presence of the magic and version strings in the header
            if(memcmp(m_data.block.ustar_magic, ustar_magic_string, sizeof(ustar_magic_string)) != 0) {
                return false;
            }
            //  The ustar version field is not null terminated
            if(memcmp(m_data.block.ustar_version, ustar_version_string, sizeof(ustar_version_string) - 1) != 0) {
                return false;
            }
            return true;
        }

        //  Sanity check values present in the header
        //  Mainly, this ensures that strings stored in the header
        //  are actually null-terminated.
        constexpr ParserError verify_header() const
        {
            if(memchr(m_data.block.file_name, '\0', sizeof(m_data.block.file_name)) == nullptr) {
                return ParserError::InvalidHeader;
            }
            if(memchr(m_data.block.file_size, '\0', sizeof(m_data.block.file_size)) == nullptr) {
                return ParserError::InvalidHeader;
            }
            return ParserError::Ok;
        }

        //  Parse the currently stored block
        //  Should only be called after filling the entire buffer.
        constexpr ParserError parse_block()
        {
            switch(m_state) {
                //  Currently parsed block should be the file header
                case ParserState::ExpectingFileHeader: {
                    //  Check if this looks like a ustar header
                    //  If not, ignore it entirely
                    if(!verify_is_ustar_format()) {
                        return ParserError::Ok;
                    }

                    //  Verify the header
                    if(const auto err = verify_header(); err != ParserError::Ok) {
                        return err;
                    }

                    size_t file_size = 0;
                    if(!tarparse::internal::parse_unsigned(m_data.block.file_size, file_size, tarparse::internal::ParseMode::Octal)) {
                        //  Corrupted or otherwise invalid file size
                        //  Remain in the FileHeader state, awaiting for the next block
                        return ParserError::InvalidHeader;
                    }

                    //  Fill the metadata structure
                    strncpy(m_metadata.name, m_data.block.file_name, sizeof(m_data.block.file_name));
                    m_metadata.size = file_size;
                    m_consumed = 0;
                    m_state = ParserState::ExpectingFileContent;

                    break;
                }
                //  Parsing file data blocks
                case ParserState::ExpectingFileContent: {
                    const auto how_many = tarparse::internal::min(m_metadata.size - m_consumed, block_size);
                    m_consumed += how_many;

                    //  Enforce metadata constness when calling the callback
                    static_cast<Derived*>(this)->on_file_contents(static_cast<FileMeta const&>(m_metadata), m_data.bytes, how_many);

                    //  End of file
                    if(m_consumed == m_metadata.size) {
                        m_state = ParserState::ExpectingFileHeader;
                    }
                }
            }

            return ParserError::Ok;
        }
    };
}
