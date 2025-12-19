#include "FileCompressor.h"
#include <fstream>
#include <algorithm>
#include <iostream>

/**
 * @details
 * Decompresses a file previously compressed with the Shannon–Fano algorithm.
 * The function loads the code table from the archive, reconstructs the mapping
 * between bit sequences and symbols, and writes the decoded content to the output file.
 */
void FileCompressor::decompress(const std::string& filename_in, const std::string& filename_out) {

    std::fstream in(filename_in, std::ios::in | std::ios::binary);
    check_file_opened(in, filename_in);
    std::fstream out(filename_out, std::ios::out | std::ios::binary);
    check_file_opened(out, filename_out);

    load_archived(in);
    decode_bitstream(in, out);
}

/**
 * @details
 * Performs full compression process:
 * 1. Counts symbol frequencies in the input file.
 * 2. Sorts symbols by frequency.
 * 3. Generates Shannon–Fano codes recursively.
 * 4. Saves the code table and encoded data to an archive file.
 */
void FileCompressor::compress(const std::string& filename_in, const std::string& filename_out) {
    count_occurances(filename_in);
    sort_occurances();
    do_Fano_Algorithm();
    save_archived(filename_out, filename_in);
}

/**
 * @details
 * Writes the final archive file, including:
 * - the encoded code table (symbols and their bit codes);
 * - the encoded bitstream representing the compressed data.
 */
void FileCompressor::save_archived(const std::string& filename, const std::string& input_file) const {
    std::fstream out(filename, std::ios::out | std::ios::binary);
    check_file_opened(out, filename);

    write_code_table(out);
    write_encoded_data(out, input_file);
}


/**
 * @details
 * Recursively splits a symbol-frequency table into two parts of approximately equal total frequency,
 * assigning `0` and `1` prefixes to left and right halves. This constructs the Shannon–Fano code tree.
 */
void FileCompressor::do_Fano_Algorithm(std::optional<std::span<std::pair<uint8_t, size_t>>> vec, size_t vec_sum) {

    if (!vec) {
        if (occurrences.size() <= 1) {
            supplement_codes(occurrences, 0);
            return;
        }
        vec = std::span(occurrences);
        vec_sum = occur_sum;
    }
    auto occur = *vec;

    if (occur.size() == 1) return;

    size_t sum_prev = 0, sum_cur = 0, i = 0;

    while (i < occur.size() && sum_cur + occur[i].second <= vec_sum / 2) {
        sum_cur += occur[i++].second;
    }
    size_t left_sum = sum_cur;
    size_t right_sum = vec_sum - left_sum;

    if (i == 0) i = 1; 
    if (i >= occur.size()) i = occur.size() - 1; 

    supplement_codes(occur.subspan(0, i), 0);
    supplement_codes(occur.subspan(i), 1);

    do_Fano_Algorithm(occur.subspan(0, i), left_sum);
    do_Fano_Algorithm(occur.subspan(i), right_sum);

}

/**
 * @details
 * Verifies that a file stream is successfully opened. Throws an exception if it fails.
 */
void FileCompressor::check_file_opened(std::fstream& file, const std::string& filename) {
    if (!file.is_open()) {
        throw std::runtime_error("File: " + filename + " opening error");
    }
}

/**
 * @details
 * Counts the frequency of each byte (0–255) in the specified input file.
 * This information is later used to build the Shannon–Fano coding tree.
 */
void FileCompressor::count_occurances(const std::string& filename) {
    std::fstream file(filename, std::ios::binary | std::ios::in);
    check_file_opened(file, filename);
    uint8_t character;
    while (file.read(reinterpret_cast<char*>(&character), 1)) {
        auto it = std::find_if(occurrences.begin(), occurrences.end(),
            [character](const auto& pair) {
                return pair.first == character;
            });
        if (it != occurrences.end()) {
            it->second++;
        }
        else {
            occurrences.emplace_back( character, 1 );
        }
        occur_sum++;
    }
}


/**
 * @details
 * Prints all symbol–code mappings to standard output for debugging and analysis.
 */
void FileCompressor::print_codes() const {
    for (const auto& [symbol, code] : codes) {
        std::cout << symbol << ": ";
        for (const auto& y : code) {
            std::cout << y;
        }
        std::cout << '\n';
    }
}

/**
 * @details
 * Sorts the symbol–frequency pairs in descending order by occurrence count.
 */
void FileCompressor::sort_occurances() {
    std::sort(occurrences.begin(), occurrences.end(),
        [](const auto& pair1, const auto& pair2) {
            return pair1.second > pair2.second;
        });
}

/**
 * @details
 * Adds a prefix bit (`0` or `1`) to all codes within a given subset of symbols.
 */
void FileCompressor::supplement_codes(std::optional<std::span<std::pair<uint8_t, size_t>>> span, bool prefix) {
    if (!span) return;
    for (const auto& x : *span) {
        codes[x.first].push_back(prefix);
    }
}

/**
 * @details
 * Serializes the code table into the archive, writing:
 * - number of entries,
 * - each symbol,
 * - bit length,
 * - packed bit representation of the code.
 */
void FileCompressor::write_code_table(std::fstream& out) const {
    uint8_t tableSize = static_cast<uint8_t>(codes.size());
    out.write(reinterpret_cast<const char*>(&tableSize), sizeof(tableSize));

    for (const auto& [symbol, bits] : codes) {
        out.write(reinterpret_cast<const char*>(&symbol), sizeof(symbol));
        uint8_t bitCount = static_cast<uint8_t>(bits.size());
        out.write(reinterpret_cast<const char*>(&bitCount), sizeof(bitCount));

        uint8_t buffer = 0;
        int bitPos = 0;
        for (bool bit : bits) {
            buffer |= (bit << (7 - bitPos));
            bitPos++;
            if (bitPos == 8) {
                out.write(reinterpret_cast<const char*>(&buffer), 1);
                buffer = 0;
                bitPos = 0;
            }
        }
        if (bitPos != 0) {
            out.write(reinterpret_cast<const char*>(&buffer), 1);
        }
    }
}


/**
 * @details
 * Encodes the input file’s content using the generated codes and writes the result as a bitstream.
 */
void FileCompressor::write_encoded_data(std::fstream& out, const std::string& input_file) const {
    std::fstream in(input_file, std::ios::in | std::ios::binary);
    check_file_opened(in, input_file);

    size_t totalBits = 0;
    std::vector<uint8_t> data;
    uint8_t ch;
    while (in.read(reinterpret_cast<char*>(&ch), 1)) {
        data.push_back(ch);
        totalBits += static_cast<size_t>(codes.at(ch).size());
    }

    out.write(reinterpret_cast<const char*>(&totalBits), sizeof(totalBits));

    uint8_t buffer = 0;
    int bitPos = 0;
    for (uint8_t c : data) {
        const auto& bits = codes.at(c);
        for (bool bit : bits) {
            buffer |= (bit << (7 - bitPos));
            bitPos++;
            if (bitPos == 8) {
                out.write(reinterpret_cast<const char*>(&buffer), 1);
                buffer = 0;
                bitPos = 0;
            }
        }
    }
    if (bitPos != 0) {
        out.write(reinterpret_cast<const char*>(&buffer), 1);
    }
}

/**
 * @details
 * Loads the symbol–code mapping from an archive file previously generated by the compressor.
 */
void FileCompressor::load_archived(std::fstream& file) {
    uint8_t tableSize;
    file.read(reinterpret_cast<char*>(&tableSize), sizeof(tableSize));
    codes.clear();

    for (int i = 0; i < tableSize; ++i) {
        uint8_t symbol, bitCount;
        file.read(reinterpret_cast<char*>(&symbol), 1);
        file.read(reinterpret_cast<char*>(&bitCount), 1);

        std::vector<bool> bits;
        bits.reserve(bitCount);
        int bitsRead = 0;
        while (bitsRead < bitCount) {
            uint8_t byte;
            file.read(reinterpret_cast<char*>(&byte), 1);
            for (int b = 0; b < 8 && bitsRead < bitCount; ++b) {
                bool bit = (byte >> (7 - b)) & 1;
                bits.push_back(bit);
                bitsRead++;
            }
        }
        codes[symbol] = bits;
    }
}

/**
 * @details
 * Builds a reverse mapping from bit sequences to symbols for use during decompression.
 */
std::map<std::vector<bool>, uint8_t> FileCompressor::make_code_to_symbol_map() const {
    std::map<std::vector<bool>, uint8_t> code_to_symbol;
    for (const auto& [symbol, bits] : codes) {
        code_to_symbol[bits] = symbol;
    }
    return code_to_symbol;
}

/**
 * @details
 * Decodes a bitstream from the input archive back into raw bytes using the previously reconstructed code table.
 */
void FileCompressor::decode_bitstream(std::fstream& in, std::fstream& out) {
    std::map<std::vector<bool>, uint8_t> code_to_symbol;
    code_to_symbol = make_code_to_symbol_map();

    size_t totalBits;
    in.read(reinterpret_cast<char*>(&totalBits), sizeof(totalBits));

    std::vector<bool> bitsBuffer;
    bitsBuffer.reserve(256);

    uint8_t byte;
    int bitPos = 8;
    size_t bitsRead = 0;

    while (bitsRead < totalBits) {
        if (bitPos == 8) {
            if (!in.read(reinterpret_cast<char*>(&byte), 1)) break;
            bitPos = 0;
        }

        bool bit = (byte >> (7 - bitPos)) & 1;
        bitsBuffer.push_back(bit);
        bitPos++;
        bitsRead++;

        auto it = code_to_symbol.find(bitsBuffer);
        if (it != code_to_symbol.end()) {
            out << it->second;
            std::cout << it->second;
            bitsBuffer.clear();
        }
    }
}
