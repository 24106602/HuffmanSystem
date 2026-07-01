#include "Utils.h"

#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>

std::string formatCharacter(char character) {
    switch (character) {
        case ' ':
            return "[space]";
        case '\n':
            return "[newline]";
        case '\t':
            return "[tab]";
        case '\r':
            return "[carriage return]";
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

void showFrequencyTable(const std::map<char, int>& freqTable) {
    std::cout << "\n========== Character Frequency Table ==========" << std::endl;

    if (freqTable.empty()) {
        std::cout << "No frequency data is available." << std::endl;
        return;
    }

    std::cout << std::left << std::setw(20) << "Character"
              << std::setw(10) << "Frequency" << std::endl;
    std::cout << "--------------------------" << std::endl;

    for (const auto& item : freqTable) {
        std::cout << std::left << std::setw(20)
                  << formatCharacter(item.first)
                  << std::setw(10) << item.second << std::endl;
    }
}

void showCodeTable(const std::map<char, std::string>& codeTable) {
    std::cout << "\n========== Huffman Code Table ==========" << std::endl;

    if (codeTable.empty()) {
        std::cout << "No code table is available." << std::endl;
        return;
    }

    std::cout << std::left << std::setw(20) << "Character"
              << std::setw(20) << "Code" << std::endl;
    std::cout << "------------------------------------" << std::endl;

    for (const auto& item : codeTable) {
        std::cout << std::left << std::setw(20)
                  << formatCharacter(item.first)
                  << std::setw(20) << item.second << std::endl;
    }
}

void showTree(const Node* root, int depth) {
    if (depth == 0) {
        std::cout << "\n========== Huffman Tree ==========" << std::endl;
        if (root == nullptr) {
            std::cout << "No Huffman tree is available." << std::endl;
            return;
        }
        std::cout << "The right subtree is shown above the left subtree; *(n) is an internal node with weight n."
                  << std::endl;
        std::cout << "--------------------------------" << std::endl;
    }

    if (root == nullptr) {
        return;
    }

    showTree(root->right, depth + 1);

    for (int level = 0; level < depth; ++level) {
        std::cout << "    ";
    }

    if (root->left != nullptr || root->right != nullptr) {
        std::cout << "*(" << root->freq << ")" << std::endl;
    } else {
        std::cout << formatCharacter(root->ch)
                  << "(" << root->freq << ")" << std::endl;
    }

    showTree(root->left, depth + 1);
}

void showCompression(std::size_t originalBits,
                     std::size_t compressedBits,
                     double compressionRate) {
    std::cout << "\n========== Compression Analysis ==========" << std::endl;

    if (originalBits == 0) {
        std::cout << "The original data is empty; the compression ratio cannot be calculated."
                  << std::endl;
        return;
    }

    const double savingRate = 1.0 - compressionRate;
    std::cout << "Original size: " << originalBits << " bits" << std::endl;
    std::cout << "Encoded size: " << compressedBits << " bits" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Compression ratio: " << compressionRate * 100.0 << "%" << std::endl;
    std::cout << "Space saving rate: " << savingRate * 100.0 << "%" << std::endl;
    std::cout << "Note: the calculation includes only Huffman-coded data bits, not the code table or file header."
              << std::endl;
}

void showEntropyAnalysis(const std::map<char, int>& freqTable,
                         const std::map<char, std::string>& codeTable,
                         std::size_t totalChars) {
    if (freqTable.empty() || codeTable.empty() || totalChars == 0) {
        std::cout << "Please input or load message first." << std::endl;
        return;
    }

    double entropy = 0.0;
    double averageCodeLength = 0.0;

    for (const auto& item : freqTable) {
        const auto code = codeTable.find(item.first);
        if (item.second <= 0 || code == codeTable.end()) {
            std::cout << "Please input or load message first." << std::endl;
            return;
        }

        const double probability =
            static_cast<double>(item.second) /
            static_cast<double>(totalChars);
        entropy -= probability * (std::log(probability) / std::log(2.0));
        averageCodeLength +=
            probability * static_cast<double>(code->second.size());
    }

    double efficiency = 0.0;
    if (averageCodeLength > 0.0) {
        efficiency = entropy / averageCodeLength * 100.0;
    }
    if (efficiency > 100.0 && efficiency < 100.000001) {
        efficiency = 100.0;
    }

    const double redundancy = efficiency <= 100.0
                                  ? 100.0 - efficiency
                                  : 0.0;

    std::cout << "\n========== Entropy Analysis ==========" << std::endl;
    std::cout << "Total characters: " << totalChars << std::endl;
    std::cout << "Different characters: " << freqTable.size() << std::endl;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Shannon entropy H: " << entropy
              << " bits/character" << std::endl;
    std::cout << "Average code length L: " << averageCodeLength
              << " bits/character" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "Coding efficiency: " << efficiency << "%" << std::endl;
    std::cout << "Redundancy: " << redundancy << "%" << std::endl;
}
