# 📊 Transaction Relevance Scoring Query Processing in C++

This project implements and compares **two relevance-based query processing methods** for transaction datasets, written in modern C++. It focuses on ranking transactions in order of their relevance to user queries using a weighting mechanism based on item rarity (Transaction Rarity Factor, or TRF).

## 📁 Project Overview

- **Main Features**:
    - Naive full-scan query processing
    - Inverted index-based efficient scoring
    - TRF-weighted ranking of transactions per query

- **Primary File**:
    - `main.cpp`: Core logic for both query evaluation strategies

- **Input Files**:
    - `transactions.txt`: Contains lists of item IDs per transaction
    - `queries.txt`: Contains lists of item IDs per query

- **Output File**:
    - `invfileocc.txt`: Human-readable dump of inverted index + TRF weights


## 🔧 Compilation

This project requires a C++20-compliant compiler.

Compile with:

    g++ -std=c++20 -O1 main.cpp

Note:
Compilation requires Boost.Multiprecision for TRF-weighted ranking and
C++20 features such as `std::ranges`.

## ▶️ Execution

Run with:

    ./a.out transactions.txt queries.txt <query_number> <method_number> <top_k>

Arguments:

    <query_number>     Index of query to process
                       Use -1 to process all queries

    <method_number>    Method to use:
                          0 → Naive Method
                          1 → Inverted Index Method
                         -1 → Run both methods

    <top_k>            Return only the top-k most relevant transactions per query

Example:

    ./a.out transactions.txt queries.txt 0 -1 2

    → Runs query 0 using both methods and returns the top 2 relevant transactions per method.


## 🧠 Methods Overview

This project implements two relevance-based query processing methods:

### 1️⃣ Naive Method

Performs a brute-force search across all transactions.

- ✅ Simple to implement
- ✅ Useful as a correctness baseline
- ❌ Inefficient for large datasets

Each transaction is scanned in full for every query. Matching items contribute to the relevance score based on their frequency and TRF weight.

---

### 2️⃣ Inverted Index Method

Builds an inverted index mapping each item to a list of transactions where it appears,
along with its occurrence count.

- ✅ Much faster for large datasets
- ✅ Efficient union of posting lists per query
- ✅ Uses TRF (Transaction Rarity Factor) to compute relevance
- ⚠️ Requires index construction before processing queries

For each query:
- Merges posting lists of all query items
- Computes a relevance score for each matching transaction
- Ranks and returns top-k results

## 📊 Relevance Scoring with TRF

The system ranks transactions based on how "important" each query item is within the dataset.

This is done using the **Transaction Rarity Factor (TRF)**:

    TRF(i) = total_transactions / number_of_transactions_containing_item_i

- Items that appear rarely across transactions get a **higher TRF weight**
- Frequent items contribute less to the final relevance score

### Relevance Score (per transaction):

    rel(query, transaction) = ∑ [occurrences_of_item × TRF(item)]

Where the sum is taken over all items in the query that also appear in the transaction.

The resulting scores are sorted in **descending order**, and only the top-k are returned (if specified).

## ⏱️ Performance and Complexity

Both methods report their computation time in seconds using high-resolution clocks.

This helps benchmark the efficiency of each approach across different dataset sizes.

| Method             | Time Complexity (Approx.)     | Description                                   |
|--------------------|-------------------------------|-----------------------------------------------|
| Naive              | O(Q × T × I)                  | Q: queries, T: transactions, I: items/query   |
| Inverted Index     | O(Q × M × log T)              | M: query length, T: transaction count         |

- ✅ Use Naive for correctness validation or small datasets
- ✅ Use Inverted Index for scalable performance on large inputs

You can test performance by setting `<query_number>` to -1 and running the program on larger datasets.

## 🛠️ Dependencies

This project is written in modern C++ and uses one external library:

- Boost.Multiprecision  
  Used for handling large integers and precision-safe sorting in bitmap operations.

### 📦 Install Boost (on Ubuntu/Debian)

Run the following command to install all Boost libraries:

    sudo apt-get update
    sudo apt-get install libboost-all-dev

No additional build system is required — compilation is done directly with `g++`.

## 📄 Output Files

During execution, the program may generate the following output file:

- invfileocc.txt  
  Contains a human-readable version of the inverted index and TRF weights.  
  Each line includes:
  • item ID  
  • TRF weight  
  • Posting list: [transaction_id, item_occurrence_count] pairs

This file is useful for:
- Debugging inverted index construction
- Understanding how item rarity (TRF) affects scoring
- Offline inspection or comparison

## 👤 Author

> GitHub: [xrddev](https://github.com/xrddev)


## 📝 License

Released under the [MIT License](LICENSE). Originally built as part of a university project.


