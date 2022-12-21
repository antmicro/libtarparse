# tarparse

Copyright (c) 2022 Antmicro

tarparse is a small, object oriented, template-based C++ library for extracting file data from a tarball stream. It was made with the following considerations in mind:

- No allocations during parse time
- Freestanding - no dependencies on host facilities
- No stream backtracking
- Stream-based file extraction (extracted files may not fit within a device's available memory)


# Limitations

The library currently has the following limitations:

- Only UStar format is supported
- UStar long filenames via prefix field is not supported
- Not all metadata fields about the file are currently exposed via the callback (only filename and file size)

Some important things to consider when using this library in an embedded context:

- The parser directly contains a 512-byte buffer for reconstructing the current tar block and slightly more than 100 bytes for the metadata structure. This might cause problems in devices with lower stack sizes. When in doubt, use a custom heap and allocate the  parser there, or increase the stack size.

# Usage

To use the parser, create an object that derives from the ```tarparse::TarParser``` class and add an ```on_file_contents``` method with the following signature:

```cpp
struct FileExtractor : public tarparse::TarParser<FileExtractor> {
    void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len) {
        //  Do things with the contents of some file
    }
};
```


After instantiating the object, you can feed the parser by calling the ```Tarparser::update``` method on the base class:

```cpp
const uint8_t data[] = { /* tar stream */ };

FileExtractor ex {};
const tarparse::ParserError error = ex.update(data, sizeof(data));
```

The ```TarParser::update``` base class method returns a status code that can be used to check whether the archive stream was well-formed so far. As new file contents appear in the archive stream, tarparse will call the ```on_file_contents``` of your derived class. By utilizing the ```tarparse::FileMeta``` structure, you can inspect the metadata of the file currently being extracted. For example, to only extract the contents of a specific named file:

```cpp
struct CustomFileExtractor : public tarparse::TarParser<CustomFileExtractor> {
     void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len) {
        if(strcmp(meta.name, "myfile.txt") != 0) {
            return;
        }
        //  Do things with the contents of file 'myfile.txt' in the archive
    }
};
```

# Advanced usage - extracting a nested tar

This library uses the Curiously Recurring Template Pattern to simplify sending data between the library and user functions. C-style function-with-data-pointer callbacks work well for simple cases, however long chains of operations (i.e stacking multiple callbacks together) require a lot of boilerplate code. By utilizing CRTP, you can easily extract multiple nested archives without too much boilerplate:

```cpp
struct InnerFileExtractor : public tarparse::TarParser<InnerFileExtractor> {
    void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len) {
        //  Callback for the contents of the inner archive
    }
};
struct OuterFileExtractor : public tarparse::TarParser<OuterFileExtractor> {
    InnerFileExtractor bar_extractor {};

    void on_file_contents(tarparse::FileMeta const& meta, uint8_t const* filedata, size_t len) {
        //  Callback for the contents of the outer archive
        if(strcmp(meta.name, "inner.tar") != 0) {
            return;
        }

        //  Ignoring errors for clarity
        (void)bar_extractor.update(filedata, len);
    }
};

```

# Development

## Formatting

The library uses clang-format for code formatting. Make sure to re-run clang-format (using the provided style file) after making any changes to the sources.

## Running tests

To run the provided test suite, run the provided ```run_tests.sh``` script in the ```scripts``` folder.

## Running the fuzzer

A fuzzing target utilizing the [libFuzzer](https://llvm.org/docs/LibFuzzer.html) library is provided. To run the fuzzer, run the provided ```run_fuzzer.sh``` script in the ```scripts``` folder. Corpus to be used for fuzzing can be added to the ```src/fuzz/corpus``` directory.

Note: this requires clang to be installed.

# License

This library is provided under the Apache-2.0 license. See the LICENSE file for more details.
