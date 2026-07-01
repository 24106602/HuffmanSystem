#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <map>
#include <string>

#include "Node.h"

std::string formatCharacter(char character);
void showFrequencyTable(const std::map<char, int>& freqTable);
void showCodeTable(const std::map<char, std::string>& codeTable);
void showTree(const Node* root, int depth = 0);
void showCompression(std::size_t originalBits,
                     std::size_t compressedBits,
                     double compressionRate);
void showEntropyAnalysis(const std::map<char, int>& freqTable,
                         const std::map<char, std::string>& codeTable,
                         std::size_t totalChars);

#endif
