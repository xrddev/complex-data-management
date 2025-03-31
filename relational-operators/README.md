# Complex Data Management

## Assignment 1: Implementation of Relational Operators

---

##### How to run:

```bash
g++ main.cpp
./a.out R_sorted.tsv S_sorted.tsv R.tsv
```

---

#### Merge-join:

We read the 2 input files line by line, making only one pass through each one according to the assignment requirements.

At the beginning, we read the first line from both files.

* If the key in the record from file r_sorted is lexicographically smaller than the key in the record from file s_sorted, we move to the next record in r_sorted while keeping the same record from s_sorted.

* If the key in the record from file r_sorted is lexicographically greater than the key in the record from file s_sorted, we move to the next record in s_sorted while keeping the same record from r_sorted.

* If the key in the record from file r_sorted matches lexicographically the key in the record from file s_sorted, then:
  
  * As long as the records in file s_sorted have the same key as the record in file r_sorted, we add those records to the buffer.
  
  * As soon as the keys of the s_sorted records no longer match the key from r_sorted, we perform the merge-join of the r_sorted key with all s_sorted records in the buffer.
  
  * If file r_sorted has more than one record with that same key that matched with s_sorted, we perform merge-join again with all records from the s buffer.
  
  * Finally, we clear the buffer and keep the maximum size it reached so far.

There are also comments in the code for more implementation details.

---

#### Union:

We read the 2 input files line by line, making only one pass through each one according to the assignment requirements.

At the beginning, we read the first line from both files.

* If both files have records:
  
  * If the record from r_sorted is lexicographically smaller than the one from s_sorted, we write the r_sorted record to the output file and move forward only in r_sorted.
  
  * If the record from r_sorted is lexicographically greater than the one from s_sorted, we write the s_sorted record to the output file and move forward only in s_sorted.
  
  * If the records match, we write the common record only once and move forward in both files.

* If only r_sorted has remaining records and s_sorted is either finished or initially empty, we write all remaining records from r_sorted to the output file.

* If only s_sorted has remaining records and r_sorted is either finished or initially empty, we write all remaining records from s_sorted to the output file.

To ensure no duplicates in the output file, before writing a record, we check if it matches the last record written, and if so, we ignore it. Since the input files are sorted, all duplicates will appear sequentially and this check is sufficient.

---

#### Intersection:

We read the 2 input files line by line, making only one pass through each one according to the assignment requirements.

At the beginning, we read the first line from both files.

- If both files have records:
  
  - If the record from r_sorted is lexicographically smaller than the one from s_sorted, we move forward in r_sorted and write nothing.
  
  - If the record from r_sorted is lexicographically greater than the one from s_sorted, we move forward in s_sorted and write nothing.
  
  - If the records match, we write the common record only once and move forward in both files.

To ensure no duplicates in the output file, before writing a record, we check if it matches the last record written, and if so, we ignore it. Since the input files are sorted, all duplicates will appear sequentially and this check is sufficient.

---

#### Difference (R_sorted - S_sorted):

We read the 2 input files line by line, making only one pass through each one according to the assignment requirements.

At the beginning, we read the first line from both files.

- While r_sorted still has records:
  
  - If s_sorted has no records at this point or the r_sorted record is lexicographically smaller than the one in s_sorted, we write the r_sorted record to the output file and move forward in r_sorted.
  
  - If the r_sorted record is lexicographically greater than the one in s_sorted, we move forward in r_sorted.
  
  - If the records are the same in both files, we move forward in both files.

To ensure no duplicates in the output file, before writing a record, we check if it matches the last record written, and if so, we ignore it. Since the input files are sorted, all duplicates will appear sequentially and this check is sufficient.

---

#### RgroupBy with Aggregation:

We create a struct named `Record` in order to store and manage the records of file R efficiently.

We create a list that contains all records from file R.tsv, with each `Record` representing one entry.

We execute the merge sort algorithm on the elements of this list, and in the end, we write the resulting records to the output file.

The implementation of the mergeSort algorithm is exactly the same as the standard one, with the only difference being that when merging the sorted lists, in the case of a tie (i.e., same key), a new `Record` is created. The key of the new object remains the same, while the second field is the sum of the corresponding fields of the two elements, which is why the function is named `merge_with_aggregation`.

All other steps of the code follow the logic of standard merge sort.

---# complex-data-management
