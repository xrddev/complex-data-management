/**
* @brief Entry point for k-nearest neighbor search.
 * @param argc Number of command-line arguments
 * @param argv Argument list: expects [Rtree.txt knqueries.txt <k>]
 *
 * Loads the R-tree, reads query points, and executes k-NN searches.
 */

#include <cmath>
#include <iostream>
#include <queue>
#include <unordered_map>
#include "Node.h"

/*
* @brief Priority queue entry for k-nearest neighbor search.
*
* Contains a pointer to an R-tree node (leaf or internal) and its minimum distance
* from the query point. Used to prioritize exploration in best-first search.
*/
struct PQEntry {
    std::shared_ptr<Node> node; ///< Pointer to the R-tree node
    double distance; ///< Distance from the query point

    bool operator>(const PQEntry& other) const {
        return distance > other.distance;
    }
};


std::shared_ptr<InternalNode> build_tree_from_file(const std::string& filename);
std::vector<std::string> extract_numbers(const std::string& line);
bool mbr_intersects(const MBR& a, const MBR& b);
double min_dist(const MBR& mbr, double qx, double qy);
void run_kn_queries(const std::shared_ptr<InternalNode>& root, const std::string& kn_queries_filename, int k);



int main(const int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: ./knqueries.out Rtree.txt knqueries.txt  <kn>\n";
        return 1;
    }

    const std::string rtree_filename = argv[1];
    const std::string kn_queries_filename = argv[2];
    const int k = std::stoi(argv[3]);

    const std::shared_ptr<InternalNode> root = build_tree_from_file(rtree_filename);
    run_kn_queries(root, kn_queries_filename, k);
    return 0;
}



/**
 * @brief Checks if two MBRs intersect
 * @param a First MBR
 * @param b Second MBR
 * @return true if MBRs intersect, false otherwise
 */
bool mbr_intersects(const MBR& a, const MBR& b) {
    if (a.x_high < b.x_low || a.x_low > b.x_high) return false;
    if (a.y_high < b.y_low || a.y_low > b.y_high) return false;
    return true;
}

/**
 * @brief Extracts numbers from a string line
 * @param line Input string containing numbers
 * @return Vector of strings representing extracted numbers
 */
std::vector<std::string> extract_numbers(const std::string& line) {
    std::vector<std::string> numbers;
    std::string curr;

    for (size_t i = 0; i < line.length(); ++i) {
        if (std::isdigit(line[i]) ||
            (line[i] == '-' && (i == 0 || !std::isdigit(line[i-1]))) ||
            (line[i] == '.' && !curr.empty() && std::isdigit(curr.back()))) {
            curr += line[i];
            } else {
                if (!curr.empty()) {
                    numbers.push_back(curr);
                    curr.clear();
                }
            }
    }
    if (!curr.empty())
        numbers.push_back(curr);

    return numbers;
}

/**
 * @brief Builds an R-tree from a text file
 * @param filename Name of the file containing R-tree data
 * @return Root node of the constructed R-tree
 *
 * Expected file format: same as produced by the bulk loading program (Rtree.txt).
 * Each line represents a node:
 * [level_flag, node_id, [[child1_id, child1_MBR], [child2_id, child2_MBR], ...]]
 */
std::shared_ptr<InternalNode> build_tree_from_file(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file " << filename << "\n";
        exit(-1);
    }

    std::unordered_map<int, std::shared_ptr<InternalNode>> id_to_node;
    std::string line;
    std::shared_ptr<InternalNode> root = nullptr;

    while (std::getline(infile, line)) {
        std::vector<std::string> numbers = extract_numbers(line);

        bool is_leaf = (numbers[0] == "0");
        int node_id = std::stoi(numbers[1]);
        auto internal_node = std::make_shared<InternalNode>(node_id, MBR{}, is_leaf);

        for (int i = 2; i < numbers.size(); i += 5) {
            int child_id = std::stoi(numbers[i]);
            double x_low = std::stod(numbers[i + 1]);
            double y_low = std::stod(numbers[i + 3]);
            double x_high = std::stod(numbers[i + 2]);
            double y_high = std::stod(numbers[i + 4]);

            MBR mbr(x_low, y_low, x_high, y_high);

            if (is_leaf) {
                auto leaf = std::make_shared<Node>(child_id, mbr);
                internal_node->children.push_back(leaf);
            } else {
                auto child_it = id_to_node.find(child_id);
                internal_node->children.push_back(child_it->second);
            }
        }

        recompute_node_mbr(*internal_node);
        id_to_node[node_id] = internal_node;
        root = internal_node;
    }

    infile.close();

    if (!root) {
        std::cerr << "Tree is empty!\n";
        return nullptr;
    }
    return root;
}

/**
 * @brief Calculates minimum distance between a point and an MBR
 * @param mbr The minimum bounding rectangle
 * @param qx X-coordinate of the query point
 * @param qy Y-coordinate of the query point
 * @return Minimum distance between point and MBR
 *
 * Computes the Euclidean distance between a query point and the closest point of the MBR.
 * If the point lies inside the MBR, the distance is 0.
 */
double min_dist(const MBR& mbr, const double qx, const double qy) {
    double dx = 0.0;
    if (qx < mbr.x_low)
        dx = mbr.x_low - qx;
    else if (qx > mbr.x_high)
        dx = qx - mbr.x_high;

    double dy = 0.0;
    if (qy < mbr.y_low)
        dy = mbr.y_low - qy;
    else if (qy > mbr.y_high)
        dy = qy - mbr.y_high;

    return std::sqrt(dx * dx + dy * dy);
}

/**
 * @brief Processes k-nearest neighbor queries from a file
 * @param root Root node of the R-tree
 * @param kn_queries_filename File containing query points
 * @param k The Number of nearest neighbors to find
 *
 * The query file should contain one point per line:
 * <x> <y>
 * where <x> and <y> are floating-point coordinates.

 */
void run_kn_queries(const std::shared_ptr<InternalNode>& root, const std::string& kn_queries_filename, int k) {
    std::ifstream infile(kn_queries_filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file " << kn_queries_filename << "\n";
        exit(-1);
    }

    std::string line;
    int query_id = 0;

    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        double x, y;
        ss >> x >> y;

        std::priority_queue<PQEntry, std::vector<PQEntry>, std::greater<>> pq;
        pq.push({root, min_dist(root->mbr, x, y)});

        std::vector<int> kn_leaf_ids;
        while (!pq.empty() && kn_leaf_ids.size() < static_cast<size_t>(k)) {
            PQEntry entry = pq.top();
            pq.pop();

            if (auto internal_node = std::dynamic_pointer_cast<InternalNode>(entry.node); internal_node != nullptr) {
                for (const auto& child : internal_node->children) {
                    pq.push({child, min_dist(child->mbr, x, y)});
                }
            } else {
                kn_leaf_ids.push_back(entry.node->node_id);
            }
        }

        std::cout << query_id++ << "(" << kn_leaf_ids.size() << "): ";
        for (const auto& id : kn_leaf_ids) {
            std::cout << id << " ";
        }
        std::cout << "\n";
    }
}
