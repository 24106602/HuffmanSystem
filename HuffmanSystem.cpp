#include "HuffmanSystem.h"

#include <climits>
#include <queue>
#include <vector>

namespace {
//有限队列比较器  
struct NodeCompare {
    bool operator()(const Node* left, const Node* right) const {
        if (left->freq != right->freq) {
            return left->freq > right->freq;
        }

        // 频率相同时增加稳定的字符顺序，便于重复运行时观察编码表。
        return static_cast<unsigned char>(left->ch) >
               static_cast<unsigned char>(right->ch);
    }
};

}  // namespace
//初始化
HuffmanSystem::HuffmanSystem() : root(nullptr) {}
//析构函数
HuffmanSystem::~HuffmanSystem() {
    clearTree(root);//程序结束或销毁时自动释放整颗树
}

void HuffmanSystem::clearTree(Node* node) {
    if (node == nullptr) {
        return;
    }

    clearTree(node->left);//先释放子树
    clearTree(node->right);
    delete node;//再释放自身
}
//输入 并构建频率表
void HuffmanSystem::input(const std::string& text) {
    clearTree(root);//如果之前建过树，先释放旧内存  保证在一个函数上反复调用input()，不会内存泄漏
    root = nullptr;
    freqTable.clear(); //清空旧的频率表
    codeTable.clear();

    for (char character : text) {
        ++freqTable[character];  //map 的【】运算符：不存在就自动创建并初始化为0
    }
}
//构建哈夫曼树
void HuffmanSystem::buildTree() {
    // 支持在同一个对象上重复建树，不保留旧树或旧编码表。
    clearTree(root);//释放旧树
    root = nullptr;
    codeTable.clear(); //旧编码表作废

    if (freqTable.empty()) {
        return;
    }//没有数据就不建
    //优先队列：小根堆
    std::priority_queue<Node*, std::vector<Node*>, NodeCompare> queue;
    for (const auto& item : freqTable) {
        queue.push(new Node(item.first, item.second));
    }

    while (queue.size() > 1) //队列只剩1个时停止
    {
        Node* left = queue.top();//取出最小的
        queue.pop();
        Node* right = queue.top();//取出次小的
        queue.pop();

        Node* parent = new Node('\0', left->freq + right->freq);//合并为新节点
        parent->left = left;
        parent->right = right;
        queue.push(parent);//新节点放回队列
    }

    root = queue.top();//队列中最后一个就是根节点
}
//深度遍历 
void HuffmanSystem::generateCodeDfs(Node* node, const std::string& path) {
    if (node == nullptr) {
        return;
    }
    //到达子节点 找到一个字符的编码
    if (node->left == nullptr && node->right == nullptr) 
    {
        // 只有一种字符时使用 0，保证每个字符仍占一个编码位。
        codeTable[node->ch] = path.empty() ? "0" : path;
        return;
    }

    generateCodeDfs(node->left, path + "0");//左分支追加"0"
    generateCodeDfs(node->right, path + "1");//右分支追加"1"
}

void HuffmanSystem::generateCode() {
    codeTable.clear();
    generateCodeDfs(root, "");
}
//编码
std::string HuffmanSystem::encode(const std::string& text) const {
    std::string result;

    if (codeTable.empty()) {
        return result;
    }

    for (char character : text) {
        const auto code = codeTable.find(character);
        if (code == codeTable.end()) {
            return std::string();
        }
        result += code->second;//查表，拼接编码
    }

    return result;
}
//解码
bool HuffmanSystem::decode(const std::string& code,
                           std::string& result) const
                            
    {
    result.clear();

    if (root == nullptr) {
        return false;
    }

    // 单节点树没有左右分支，其合法编码只能由 0 组成。
    if (root->left == nullptr && root->right == nullptr) {
        for (char bit : code) {
            if (bit != '0') {
                result.clear();
                return false;
            }
            result += root->ch;
        }
        return true;
    }

    const Node* current = root;
    for (char bit : code) {
        if (bit == '0') {
            current = current->left;
        } else if (bit == '1') {
            current = current->right;
        }
         else {
            result.clear();
            return false;//非法字符
        }

        if (current == nullptr) {
            result.clear();
            return false;//空指针异常
        }
     
        //到达叶子节点 还原一个字符
        if (current->left == nullptr && current->right == nullptr) {
            result += current->ch;
            current = root;//回到跟根节点
        }
    }

    // 没有回到根节点表示最后一个字符的编码不完整。
    if (current != root) {
        result.clear();
        return false;
    }

    return true;
}
//压缩率
double HuffmanSystem::getCompressionRate(
    const std::string& original,
    const std::string& encoded) const {
    if (original.empty()) {
        return 0.0;
    }

    const double originalBits =
        static_cast<double>(original.size()) * CHAR_BIT;
    return static_cast<double>(encoded.size()) / originalBits;
}


bool HuffmanSystem::isReady() const {
    return root != nullptr && !codeTable.empty();
}

const Node* HuffmanSystem::getRoot() const {
    return root;
}

const std::map<char, int>& HuffmanSystem::getFreqTable() const {
    return freqTable;
}

const std::map<char, std::string>& HuffmanSystem::getCodeTable() const {
    return codeTable;
}
