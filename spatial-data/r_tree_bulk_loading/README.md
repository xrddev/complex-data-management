### Part 1: R-Tree Construction via Bulk Loading

#### Logic Overview

This module implements the construction of an R-Tree using the *Bulk Loading* technique. The process involves the following main stages:

1. **Data Reading:**
   
   - The program reads 2D coordinates from the `coords.txt` file and offset information from the `offsets.txt` file.
   - The coordinates are stored as a list of `Point` structures, while the offsets are stored as a list of `OffsetRecord` structures. Each offset defines the start and end indices for a polygon in the coordinate list.

2. **MBR (Minimum Bounding Rectangle) Computation:**
   
   - For each record in `offsets.txt`, the corresponding points are identified from `coords.txt`.
   - The minimum and maximum `x` and `y` values are calculated to create an MBR that fully encloses the polygon.
   - The results are stored as `Entry` objects.

3. **Z-Order Calculation:**
   
   - The center of each MBR is calculated and written to the `MBRs_centers.txt` file in the format: `x_center,y_center` (one center per line).
   - An **external Python script** (`z_order.py`) is then executed via a system call from the C++ program. This script reads the centers from `MBRs_centers.txt` and computes the corresponding **Z-order values (Morton codes)** for each center point.
   - The output of the script is saved in `z_values.txt`, with one Morton code per line, corresponding to each entry in order.
   - After the script finishes, the C++ program reads the `z_values.txt` file and updates the `z_value` field of each `Entry` object accordingly.
   
   > ⚠️ **Note:** This integration allows complex spatial mapping (Z-order curve calculation) to be handled externally in Python, enabling flexibility and simplicity, while keeping the C++ code focused on R-tree construction.

4. **Entry Sorting:**
   
   - The list of `Entry` objects is sorted based on their Z-order value, ensuring spatial locality is preserved in the tree structure.

5. **R-Tree Construction:**
   
   - Leaf nodes are created initially, each containing a single MBR.
   - The tree is built level by level, grouping nodes into clusters of 20 (maximum node capacity), while ensuring each node has at least 8 children (minimum node capacity).
   - The resulting R-Tree is saved in the `Rtree.txt` file.

---

#### Data Structures

The R-Tree implementation is based on the following main classes:

- **MBR (Minimum Bounding Rectangle):**
  
  - Represents the bounds of a rectangle with `(x_low, y_low, x_high, y_high)`.
  - Provides a `toString()` method for formatted display.

- **Entry:**
  
  - Represents an R-Tree entry.
  - Contains an `id`, its `MBR`, and a `z_value` (Morton code).
  - Provides a `toString()` method for easy inspection.

- **LeafNode:**
  
  - Represents a leaf node in the R-Tree.
  - Contains a `node_id` and its `MBR`.
  - Provides a `toString()` method for formatted output.

- **InternalNode (inherits from LeafNode):**
  
  - Represents an internal node of the tree.
  - Contains a list of children (`std::vector<std::shared_ptr<LeafNode>>`).
  - Includes a boolean `children_are_leafs` indicating whether its children are leaf or internal nodes.
  - The `toString()` method returns the node’s structure, including its children's details in a formatted string.

- **Helper Functions:**
  
  - `update_parent_mbr()`: Updates a parent node's MBR to encompass all its children's MBRs.
  - `recompute_node_mbr()`: Recomputes an internal node's MBR based on its current children.

---

#### Output

- **Rtree.txt:**  
  This file contains the textual representation of the constructed R-Tree.  
  
  Each node is written as a single line in the format:
  
  - **LeafNode:**  
    `[node_id, [x_low, x_high, y_low, y_high]]`
  
  - **InternalNode:**  
    `[level_flag, node_id, [[child1_id, child1_MBR], [child2_id, child2_MBR], ...]]`
    
    - `level_flag`: 0 if children are leaf nodes, 1 if children are internal nodes.
    - Each `child_MBR` is printed as: `[x_low, x_high, y_low, y_high]`.

---

#### How to Run

```bash
g++ r_tree_bulk_loading.cpp -o r_tree_bulk_loading.out
./r_tree_bulk_loading.out coords.txt offsets.txt
```

---
