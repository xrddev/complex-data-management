## Part 3: k-Nearest Neighbor (kNN) Queries

### Logic Overview

This module implements **k-Nearest Neighbor (kNN) queries** on an R-Tree structure using a **best-first search algorithm**.

- The goal is to retrieve the *k* closest object MBRs (Minimum Bounding Rectangles) to a query point `q`.
- The implementation uses a **priority queue**, where the next node or object with the smallest distance from `q` is explored first.
- The queue holds both **internal nodes** (to explore their children) and **leaf nodes (object MBRs)**.
- When a leaf node is dequeued, it represents the next closest result.

### Implementation Details

1. **R-Tree Loading:**
   
   - The R-Tree is reconstructed from the `Rtree.txt` file using the `build_tree_from_file()` function.
   - The format of `Rtree.txt` is the same as produced by the bulk-loading program.

2. **Query Loading:**
   
   - The queries are read from `knqueries.txt`.
   - Each query is a single line containing two floating-point numbers:  
     `<x> <y>` — the coordinates of the query point.

3. **kNN Search:**
   
   - For each query:
     - The algorithm starts by pushing the root of the R-Tree into the priority queue, along with its minimum distance to the query point (calculated using `min_dist()`).
     - While fewer than *k* results have been found:
       - The closest element (node or object) is popped from the queue.
       - If it's an internal node, all its children are pushed into the queue with their corresponding distances.
       - If it's a leaf node (object MBR), it is added to the result list.
   - The results are printed for each query in the format:  
     `<query_id> (<number_of_results>): <list_of_object_ids>`

---

### Key Functions

- **build_tree_from_file():**
  
  - Reconstructs the R-Tree from `Rtree.txt`.

- **min_dist():**
  
  - Computes the minimum Euclidean distance between the query point `(x, y)` and an MBR.

- **run_kn_queries():**
  
  - Processes all queries from `knqueries.txt` and runs the kNN search for each query.

---

### Priority Queue Logic

The algorithm uses a **priority queue (min-heap)** to manage the order in which nodes are explored. Here's how it works:

- Each entry in the queue is a `PQEntry` that holds:
  
  - A pointer to an R-Tree node (either an internal node or a leaf/object node).
  - The **minimum distance** between the query point and that node’s MBR (calculated via `min_dist()`).

- The priority queue ensures that at each step:
  
  - The node or object **closest** to the query point (based on the MBR) is dequeued **first**.
  - If that node is an internal node, **all its children** are added to the queue, each with their own distance from the query point.
  - If that node is a leaf (an actual object), it is **added to the results list.**

- This approach guarantees that the next closest node or object is **always explored next**. Therefore:
  
  - The algorithm is efficient because it avoids traversing irrelevant parts of the tree (i.e., branches far away from the query point).
  - The search **stops as soon as `k` nearest neighbors are found**, without needing to visit the entire tree.

In effect, this is a **best-first search strategy**, which is optimal for kNN in R-Trees because it prioritizes nodes that are geometrically closer, reducing unnecessary computation.

---

#### Input File Formats

- **Rtree.txt:**
  Must be the same format as produced by the bulk loading program. Each line represents a node, as described above.

- **knqueries.txt:**
  Each line should contain 2 numbers (space-separated):
  `<x> <y>`,
  specifying the coordinates of the query point.

---

### How to Run

```bash
g++ k_nearest_neighbors.cpp -o k_nearest_neighbors.out
./k_nearest_neighbors.out Rtree.txt knqueries.txt 10
```
