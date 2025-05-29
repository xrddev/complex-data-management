# 🔍 Transaction Query Processing in C++

This project implements and compares **four query processing methods** over transaction datasets using C++. The goal is to provide both functional correctness and performance benchmarking.

## 📁 Project Structure

- `main.cpp`: Main source file that includes:
    - Naive Method
    - Signature File Method
    - Exact Bitslice Signature Method
    - Inverted File Method

- `transactions.txt`, `queries.txt`: Input files containing datasets.

- Output files (for inspection):
    - `sigfile.txt`: Signature representations
    - `bitslice.txt`: Bit-slice signatures
    - `invfile.txt`: Inverted index dump


### 🔨 Compile

```bash
g++ -std=c++17 -O2 main.cpp -o query_processor
```

> Note: If your compiler already defaults to C++17 or newer, you may omit `-std=c++17`.

### ▶️ Execute

```bash
./query_processor transactions.txt queries.txt <query_number> <method_number>
```

Arguments:

    <query_number>     Query index to process (use -1 to process all queries)
    <method_number>    Method to use:
                       0  → Naive Method
                       1  → Signature File
                       2  → Exact Bitslice Signature File
                       3  → Inverted File
                      -1  → Run all methods

## 🧠 Methods Overview

### 1️⃣ Naive Method
Performs a brute-force search by comparing each query with every transaction.
- ✅ Accurate and straightforward to implement
- ❌ Extremely inefficient for large datasets
- 💡 Best suited for baseline performance comparison

---

### 2️⃣ Signature File Method
Represents transactions and queries using dynamically-sized bit-vectors (signatures).  
Each item is directly mapped to a specific bit based on its ID, ensuring exact representation.

Unlike traditional signature file implementations that use hash functions and may introduce false positives,  
**this implementation uses direct bit-position mapping**, providing exact containment checks with no false positives.

- ✅ Accurate bit-based representation (no false positives)
- ✅ Scales naturally for large item domains
- ✅ Efficient set containment check using bitwise operations
- ⚠️ Signature size grows with the maximum item ID

---

### 3️⃣ Exact Bitslice Signature Method
Unlike the Signature File Method, which represents each **transaction** as a bit-vector,  
this method represents each **item** as a vertical **bit column** (bitmap) across all transactions.

It performs efficient intersection of these bitmaps to determine exact query matches.

- ✅ No false positives — results are exact
- ✅ High performance with **bitwise AND** operations
- ⚠️ Requires use of **multiprecision integers** for large datasets


---

### 4️⃣ Inverted File Method
Builds an **inverted index** mapping each item to the transactions it appears in.  
Each query is processed via **sorted list intersection**.
- ✅ Very efficient for sparse queries and large datasets
- ✅ Uses standard data structures (maps and sets)
- ⚠️ Needs preprocessing but yields fast query times

## ⏱️ Performance

Each method prints its own computation time to `stdout` using high-resolution clocks.  
This allows easy benchmarking and comparison between the following approaches:

| Method                        | Time Complexity (approx.)     | Notes                                                  |
|------------------------------|-------------------------------|--------------------------------------------------------|
| Naive                        | O(Q × T × I)                  | Q: queries, T: transactions, I: items                 |
| Signature File               | O(Q × T × S)                  | S: signature size (depends on max item ID)            |
| Exact Bitslice Signature     | O(Q × M × B)                  | M: query length, B: bits per bitmap (≈ #transactions) |
| Inverted File (Intersection) | O(Q × M × log T)              | M: query length, T: transaction count                 |

All methods report timings in seconds and can be directly compared.

> 💡 Tip: You can test scalability by running with different dataset sizes and enabling `-1` mode to process all queries.


## 🛠️ Dependencies

This project is written in modern C++ and uses one external library:

- **Boost.Multiprecision**: Required for the Exact Bitslice Signature method

### 📦 Install Boost (on Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install libboost-all-dev
```

No additional build system is required — you can compile directly with `g++` using the `-std=c++17` flag.


## 📄 Output Files Explained

During execution, the program generates the following intermediate files:

- `sigfile.txt`:  
  Contains the bit signatures for all transactions (one per line).  
  Used by the Signature File method.

- `bitslice.txt`:  
  Maps each item ID to a large bit-vector indicating in which transactions the item appears.  
  Used by the Exact Bitslice Signature method.

- `invfile.txt`:  
  Lists the inverted index in a human-readable format (item → transaction IDs).  
  Used by the Inverted File method.

These files help with debugging, inspection, or offline analysis.

## 👤 Author

> GitHub: [xrddev](https://github.com/xrddev)


## 📝 License

This project is licensed under the [MIT License](LICENSE).
