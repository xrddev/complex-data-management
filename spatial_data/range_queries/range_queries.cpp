/**
* @brief Entry point of the program.
 * @param argc Argument count
 * @param argv Argument vector: expects [rtree.txt rqueries.txt]
 *
 * Loads the R-tree from a file and runs range queries specified in the query file.
 */



#include <unordered_map>
#include <iostream>
#include "Node.h"


std::shared_ptr<InternalNode> build_tree_from_file(const std::string &filename);
std::vector<std::string> extract_numbers(const std::string &line);
bool mbr_intersects(const MBR &a, const MBR &b);
void range_query(const std::shared_ptr<InternalNode> &node, const MBR &query_mbr, std::vector<int> &results);
void run_range_queries(const std::shared_ptr<InternalNode>& root, const std::string& r_queries_filename);


int main(const int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./program rtree.txt rqueries.txt\n";
        return 1;
    }

    const std::string rtree_filename = argv[1];
    const std::string r_queries_filename = argv[2];

    const std::shared_ptr<InternalNode> root = build_tree_from_file(rtree_filename);
    run_range_queries(root, r_queries_filename);

    return 0;
}



/**
 * @brief Extracts numeric values from a string line
 * @param line Input string containing numbers
 * @return Vector of strings containing extracted numbers
 * 
 * Extracts numbers (including negative numbers and decimals) from the input string,
 * ignoring any non-numeric characters.
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
 * @brief Constructs an R-tree from a text file
 * @param filename Path to the input file containing R-tree structure
 * @return The root node of the constructed R-tree
 * 
 * Each line in the file represents a node in the format used by the r_tree bulk loading process before:
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
            double x_high = std::stod(numbers[i + 2]);
            double y_low = std::stod(numbers[i + 3]);
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
 * @brief Checks if two MBRs intersect
 * @param a First MBR
 * @param b Second MBR
 * @return True, if the MBRs intersect, false otherwise
 * 
 * Determines whether two minimum bounding rectangles have any overlapping area.
 */
bool mbr_intersects(const MBR& a, const MBR& b) {
    if (a.x_high < b.x_low || a.x_low > b.x_high) return false;
    if (a.y_high < b.y_low || a.y_low > b.y_high) return false;
    return true;
}

/**
 * @brief Performs a range query on the R-tree
 * @param node Current node being examined
 * @param query_mbr Query region as MBR
 * @param results Vector to store matching node IDs
 * 
 * Recursively traverses the R-tree to find all leaf nodes whose MBRs
 * intersect with the query MBR.
 */
void range_query(const std::shared_ptr<InternalNode>& node, const MBR& query_mbr, std::vector<int>& results) {
    if(node == nullptr) return;

    for(const auto& child : node->children) {
        if(!mbr_intersects(child->mbr, query_mbr)) continue;

        if(auto internal_node = std::dynamic_pointer_cast<InternalNode>(child); internal_node != nullptr) {
            range_query(internal_node, query_mbr, results);
        }else {
            results.push_back(child->node_id);
        }
    }
}

/**
 * @brief Executes multiple range queries from a file
 * @param root Root node of the R-tree
 * @param r_queries_filename File containing query MBRs
 *
 * Reads query MBRs from a file and performs range queries on the R-tree,
 * printing results for each query.
 *
 * The query file should contain one query MBR per line in the format: <x_low> <y_low> <x_high> <y_high>
 */
void run_range_queries(const std::shared_ptr<InternalNode>& root, const std::string& r_queries_filename) {
    std::ifstream infile(r_queries_filename);
    if (!infile.is_open()) {
        std::cerr << "Failed to open file " << r_queries_filename << "\n";
        exit(-1);
    }

    std::string line;
    std::vector<int> results;

    int line_count = 0;
    while(std::getline(infile, line)) {
        std::stringstream ss(line);
        double x_low, x_high, y_low, y_high;
        ss >> x_low >> y_low >> x_high >> y_high;
        MBR query_mbr(x_low, y_low, x_high, y_high);
        range_query(root, query_mbr, results);

        std::cout << line_count++ << " (" << results.size() << "): ";
        for(const auto& result : results) {
            std::cout << result << " ";
        }
        std::cout << "\n";
        results.clear();
    }
}





