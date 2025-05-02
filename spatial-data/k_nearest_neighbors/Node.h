#ifndef NODE_H
#define NODE_H
#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>

#include <vector>

class MBR {
public:
    double x_low;
    double y_low;
    double x_high;
    double y_high;

    MBR(const double xLow, const double yLow, const double xHigh, const double yHigh)
        : x_low(xLow), y_low(yLow), x_high(xHigh), y_high(yHigh) {}

    MBR()
        : x_low(0.0), y_low(0.0), x_high(0.0), y_high(0.0) {}

    [[nodiscard]] std::string toString() const {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(6);
        oss << "["
            << x_low << ", "
            << x_high << ", "
            << y_low << ", "
            << y_high
            << "]";
        return oss.str();
    }
};

class Entry {
public:
    int id;
    MBR mbr;
    std::string z_value;

    Entry(const int id, const MBR& mbr, std::string  z = "")
        : id(id), mbr(mbr), z_value(std::move(z)) {}

    Entry() : id(-1) {}

    [[nodiscard]] std::string toString() const {
        std::ostringstream oss;
        oss << "[" << id << ", " << mbr.toString() << "]";
        return oss.str();
    }
};

class Node {
public:
    int node_id;
    MBR mbr;

    Node(const int id, const MBR& mbr) : node_id(id), mbr(mbr) {}

    virtual ~Node() = default;

    [[nodiscard]] virtual std::string toString() const {
        std::ostringstream oss;
        oss << "[" << node_id << ", " << mbr.toString() << "]";
        return oss.str();
    }
};

class InternalNode final : public Node {
public:
    bool children_are_leafs;
    std::vector<std::shared_ptr<Node>> children;

    InternalNode(int id, const MBR& mbr, bool childrenAreLeafs)
        : Node(id, mbr), children_are_leafs(childrenAreLeafs) {}

    [[nodiscard]] std::string toString() const override {
        std::ostringstream oss;
        oss << "[" << (children_are_leafs ? 0 : 1) << ", " << node_id << ", [";
        for (size_t i = 0; i < children.size(); ++i) {
            oss << "[" << children[i]->node_id << ", " << children[i]->mbr.toString() << "]";
            if (i != children.size() - 1) oss << ", ";
        }
        oss << "]]";
        return oss.str();
    }
};


inline void update_parent_mbr(MBR& parent_mbr, const MBR& child_mbr) {
    parent_mbr.x_low = std::min(parent_mbr.x_low, child_mbr.x_low);
    parent_mbr.y_low = std::min(parent_mbr.y_low, child_mbr.y_low);
    parent_mbr.x_high = std::max(parent_mbr.x_high, child_mbr.x_high);
    parent_mbr.y_high = std::max(parent_mbr.y_high, child_mbr.y_high);
}

inline void recompute_node_mbr(InternalNode& internal_node) {
    internal_node.mbr = internal_node.children[0]->mbr;
    for (size_t i = 1; i < internal_node.children.size(); ++i) {
        update_parent_mbr(internal_node.mbr, internal_node.children[i]->mbr);
    }
}
#endif //NODE_H
