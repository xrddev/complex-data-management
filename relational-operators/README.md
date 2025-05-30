# 📊 Relational Algebra Operations on TSV Files in C++

This project implements **five relational algebra operations** for TSV-formatted datasets, written in modern C++.
All operations (except group-by) are designed to run in a **streaming manner**,
processing files line-by-line **without loading them entirely into memory**,
making them highly efficient for large datasets.



## 📁 Project Overview

- **Implemented Operations**:
  - Merge Join (`R ⋈ S`)
  - Union (`R ∪ S`)
  - Intersection (`R ∩ S`)
  - Difference (`R − S`)
  - Group-By with Aggregation (Σ over value column)

- **Primary File**:
  - `main.cpp`: All core logic and file-based algorithms

- **Input Files**:
  - `R_sorted.tsv`: Relation `R`, sorted by key (column 1)
  - `S_sorted.tsv`: Relation `S`, sorted by key (column 1)
  - `R.tsv`: Unsorted `R`, used for group-by

- **Output Files**:
  - `RjoinS.tsv`, `RunionS.tsv`, `RintersectionS.tsv`, `RdifferenceS.tsv`, `Rgroupby.tsv`

## 🔧 Compilation

```bash
g++ main.cpp
./a.out R_sorted.tsv S_sorted.tsv R.tsv
```

## 🧠 Operations Overview

---
⚠️ This program assumes that `R_sorted.tsv` and `S_sorted.tsv` are pre-sorted by key.

### 1️⃣ Merge Join

Performs a **merge join** between `R` and `S` on their common key.

- ✅ Efficient on sorted files  
- ✅ Handles multiple matching keys (many-to-many)  
- 🧠 Uses buffered approach for S-side matches

Output: `RjoinS.tsv`

---

### 2️⃣ Union

Computes the **set union** of `R` and `S`, removing duplicates.

- ✅ Assumes both inputs are sorted  
- ✅ Skips duplicates via previous-line tracking

Output: `RunionS.tsv`

---

### 3️⃣ Intersection

Finds records **common to both** `R` and `S`.

- ✅ Assumes sorted input  
- ✅ Eliminates duplicate matches

Output: `RintersectionS.tsv`

---

### 4️⃣ Difference (R − S)

Returns records that exist in `R` but **not in** `S`.

- ✅ Assumes sorted input  
- ✅ Eliminates duplicates in output

Output: `RdifferenceS.tsv`

---

### 5️⃣ Group By + Aggregation

Groups records in `R` by key (column 1) and **sums** values in column 2.

- ✅ Uses recursive **merge sort with aggregation**  
- ✅ Works efficiently in-memory

Output: `Rgroupby.tsv`

## 📊 Internal Record Format

Each row is modeled using the following `Record` structure:

```cpp
struct Record {
    std::string column_1;  // key
    int column_2;          // value
};
```

## ⏱️ Performance

All operations (except group-by) are implemented using **streaming algorithms** that process files line-by-line **without loading them entirely into memory**.

| Operation         | Complexity (Approx.)      | Notes                                         |
|------------------|---------------------------|-----------------------------------------------|
| Merge Join       | O(n + m)                  | Linear scan of sorted R and S (fully streamed)|
| Union / Intersect| O(n + m)                  | Single-pass merge, no memory accumulation     |
| Difference       | O(n + m)                  | Streamed difference computation               |
| Group By + Sum   | O(n log n)                | Uses merge sort with aggregation (in-memory)  |

✅ Highly scalable for large input files due to minimal memory usage  
✅ Only the **group-by operation** loads records into memory for sorting  

## 📄 Output Details

Each result is written to a separate file:

| File Name           | Contents                                |
|---------------------|------------------------------------------|
| `RjoinS.tsv`        | Joined tuples on common key              |
| `RunionS.tsv`       | All unique tuples from R and S           |
| `RintersectionS.tsv`| Tuples common to both R and S            |
| `RdifferenceS.tsv`  | Tuples in R that do not appear in S      |
| `Rgroupby.tsv`      | Sum of values grouped by key in R        |

All output files are **tab-separated** and can be inspected or used in downstream processing.

---
## 👤 Author

> GitHub: [xrddev](https://github.com/xrddev)


## 📝 License

Released under the [MIT License](LICENSE). Originally built as part of a university project.

