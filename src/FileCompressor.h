#pragma once
#include <vector>
#include <string>
#include <span>
#include <optional>
#include <unordered_map>
#include <map>

/**
 * @brief Total number of ASCII characters.
 * @details
 * This constant defines the size of the ASCII table (0–255),
 * used as the upper bound for symbol codes in the compression algorithm.
 */
constexpr uint16_t ASCII = 256;

/**
 * @class FileCompressor
 * @brief Provides file compression and decompression using the Shannon–Fano algorithm.
 *
 * The FileCompressor class handles symbol frequency analysis, code generation,
 * and input/output operations for compression and decompression.
 */



class FileCompressor {
private:
	/// @brief List of pairs representing symbol and its occurrence count.
	std::vector<std::pair<uint8_t, size_t>> occurrences;
	/// @brief Total number of symbol occurrences in the file.
	size_t occur_sum = 0;
	/// @brief Map of symbol codes (each symbol represented as a vector of bits).
	std::unordered_map<uint8_t, std::vector<bool>> codes{ASCII};


private:
	/**
	 * @brief Checks whether a file stream is successfully opened.
	 * @param file The file stream to check.
	 * @param filename The file name (used for error messages).
	 * @throws std::runtime_error if the file could not be opened.
	 */
	static void check_file_opened(std::fstream& file, const std::string& filename);
	/**
	 * @brief Counts symbol occurrences in the specified file.
	 * @param filename Input file name to analyze.
	 */
	void count_occurances(const std::string& filename);
	/**
	 * @brief Sorts symbol occurrences in descending order.
	 */
	void sort_occurances(); 
	/**
	 * @brief Assigns binary codes to symbols recursively based on Shannon–Fano algorithm.
	 * @param span Optional span of symbol-frequency pairs to process.
	 * @param prefix Prefix flag to differentiate bit assignment.
	 */
	void supplement_codes(std::optional<std::span<std::pair<uint8_t, size_t>>> subset, bool prefix);
	/**
	 * @brief Decodes the bitstream from a compressed file.
	 * @param in Input file stream (compressed data).
	 * @param out Output file stream (decompressed data).
	 */
	void decode_bitstream(std::fstream& in, std::fstream& out);
	/**
	 * @brief Loads compressed file contents and metadata.
	 * @param filename Path to the archived file.
	 */
	void load_archived(std::fstream& file);
	/**
	 * @brief Builds a reverse lookup table mapping bit codes to symbols.
	 * @return A map that converts binary codes back into corresponding characters.
	 */
	std::map<std::vector<bool>, uint8_t> make_code_to_symbol_map() const;
	/**
	 * @brief Saves compressed data and code table into an archive.
	 * @param filename Output archive filename.
	 * @param input_file Original input filename.
	 */
	void save_archived(const std::string& filename, const std::string& input_file) const;
	/**
	 * @brief Performs the Shannon–Fano algorithm to generate optimal binary codes.
	 * @param vec Optional span of symbol-frequency pairs.
	 * @param vec_sum Sum of occurrences in the provided span.
	 */
	void do_Fano_Algorithm(std::optional<std::span<std::pair<uint8_t, size_t>>> vec = std::nullopt, size_t vec_sum = 0);
	/**
	 * @brief Writes the generated code table to the output file stream.
	 * @param out Output file stream.
	 */

	void write_code_table(std::fstream& out) const;
	/**
	 * @brief Writes encoded binary data of the input file to the output stream.
	 * @param out Output file stream.
	 * @param input_file Path to the input file.
	 */
	void write_encoded_data(std::fstream& out, const std::string& input_file) const;

public:
	/**
	 * @brief Prints all generated symbol codes to the console.
	 */
	void print_codes() const;
	/**
	 * @brief Decompresses an archived file into its original form.
	 * @param filename_in Input compressed file.
	 * @param filename_out Output decompressed file.
	 */
	void decompress(const std::string& filename_in, const std::string& filename_out);
	/**
	 * @brief Compresses a file and saves it as an archive.
	 * @param filename_in Input file to compress.
	 * @param filename_out Output archive file.
	 */
	void compress(const std::string& filename_in, const std::string& filename_out);
};
