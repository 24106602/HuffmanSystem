#ifndef HUFFMAN_SYSTEM_H
#define HUFFMAN_SYSTEM_H

#include <map>
#include <string>

#include "Node.h"

class HuffmanSystem {
private:
    Node* root;
    std::map<char, int> freqTable;
    std::map<char, std::string> codeTable;

    void generateCodeDfs(Node* node, const std::string& path);
    void clearTree(Node* node);

public:
    HuffmanSystem();
    ~HuffmanSystem();

    // 禁止拷贝，避免多个对象重复释放同一棵树。
    HuffmanSystem(const HuffmanSystem&) = delete;
    HuffmanSystem& operator=(const HuffmanSystem&) = delete;

    // 输入文本并统计字符频率。
    void input(const std::string& text);
    // 根据频率表构建哈夫曼树。
    void buildTree();
    // 遍历哈夫曼树并生成编码表。
    void generateCode();

    std::string encode(const std::string& text) const;
    bool decode(const std::string& code, std::string& result) const;
    double getCompressionRate(const std::string& original,
                              const std::string& encoded) const;

    bool isReady() const;

    const Node* getRoot() const;
    const std::map<char, int>& getFreqTable() const;
    const std::map<char, std::string>& getCodeTable() const;
};

#endif
