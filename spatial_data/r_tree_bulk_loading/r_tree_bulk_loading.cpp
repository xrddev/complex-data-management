/**
 * @file r_tree_bulk_loading.cpp
 * @brief Builds an R-tree from polygon data using MBRs and z-order values.
 *
 * This program:
 * - Reads 2D coordinates and offset records defining polygons
 * - Computes Minimum Bounding Rectangles (MBRs) for each polygon
 * - Calculates z-order values to spatially sort the entries
 * - Builds an R-tree with leaf and internal nodes (bulk loading)
 * - Outputs the R-tree structure to "Rtree.txt"
 *
 * Usage:
 *   ./r_tree_build.out coords.txt offsets.txt
 */


#include "LeafNode.h"


/**
 * @brief Represents a 2D point with x and y coordinates
 */
struct Point {
    double x; ///< X-coordinate of the point
    double y; ///< Y-coordinate of the point
};

/**
 * @brief Represents a record containing offset information for polygon coordinates
 */
struct OffsetRecord {
    int id; ///< Unique identifier for the record
    int startOffset; ///< Starting index in the coordinates array
    int endOffset; ///< Ending index in the coordinates array
};


std::vector<Entry> computeMBRs(const std::vector<Point>& coords, const std::vector<OffsetRecord>& offsets);
std::vector<Point> readCoords(const std::string& filename);
std::vector<OffsetRecord> readOffsets(const std::string& filename);
void update_z_value_to_entries(std::vector<Entry>& entries, const std::string& filename);
void sortEntriesByZValue(std::vector<Entry>& entries);
std::vector<std::shared_ptr<LeafNode>> create_upper_level(const std::vector<std::shared_ptr<LeafNode>>& nodes, int &node_id);
void generate_z_value(std::vector<Entry>& entries);
std::vector<std::shared_ptr<LeafNode>> build_leaf_nodes(const std::vector<Entry>& entries);
std::shared_ptr<InternalNode> build_tree(std::vector<std::shared_ptr<LeafNode>>& nodes);
std::string calculate_MBRs_centers(const std::vector<Entry>& entries);


int main(const int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./r_tree_build.out coords.txt offsets.txt\n";
        return 1;
    }

    const std::vector<Point> records = readCoords(argv[1]);
    const std::vector<OffsetRecord> offsets = readOffsets(argv[2]);
    std::vector<Entry> entries = computeMBRs(records, offsets);
    generate_z_value(entries);
    sortEntriesByZValue(entries);

    std::vector<std::shared_ptr<LeafNode>> leaf_nodes = build_leaf_nodes(entries);
    std::shared_ptr<InternalNode> root = build_tree(leaf_nodes);
    return 0;
}



/**
 * @brief Reads coordinates from a file and converts them to Points
 * @param filename Path to the file containing coordinates
 * @return Vector of Points read from the file
 * @throw Exits with error if a file cannot be opened or read
 */
std::vector<Point> readCoords(const std::string& filename) {
    std::vector<Point> points;
    std::string line;

    std::ifstream coords_file(filename);
    if(!coords_file.is_open()) {
        std::cerr << "Error opening coords file\n";
        exit(-1);
    }


    while (std::getline(coords_file, line)) {
        std::stringstream ss(line);
        std::string x, y;

        bool x_read_success =  static_cast<bool>(std::getline(ss, x, ','));
        bool y_read_success =  static_cast<bool>(std::getline(ss, y));
        if (!x_read_success || !y_read_success) {
            std::cerr << "Error reading coords file\n";
            exit(-1);
        }

        points.push_back(Point{std::stod(x), std::stod(y)});
    }
    return points;
}

/**
 * @brief Reads offset records from a file
 * @param filename Path to the file containing offset records
 * @return Vector of OffsetRecords read from the file
 * @throw Exits with error if a file cannot be opened or read
 */
std::vector<OffsetRecord> readOffsets(const std::string& filename) {
    std::vector<OffsetRecord> offsets;
    std::string line;

    std::ifstream offsets_file(filename);
    if(!offsets_file.is_open()) {
        std::cerr << "Error opening offsets file\n";
        exit(-1);
    }


    while (std::getline(offsets_file, line)) {
        std::stringstream ss(line);
        std::string id, startOffset, endOffset;

        bool id_read_success =  static_cast<bool>(std::getline(ss, id, ','));
        bool startOffset_read_success =  static_cast<bool>(std::getline(ss, startOffset, ','));
        bool endOffset_read_success =  static_cast<bool>(std::getline(ss, endOffset));

        if (!id_read_success || !startOffset_read_success || !endOffset_read_success) {
            std::cerr << "Error reading offsets file\n";
            exit(-1);
        }

        offsets.push_back(OffsetRecord{std::stoi(id), std::stoi(startOffset), std::stoi(endOffset)});
    }

    return offsets;
}

/**
 * @brief Computes Minimum Bounding Rectangles (MBRs) for polygons defined by coordinate offsets.
 *
 * Each OffsetRecord specifies the start and end indices of a polygon's points in the coords vector.
 * The function calculates the bounding rectangle for each polygon.
 *
 * @param coords Vector of 2D points (all polygon coordinates)
 * @param offsets Vector of offset records defining which points belong to each polygon
 * @return Vector of Entries containing the computed MBRs
 */

std::vector<Entry> computeMBRs(const std::vector<Point>& coords, const std::vector<OffsetRecord>& offsets) {
    std::vector<Entry> entries;

    for (const auto&[id, startOffset, endOffset] : offsets) {
        double x_low = coords[startOffset].x;
        double y_low = coords[startOffset].y;
        double x_high = x_low;
        double y_high = y_low;

        for(int i = startOffset + 1; i <= endOffset; i++) {
            x_low = std::min(x_low, coords[i].x);
            y_low = std::min(y_low, coords[i].y);
            x_high = std::max(x_high, coords[i].x);
            y_high = std::max(y_high, coords[i].y);
        }
        entries.emplace_back(id, MBR(x_low, y_low, x_high, y_high));
    }
    return entries;
}

/**
 * @brief Calculates and writes the centers of MBRs to a file
 * @param entries Vector of entries containing MBRs
 * @return Name of the file where centers were written
 * @throw Exits with error if a file cannot be opened
 */
std::string calculate_MBRs_centers(const std::vector<Entry>& entries) {
    std::ofstream out_file("MBRs_centers.txt");
    if(!out_file.is_open()) {
        std::cerr << "Error opening MBRs centers file\n";
        exit(-1);
    }

    for (const auto& entry : entries) {
        const double x_center = (entry.mbr.x_low + entry.mbr.x_high) / 2.0;
        const double y_center = (entry.mbr.y_low + entry.mbr.y_high) / 2.0;
        out_file << std::setprecision(10) << x_center << "," << y_center << "\n";
    }
    out_file.close();
    return "MBRs_centers.txt";
}

/**
 * @brief Updates z-values of the provided entries by reading them from a file.
 *
 * Each line in the file corresponds to the z-value of an entry (same order as the entry vector).
 *
 * @param entries Vector of entries to update
 * @param filename Path to the file containing z-values (one per line)
 * @throw Exits with error if the file cannot be opened
 */

void update_z_value_to_entries(std::vector<Entry>& entries, const std::string& filename) {
    std::ifstream in_file(filename);
    if (!in_file.is_open()) {
        std::cerr << "Failed to open z_values file\n";
        exit(-1);
    }

    std::string line;
    size_t index = 0;
    while (std::getline(in_file, line) && index < entries.size()) {
        entries[index++].z_value = line;
    }

    in_file.close();
}


/**
 * @brief Generates z-values for entries using an external Python script.
 *
 * This function:
 * 1. Calculates the centers of MBRs and writes them to "MBRs_centers.txt"
 * 2. Executes the "z_order.py" script to compute z-values
 * 3. Reads the computed z-values from "z_values.txt" and updates the entries
 *
 * @param entries Vector of entries to generate z-values for
 * @throw Exits with error if the Python script fails or files cannot be accessed
 */

void generate_z_value(std::vector<Entry>& entries) {
    const std::string centers_file = calculate_MBRs_centers(entries);
    const std::string z_values_file = "z_values.txt";
    const std::string script_path = "z_order.py";

    const std::string command = "python3 " + script_path + " " + centers_file + " " + z_values_file;

    if (const int ret = system(command.c_str()); ret != 0) {
        std::cerr << "Error running z_order.py script\n";
        exit(-1);
    }

    update_z_value_to_entries(entries,z_values_file);
}

/**
 * @brief Sorts entries based on their z-values
 * @param entries Vector of entries to sort
 */
void sortEntriesByZValue(std::vector<Entry>& entries) {
    std::sort(entries.begin(), entries.end(), [](const Entry& a, const Entry& b) {
        return a.z_value < b.z_value;
    });
}

/**
 * @brief Creates leaf nodes from entries
 * @param entries Vector of entries to convert to leaf nodes
 * @return Vector of leaf node pointers
 */
std::vector<std::shared_ptr<LeafNode>> build_leaf_nodes(const std::vector<Entry>& entries) {
    std::vector<std::shared_ptr<LeafNode>> leaf_nodes;
    for (const auto& entry : entries) {
        auto leaf_node = std::make_shared<LeafNode>(entry.id, entry.mbr);
        leaf_nodes.push_back(leaf_node);
    }
    return leaf_nodes;
}

/**
 * @brief Builds R-tree from leaf nodes
 * @param nodes Vector of leaf nodes to build a tree from
 * @return Pointer to the root node of the tree
 * @throw Exits with error if an output file cannot be opened
 */
std::shared_ptr<InternalNode> build_tree(std::vector<std::shared_ptr<LeafNode>>& nodes) {
    std::ofstream outfile("Rtree.txt");
    if (!outfile.is_open()) {
        std::cerr << "Failed to open Rtree.txt for writing!\n";
        exit(-1);
    }

    int node_id = 0;
    int level = 0;

    while (nodes.size() > 1) {
        nodes = create_upper_level(nodes, node_id);

        std::cout << nodes.size() << " nodes at level " << level++ << std::endl;
        for (const auto& node : nodes) outfile << node->toString() << "\n";
    }

    outfile.close();
    return std::dynamic_pointer_cast<InternalNode>(nodes.front());
}

/**
 * @brief Creates the upper level of R-tree from given nodes
 * @param nodes Vector of nodes to group into upper level
 * @param node_id Reference to current node ID counter
 * @return Vector of nodes forming the upper level
 */
std::vector<std::shared_ptr<LeafNode>> create_upper_level(const std::vector<std::shared_ptr<LeafNode>>& nodes, int &node_id) {
    std::vector<std::shared_ptr<LeafNode>> upper_level_nodes;
    constexpr int MAX_CHILDREN_PER_NODE = 20;
    constexpr int MIN_CHILDREN_PER_NODE = 8;
    const size_t number_of_full_nodes = nodes.size() / MAX_CHILDREN_PER_NODE;

    bool children_are_leafs = node_id == 0;
    for(size_t i = 0; i < number_of_full_nodes; i++) {
        //Parent node initialized with child 0 MBR. We need a valid starting point for calculating mbr.
        auto internal_node = std::make_shared<InternalNode>(node_id++, nodes[i * MAX_CHILDREN_PER_NODE]->mbr, children_are_leafs);
        internal_node->children.push_back(nodes[i * MAX_CHILDREN_PER_NODE]);

        //For the rest of the children
        for(size_t j = 1; j < MAX_CHILDREN_PER_NODE; j++) {
            update_parent_mbr(internal_node->mbr, nodes[i * MAX_CHILDREN_PER_NODE + j]->mbr);
            internal_node->children.push_back(nodes[i * MAX_CHILDREN_PER_NODE + j]);
        }
        upper_level_nodes.push_back(internal_node);
    }


    if(const size_t remaining_nodes = nodes.size() % MAX_CHILDREN_PER_NODE; remaining_nodes != 0 ) {
        const auto internal_node_with_remainders = std::make_shared<InternalNode>(node_id++, nodes[number_of_full_nodes * MAX_CHILDREN_PER_NODE]->mbr, children_are_leafs);
        internal_node_with_remainders->children.push_back(nodes[number_of_full_nodes * MAX_CHILDREN_PER_NODE]);

        for (size_t i = number_of_full_nodes * MAX_CHILDREN_PER_NODE + 1; i < nodes.size(); i++) {
            update_parent_mbr(internal_node_with_remainders->mbr, nodes[i]->mbr);
            internal_node_with_remainders->children.push_back(nodes[i]);
        }

        if(const size_t needed_children = MIN_CHILDREN_PER_NODE - remaining_nodes; needed_children > 0 && !upper_level_nodes.empty()) {
            for(int i = 0; i < needed_children; i++) {
                auto last_full_internal_node = std::dynamic_pointer_cast<InternalNode>(upper_level_nodes.back());
                auto child = last_full_internal_node->children.back();
                update_parent_mbr(internal_node_with_remainders->mbr, child->mbr);
                internal_node_with_remainders->children.insert(internal_node_with_remainders->children.begin(), child);
                last_full_internal_node->children.pop_back();
                recompute_node_mbr(*last_full_internal_node);
            }
        }
        upper_level_nodes.push_back(internal_node_with_remainders);
    }
    return upper_level_nodes;
}

