#include "HuffmanSystem.h"
#include "Utils.h"

#include <cctype>
#include <climits>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>

namespace {

void showMenu() {
    std::cout << "\n========================================\n"
              << "       Huffman Coding System Menu\n"
              << "========================================\n"
              << "1. Input text manually\n"//请输入文本
              << "2. Show frequency table\n"//频率表
              << "3. Show Huffman code table\n"//哈夫曼编码表
              << "4. Show Huffman tree\n"//哈夫曼树
              << "5. Encode current text\n"//编码
              << "6. Decode current code\n"//解码
              << "7. Show compression analysis\n"//压缩分析
              << "8. Clear current data\n"//清除
              << "9. Load text from file\n"//从文件中加载
              << "10. Save encoded result to file\n"//保存编码结果到文件中
              << "11. Save decoded result to file\n"//保存解码结果到文件中
              << "12. Save code table to file\n"//保存编码表到文件中
              << "13. Save transmission packet\n"//保存压缩报
              << "14. Load transmission packet\n"
              << "15. Decode packet\n"
              << "0. Exit\n"
              << "Enter your choice: ";
}

bool parseMenuChoice(const std::string& input, int& choice) {
    std::istringstream stream(input);
    stream >> std::ws;

    if (!(stream >> choice)) {
        return false;
    }

    stream >> std::ws;
    return stream.eof();
}

bool saveTextFile(const std::string& path, const std::string& content) {
    std::ofstream outputFile(path.c_str());
    if (!outputFile.is_open()) {
        return false;
    }

    outputFile << content;
    return outputFile.good();
}

bool saveCodeTableFile(
    const std::string& path,
    const std::map<char, int>& frequencyTable,
    const std::map<char, std::string>& codeTable) {
    std::ofstream outputFile(path.c_str());
    if (!outputFile.is_open()) {
        return false;
    }

    outputFile << std::left << std::setw(20) << "Character"
               << std::setw(14) << "Frequency"
               << "Code\n";
    outputFile << "------------------------------------------------\n";

    for (const auto& item : codeTable) {
        const auto frequency = frequencyTable.find(item.first);
        if (frequency == frequencyTable.end()) {
            return false;
        }

        outputFile << std::left << std::setw(20)
                   << formatCharacter(item.first)
                   << std::setw(14) << frequency->second
                   << item.second << '\n';
    }

    return outputFile.good();
}

enum class PacketLoadResult {
    Success,
    FileOpenFailed,
    FormatError
};

std::string packetSymbolToken(char character) {
    switch (character) {
        case ' ':
            return "[space]";
        case '\n':
            return "[newline]";
        case '\t':
            return "[tab]";
        case '\r':
            return "[carriage-return]";
        default:
            break;
    }

    const unsigned char value = static_cast<unsigned char>(character);
    if (std::isprint(value) != 0) {
        return std::string(1, character);
    }

    std::ostringstream stream;
    stream << "0x" << std::uppercase << std::hex << std::setw(2)
           << std::setfill('0') << static_cast<int>(value);
    return stream.str();
}

bool parsePacketSymbol(const std::string& token, char& character) {
    if (token == "[space]") {
        character = ' ';
        return true;
    }
    if (token == "[newline]") {
        character = '\n';
        return true;
    }
    if (token == "[tab]") {
        character = '\t';
        return true;
    }
    if (token == "[carriage-return]") {
        character = '\r';
        return true;
    }
    if (token.size() == 1) {
        character = token[0];
        return true;
    }

    if (token.size() == 4 && token[0] == '0' &&
        (token[1] == 'x' || token[1] == 'X')) {
        unsigned int value = 0;
        std::istringstream stream(token.substr(2));
        stream >> std::hex >> value;
        if (stream && stream.eof() && value <= 0xFFU) {
            character = static_cast<char>(static_cast<unsigned char>(value));
            return true;
        }
    }

    return false;
}

bool parsePacketLength(const std::string& line,
                       const std::string& label,
                       std::size_t& value) {
    if (line.compare(0, label.size(), label) != 0) {
        return false;
    }

    std::istringstream stream(line.substr(label.size()));
    unsigned long long parsedValue = 0;
    stream >> std::ws;
    if (!(stream >> parsedValue)) {
        return false;
    }
    stream >> std::ws;

    if (!stream.eof() ||
        parsedValue > std::numeric_limits<std::size_t>::max()) {
        return false;
    }

    value = static_cast<std::size_t>(parsedValue);
    return true;
}

bool isBinaryString(const std::string& data) {
    if (data.empty()) {
        return false;
    }

    for (char bit : data) {
        if (bit != '0' && bit != '1') {
            return false;
        }
    }
    return true;
}

bool saveTransmissionPacket(
    const std::string& path,
    std::size_t originalLength,
    const std::string& encodedData,
    const std::map<char, int>& frequencyTable,
    const std::map<char, std::string>& codeTable) {
    std::ofstream outputFile(path.c_str());
    if (!outputFile.is_open()) {
        return false;
    }

    outputFile << "===== HUFFMAN PACKET =====\n"
               << "OriginalLength: " << originalLength << '\n'
               << "EncodedLength: " << encodedData.size() << "\n\n"
               << "[CodeTable]\n";

    for (const auto& item : codeTable) {
        const auto frequency = frequencyTable.find(item.first);
        if (frequency == frequencyTable.end()) {
            return false;
        }

        outputFile << packetSymbolToken(item.first) << ' '
                   << frequency->second << ' '
                   << item.second << '\n';
    }

    outputFile << "\n[EncodedData]\n"
               << encodedData << '\n';
    return outputFile.good();
}

PacketLoadResult loadTransmissionPacket(
    const std::string& path,
    std::size_t& originalLength,
    std::size_t& encodedLength,
    std::map<char, int>& frequencyTable,
    std::map<char, std::string>& codeTable,
    std::string& encodedData) {
    std::ifstream inputFile(path.c_str());
    if (!inputFile.is_open()) {
        return PacketLoadResult::FileOpenFailed;
    }

    std::string line;
    std::size_t parsedOriginalLength = 0;
    std::size_t parsedEncodedLength = 0;
    std::map<char, int> parsedFrequencyTable;
    std::map<char, std::string> parsedCodeTable;
    std::map<std::string, char> reverseCodeTable;
    std::string parsedEncodedData;

    if (!std::getline(inputFile, line) ||
        line != "===== HUFFMAN PACKET =====" ||
        !std::getline(inputFile, line) ||
        !parsePacketLength(line, "OriginalLength:", parsedOriginalLength) ||
        !std::getline(inputFile, line) ||
        !parsePacketLength(line, "EncodedLength:", parsedEncodedLength)) {
        return PacketLoadResult::FormatError;
    }

    bool foundCodeTable = false;
    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (line == "[CodeTable]") {
            foundCodeTable = true;
            break;
        }
        return PacketLoadResult::FormatError;
    }
    if (!foundCodeTable) {
        return PacketLoadResult::FormatError;
    }

    bool foundEncodedData = false;
    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (line == "[EncodedData]") {
            foundEncodedData = true;
            break;
        }

        std::istringstream row(line);
        std::string symbolToken;
        std::string code;
        unsigned long long frequency = 0;
        char character = '\0';

        if (!(row >> symbolToken >> frequency >> code)) {
            return PacketLoadResult::FormatError;
        }
        row >> std::ws;

        if (!row.eof() || frequency == 0 ||
            frequency > static_cast<unsigned long long>(
                            std::numeric_limits<int>::max()) ||
            !parsePacketSymbol(symbolToken, character) ||
            !isBinaryString(code) ||
            parsedCodeTable.count(character) != 0 ||
            reverseCodeTable.count(code) != 0) {
            return PacketLoadResult::FormatError;
        }

        parsedFrequencyTable[character] = static_cast<int>(frequency);
        parsedCodeTable[character] = code;
        reverseCodeTable[code] = character;
    }

    if (!foundEncodedData || parsedCodeTable.empty()) {
        return PacketLoadResult::FormatError;
    }

    while (std::getline(inputFile, line)) {
        if (line.empty()) {
            continue;
        }
        if (!isBinaryString(line)) {
            return PacketLoadResult::FormatError;
        }
        parsedEncodedData += line;
    }

    if (inputFile.bad() || parsedOriginalLength == 0 ||
        parsedEncodedLength == 0 ||
        parsedEncodedData.size() != parsedEncodedLength) {
        return PacketLoadResult::FormatError;
    }

    std::size_t totalFrequency = 0;
    for (const auto& item : parsedFrequencyTable) {
        const std::size_t frequency = static_cast<std::size_t>(item.second);
        if (totalFrequency > parsedOriginalLength ||
            frequency > parsedOriginalLength - totalFrequency) {
            return PacketLoadResult::FormatError;
        }
        totalFrequency += frequency;
    }
    if (totalFrequency != parsedOriginalLength) {
        return PacketLoadResult::FormatError;
    }

    for (const auto& first : parsedCodeTable) {
        for (const auto& second : parsedCodeTable) {
            if (first.first != second.first &&
                second.second.compare(0,
                                      first.second.size(),
                                      first.second) == 0) {
                return PacketLoadResult::FormatError;
            }
        }
    }

    originalLength = parsedOriginalLength;
    encodedLength = parsedEncodedLength;
    frequencyTable = parsedFrequencyTable;
    codeTable = parsedCodeTable;
    encodedData = parsedEncodedData;
    return PacketLoadResult::Success;
}

bool decodeTransmissionPacket(
    const std::string& encodedData,
    const std::map<char, std::string>& codeTable,
    std::string& result) {
    std::map<std::string, char> reverseCodeTable;
    for (const auto& item : codeTable) {
        if (reverseCodeTable.count(item.second) != 0) {
            return false;
        }
        reverseCodeTable[item.second] = item.first;
    }

    result.clear();
    std::string prefix;
    for (char bit : encodedData) {
        if (bit != '0' && bit != '1') {
            result.clear();
            return false;
        }

        prefix += bit;
        const auto match = reverseCodeTable.find(prefix);
        if (match != reverseCodeTable.end()) {
            result += match->second;
            prefix.clear();
        }
    }

    if (!prefix.empty()) {
        result.clear();
        return false;
    }
    return true;
}

}  // namespace

int main() {
    HuffmanSystem system;

    std::string text;
    std::string encoded;
    std::string decoded;
    bool hasText = false;
    bool hasEncoded = false;
    bool hasDecoded = false;

    std::size_t packetOriginalLength = 0;
    std::size_t packetEncodedLength = 0;
    std::map<char, int> packetFrequencyTable;
    std::map<char, std::string> packetCodeTable;
    std::string packetEncodedData;
    bool hasPacket = false;

    const auto initializeText = [&](const std::string& newText) {
        text = newText;
        system.input(text);
        system.buildTree();
        system.generateCode();

        encoded.clear();
        decoded.clear();
        hasText = system.isReady();
        hasEncoded = false;
        hasDecoded = false;

        packetOriginalLength = 0;
        packetEncodedLength = 0;
        packetFrequencyTable.clear();
        packetCodeTable.clear();
        packetEncodedData.clear();
        hasPacket = false;
    };

    bool running = true;
    while (running) {
        showMenu();

        std::string choiceInput;
        if (!std::getline(std::cin, choiceInput)) {
            std::cout << "\nInput stream closed. Exiting." << std::endl;
            break;
        }

        int choice = -1;
        if (!parseMenuChoice(choiceInput, choice)) {
            std::cin.clear();
            std::cout << "Invalid input." << std::endl;
            continue;
        }

        switch (choice) {
            case 1: {
                std::cout << "Enter the text to process: ";
                std::string newText;
                if (!std::getline(std::cin, newText)) {
                    std::cin.clear();
                    std::cout << "Invalid input." << std::endl;
                    break;
                }

                if (newText.empty()) {
                    std::cout << "Input text cannot be empty." << std::endl;
                    break;
                }

                initializeText(newText);
                if (hasText) {
                    std::cout << "Text loaded and Huffman tree built successfully."
                              << std::endl;
                } else {
                    text.clear();
                    std::cout << "Failed to initialize Huffman data."
                              << std::endl;
                }
                break;
            }

            case 2:
                if (!hasText) {
                    std::cout << "Please input text first." << std::endl;
                } else {
                    showFrequencyTable(system.getFreqTable());
                }
                break;

            case 3:
                if (!hasText) {
                    std::cout << "Please input text first." << std::endl;
                } else {
                    showCodeTable(system.getCodeTable());
                }
                break;

            case 4:
                if (!hasText) {
                    std::cout << "Please input text first." << std::endl;
                } else {
                    showTree(system.getRoot());
                }
                break;

            case 5:
                if (!hasText) {
                    std::cout << "Please input text first." << std::endl;
                    break;
                }

                encoded = system.encode(text);
                decoded.clear();
                hasEncoded = !encoded.empty();
                hasDecoded = false;

                if (hasEncoded) {
                    std::cout << "\n========== Encoded Result ==========\n"
                              << encoded << std::endl;
                } else {
                    std::cout << "Encode failed." << std::endl;
                }
                break;

            case 6:
                if (!hasEncoded) {
                    std::cout << "Please encode text first." << std::endl;
                    break;
                }

                decoded.clear();
                hasDecoded = system.decode(encoded, decoded);
                if (hasDecoded) {
                    std::cout << "\n========== Decoded Result ==========\n"
                              << decoded << std::endl;

                    if (decoded == text) {
                        std::cout << "Verification passed: decoded text matches the original text."
                                  << std::endl;
                    } else {
                        std::cout << "Verification failed: decoded text does not match the original text."
                                  << std::endl;
                    }
                } else {
                    std::cout << "Decode failed." << std::endl;
                }
                break;

            case 7: {
                if (!hasEncoded) {
                    std::cout << "Please encode text first." << std::endl;
                    break;
                }

                const std::size_t originalBits = text.size() * CHAR_BIT;
                const std::size_t compressedBits = encoded.size();
                const double compressionRate =
                    system.getCompressionRate(text, encoded);
                showCompression(originalBits, compressedBits, compressionRate);
                break;
            }

            case 8:
                text.clear();
                encoded.clear();
                decoded.clear();
                hasText = false;
                hasEncoded = false;
                hasDecoded = false;
                packetOriginalLength = 0;
                packetEncodedLength = 0;
                packetFrequencyTable.clear();
                packetCodeTable.clear();
                packetEncodedData.clear();
                hasPacket = false;
                system.input(std::string());
                std::cout << "Data cleared." << std::endl;
                break;

            case 9: {
                std::cout << "Enter input file path: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "File open failed." << std::endl;
                    break;
                }

                std::ifstream inputFile(path.c_str());
                if (!inputFile.is_open()) {
                    std::cout << "File open failed." << std::endl;
                    break;
                }

                std::ostringstream buffer;
                buffer << inputFile.rdbuf();
                if (inputFile.bad()) {
                    std::cout << "File open failed." << std::endl;
                    break;
                }

                const std::string loadedText = buffer.str();
                if (loadedText.empty()) {
                    std::cout << "File is empty." << std::endl;
                    break;
                }

                initializeText(loadedText);
                if (hasText) {
                    std::cout << "File loaded successfully." << std::endl;
                } else {
                    text.clear();
                    std::cout << "Failed to initialize Huffman data."
                              << std::endl;
                }
                break;
            }

            case 10: {
                if (!hasEncoded) {
                    std::cout << "Please encode text first." << std::endl;
                    break;
                }

                std::cout << "Enter output file path [encoded.txt]: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "Save failed." << std::endl;
                    break;
                }
                if (path.empty()) {
                    path = "encoded.txt";
                }

                std::cout << (saveTextFile(path, encoded)
                                  ? "Save succeeded."
                                  : "Save failed.")
                          << std::endl;
                break;
            }

            case 11: {
                if (!hasDecoded) {
                    std::cout << "Please decode text first." << std::endl;
                    break;
                }

                std::cout << "Enter output file path [decoded.txt]: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "Save failed." << std::endl;
                    break;
                }
                if (path.empty()) {
                    path = "decoded.txt";
                }

                std::cout << (saveTextFile(path, decoded)
                                  ? "Save succeeded."
                                  : "Save failed.")
                          << std::endl;
                break;
            }

            case 12: {
                if (!hasText) {
                    std::cout << "Please input text first." << std::endl;
                    break;
                }

                std::cout << "Enter output file path [code_table.txt]: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "Save failed." << std::endl;
                    break;
                }
                if (path.empty()) {
                    path = "code_table.txt";
                }

                std::cout
                    << (saveCodeTableFile(path,
                                          system.getFreqTable(),
                                          system.getCodeTable())
                            ? "Save succeeded."
                            : "Save failed.")
                    << std::endl;
                break;
            }

            case 13: {
                if (!hasEncoded) {
                    std::cout << "Please encode text first." << std::endl;
                    break;
                }

                std::cout << "Enter packet file path [packet.txt]: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "Save failed." << std::endl;
                    break;
                }
                if (path.empty()) {
                    path = "packet.txt";
                }

                std::cout
                    << (saveTransmissionPacket(path,
                                               text.size(),
                                               encoded,
                                               system.getFreqTable(),
                                               system.getCodeTable())
                            ? "Save succeeded."
                            : "Save failed.")
                    << std::endl;
                break;
            }

            case 14: {
                std::cout << "Enter packet file path [packet.txt]: ";
                std::string path;
                if (!std::getline(std::cin, path)) {
                    std::cin.clear();
                    std::cout << "File open failed." << std::endl;
                    break;
                }
                if (path.empty()) {
                    path = "packet.txt";
                }

                std::size_t loadedOriginalLength = 0;
                std::size_t loadedEncodedLength = 0;
                std::map<char, int> loadedFrequencyTable;
                std::map<char, std::string> loadedCodeTable;
                std::string loadedEncodedData;

                const PacketLoadResult loadResult =
                    loadTransmissionPacket(path,
                                           loadedOriginalLength,
                                           loadedEncodedLength,
                                           loadedFrequencyTable,
                                           loadedCodeTable,
                                           loadedEncodedData);

                if (loadResult == PacketLoadResult::FileOpenFailed) {
                    std::cout << "File open failed." << std::endl;
                    break;
                }
                if (loadResult == PacketLoadResult::FormatError) {
                    std::cout << "Packet format error." << std::endl;
                    break;
                }

                packetOriginalLength = loadedOriginalLength;
                packetEncodedLength = loadedEncodedLength;
                packetFrequencyTable = loadedFrequencyTable;
                packetCodeTable = loadedCodeTable;
                packetEncodedData = loadedEncodedData;
                hasPacket = true;

                decoded.clear();
                hasDecoded = false;
                std::cout << "Packet loaded successfully." << std::endl;
                break;
            }

            case 15: {
                if (!hasPacket) {
                    std::cout << "Please load packet first." << std::endl;
                    break;
                }

                std::string packetDecoded;
                const bool decodeSucceeded =
                    packetEncodedData.size() == packetEncodedLength &&
                    decodeTransmissionPacket(packetEncodedData,
                                             packetCodeTable,
                                             packetDecoded) &&
                    packetDecoded.size() == packetOriginalLength;

                if (!decodeSucceeded) {
                    decoded.clear();
                    hasDecoded = false;
                    std::cout << "Decode failed." << std::endl;
                    break;
                }

                decoded = packetDecoded;
                hasDecoded = true;
                std::cout << "\n========== Packet Decoded Result ==========\n"
                          << decoded << std::endl;
                std::cout << "Packet verification passed: decoded length matches OriginalLength."
                          << std::endl;
                break;
            }

            case 0:
                running = false;
                std::cout << "Goodbye." << std::endl;
                break;

            default:
                std::cout << "Invalid input." << std::endl;
                break;
        }
    }

    return 0;
}
