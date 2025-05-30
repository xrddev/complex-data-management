# Spatial Data Structures: R-Tree Implementation and Querying

This repository provides a **complete pipeline** for spatial data indexing using **R-Trees** and implements three main operations:

1. **Bulk Loading of R-Trees**
2. **Range Queries**
3. **k-Nearest Neighbor (kNN) Queries**

All modules work together to demonstrate how spatial data (e.g., polygons) can be efficiently indexed and queried.

---

## 📂 Submodules

### 1️⃣ [r_tree_bulk_loading](./r_tree_bulk_loading)

- **Goal:** Build an R-Tree from a set of polygons using **bulk loading**.
- **Highlights:**
    - Computes **Minimum Bounding Rectangles (MBRs)** from input data.
    - Uses **Z-order (Morton) curve sorting** to optimize spatial locality.
    - Outputs the full R-Tree structure to `Rtree.txt`.
- **Includes:**
    - External Python script to calculate Z-order values.

---

### 2️⃣ [range_queries](./range_queries)

- **Goal:** Perform **range queries** on a pre-built R-Tree.
- **Highlights:**
    - Efficiently finds all objects that **intersect** a given query rectangle.
    - Uses the R-Tree loaded from `Rtree.txt`.

---

### 3️⃣ [k_nearest_neighbors](./k_nearest_neighbors)

- **Goal:** Perform **k-Nearest Neighbor (kNN) queries** on an R-Tree.
- **Highlights:**
    - Implements a **best-first search** strategy using a **priority queue** (min-heap).
    - Finds the *k* closest objects to a query point without traversing the entire tree.

---

## 📦 Shared Architecture

All modules rely on:

- **Common Data Structures:**
    - `Node`, `LeafNode`, `InternalNode`
    - `MBR` (Minimum Bounding Rectangle)
- **Standard R-Tree Format:**
    - The R-Tree is saved in `Rtree.txt` (produced by the bulk loading module and consumed by the other modules).

---

## 🚀 How to Use

Each module comes with its own `README.md` that explains:

- The logic and implementation details.
- How to compile and run the code.
- Input/output file formats.

Make sure to start with the `r_tree_bulk_loading` module to generate `Rtree.txt` before running range or kNN queries.

---

## 🔧 Requirements

- **C++17 or later**
- **Python 3** (required only for the Z-order script in the bulk loading module)
- 
---
## 👤 Author

> GitHub: [xrddev](https://github.com/xrddev)


## 📝 License

Released under the [MIT License](LICENSE). Originally built as part of a university project.
