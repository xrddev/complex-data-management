# 📦 Complex Data Management in C++

This project explores fundamental techniques in **complex data management**, implemented in modern C++.  
It includes components for:

- Relational algebra operations
- Keyword-based containment and relevance queries using inverted indexes
- Spatial indexing with bulk-loaded R-Trees

Each module is fully self-contained in its own folder, with dedicated documentation and code.

---

## 📁 Repository Structure

| Folder               | Description                                                                 |
|----------------------|-----------------------------------------------------------------------------|
| [`relational-operators/`](./relational-operators/) | Implements classic relational operators (join, union, intersection, etc.) on TSV data |
| [`query-processing/`](./query-processing/)         | Contains containment and relevance-based query processing on transactional datasets   |
| [`spatial-data/`](./spatial-data/)                 | Implements spatial indexing using bulk-loaded R-Trees                                 |

---

## 🛠️ Technologies

- Language: **C++20**
- Build: Manual (via `g++`)
- Input: `.tsv` and `.txt` datasets

---
## 👤 Author

> GitHub: [xrddev](https://github.com/xrddev)


## 📝 License

Released under the [MIT License](LICENSE). Originally built as part of a university project.
