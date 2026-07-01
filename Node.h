#ifndef NODE_H
#define NODE_H
//树节点
struct Node
{
    char ch; ///该节点代表的字符（仅叶子节点有意义，内部节点为 '\0'）
    int freq;//权重 权重 = 该字符出现的次数（内部节点 = 子树权重之和）
    Node*  left; //左孩子 左节  编码方向为 '0'
    Node*  right;//右孩子 右节点  编码方向为 '1'

    Node (char c,int f)
    {
        ch =c;
        freq =f;//简写
        left =right = nullptr; //置空
    }
};

#endif