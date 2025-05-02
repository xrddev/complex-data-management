### Part 2: Range Queries on R-Tree

#### Logic Overview

This part focuses on executing **range queries** on the R-Tree that was constructed in Part 1. The process consists of the following stages:

1. **Reading the R-Tree:**
   
   - The program reads the `Rtree.txt` file, where each line describes a node (internal or leaf) along with its children and their bounding rectangles.
   - The algorithm reconstructs the R-Tree recursively:
     - For each line, an `InternalNode` is created, and its children are either other `InternalNode`s or `LeafNode`s.
     - MBRs (Minimum Bounding Rectangles) are recomputed based on the children's MBRs to maintain correctness.

2. **Executing Range Queries:**
   
   - The queries are read from the `rqueries.txt` file. Each line defines an MBR query in the format:  
     `<x_low> <y_low> <x_high> <y_high>` (space-separated).
   - For each query, the recursive function `range_query()` is executed:
     - It checks whether the MBR of each node intersects with the query MBR (using the `mbr_intersects()` function).
     - If an internal node intersects, the search continues recursively on its children.
     - If a leaf node intersects, its `id` is added to the result set.

---

#### Key Functions Overview

- **build_tree_from_file():**
  
  - Reconstructs the R-Tree from the `Rtree.txt` file.
  
  - Uses an `unordered_map` to map each `node_id` to its corresponding `InternalNode`.
  
  - The expected file format is consistent with the output of the bulk loading process:
    
    ```
    [level_flag, node_id, [[child1_id, child1_MBR], [child2_id, child2_MBR], ...]]
    ```
    
    where each `child_MBR` is printed as: `[x_low, x_high, y_low, y_high]`.

- **extract_numbers():**
  
  - Extracts all numeric values from a string line, handling integers and floating-point numbers.
  - Used to parse the R-tree structure from each line in the input file.

- **mbr_intersects():**
  
  - Returns `true` if two MBRs intersect, `false` otherwise.

- **range_query():**
  
  - Recursively performs the range search on a node and its children.

- **run_range_queries():**
  
  - Reads all queries from the query file and calls `range_query()` for each one.
  - Prints the results in the format:  
    `<query_number> (<number_of_results>): <result_id1> <result_id2> ...`

---

#### Input File Formats

- **Rtree.txt:**  
  Must be the same format as produced by the bulk loading program. Each line represents a node, as described above.

- **rqueries.txt:**  
  Each line should contain 4 numbers (space-separated):  
  `<x_low> <y_low> <x_high> <y_high>`,  
  specifying the rectangle to be queried.

---

#### How to Run

```bash
g++ range_queries.cpp -o range_queries.out
./range_queries.out Rtree.txt rqueries.txt
```
