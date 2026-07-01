#ifndef UTILS_H
#define UTILS_H

#include <cstddef>
#include <map>
#include <string>

#include "Node.h"

std::string formatCharacter(char character);
void showFrequencyTable(const std::map<char, int>& freqTable);///频率表
void showCodeTable(const std::map<char, std::string>& codeTable);//编码表
void showTree(const Node* root, int depth = 0);//哈夫曼树
void showCompression(std::size_t originalBits,//压缩
                     std::size_t compressedBits,
                     double compressionRate);

#endif
