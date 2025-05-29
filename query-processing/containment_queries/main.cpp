/**
 * @file main.cpp
 * @brief Implementation of various query processing methods for transaction datasets.
 *
 * This file implements four different query processing methods:
 * 1. Naive Method - Direct comparison of queries against transactions
 * 2. Signature File Method - Uses bit signatures for efficient matching
 * 3. Exact Bitslice Signature Method - Uses vertical bit representation
 * 4. Inverted File Method - Uses inverted index with set intersection
 *
 * Each method provides different trade-offs between processing speed and memory usage.
 * The implementation includes functionality for:
 * - Loading and parsing transaction and query datasets
 * - Computing and storing signatures
 * - Processing queries using different algorithmic approaches
 * - Measuring and reporting computation time
 * - Writing intermediate results to files for analysis
 *
 * @author xrddev
 * @date 2025-05-30
 */

#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <boost/multiprecision/cpp_int.hpp>


using Signature = std::vector<uint64_t>;
using QueryResult = std::unordered_map<int, std::unordered_set<int>>;
using ItemToTransactionMap = std::map<int, std::set<int>>;


std::vector<std::vector<int>> load_item_sets_from_file(const std::string& item_sets_file);
std::vector<QueryResult> run_method(const std::string& transactions_file, const std::string& queries_file, int query_number , int method_number);
inline void print_query_resulted_item_ids(const std::string& method_name, const std::unordered_set<int>& item_ids);



inline void process_single_query_naive(const std::vector<std::vector<int>>& transactions,const std::vector<int>& query, int query_number, QueryResult& query_results);
QueryResult naive_method(const std::string& transactions_file, const std::string& queries_file, int query_number);


Signature compute_signature(const std::vector<int>& item_set);
bool transaction_signature_covers_query(const Signature& transaction_sig, const Signature& query_sig);
inline void process_single_query_signature_file(const std::vector<Signature> &transaction_signatures, const Signature& query_signature, int query_number, QueryResult& query_results);
QueryResult signature_file_method(const std::string& transactions_file, const std::string& queries_file, int query_number);


std::map<int, boost::multiprecision::cpp_int> build_item_transactions_bit_map(const std::vector<std::vector<int>>& transactions);
void write_bitslice_signatures(const std::map<int, boost::multiprecision::cpp_int>& item_transactions_bit_map,std::ofstream& bitslice_file);
inline void process_single_query_exact_bitslice(const std::map<int, boost::multiprecision::cpp_int>& item_transactions_bit_map,const std::vector<int>& query_items_set,int query_number,QueryResult& query_results);
QueryResult exact_bitslice_signature_file(const std::string& transactions_file, const std::string& queries_file, int query_number);



std::map<int, std::set<int>> build_inverted_index(const std::vector<std::vector<int>>& transactions);
void write_inverted_index_to_file(const std::map<int, std::set<int>>& inverted_index);
inline void process_single_query_inverted_index(const std::map<int, std::set<int>>& inverted_index,const std::vector<int>& query_items_set,int query_number,QueryResult& query_results);
QueryResult inverted_file_with_intersection(const std::string& transactions_file, const std::string& queries_file, int query_number);


constexpr int NAIVE = 0;
constexpr int SIGNATURE_FILE = 1;
constexpr int EXACT_BITSLICE_SIGNATURE_FILE = 2;
constexpr int INVERTED_FILE = 3;



//
//Main Function
//
int main(const int argc, char* argv[]) {
    if (argc < 5) {
        std::cerr << "Invalid number of arguments" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <transactions.txt> <queries.txt> <qnum> <method>" << std::endl;
        return 1;
    }

    std::vector<QueryResult> results =
        run_method(argv[1],argv[2], std::stoi(argv[3]), std::stoi(argv[4]));
    return 0;
}


/**
 * Executes a specified query processing method on transaction and query files.
 *
 * The method processes the given transaction file and query file based
 * on the specified query number and method number. It supports different
 * query execution techniques, including naive method, signature file,
 * exact bitslice signature file, and inverted file with intersection.
 * It either returns results for the specified method or for all methods
 * if the method number is -1.
 *
 * @param transactions_file The file path containing the transactions data.
 * @param queries_file The file path containing the queries data.
 * @param query_number The identifier for the specific query to process.
 * @param method_number The identifier for the query processing method
 *                      (0 for naive, 1 for signature file,
 *                      2 for exact bitslice signature file,
 *                      3 for inverted file, and -1 for all methods).
 * @return A vector containing four QueryResult elements, each corresponding
 *         to results from a specific query processing method.
 */
std::vector<QueryResult> run_method(
    const std::string& transactions_file,
    const std::string& queries_file,
    const int query_number ,
    const int method_number) {

    std::vector<QueryResult> method_results(4);

    switch(method_number) {
        case 0  :
            method_results[NAIVE] = naive_method(transactions_file, queries_file, query_number);
            break;

        case 1:
            method_results[SIGNATURE_FILE] = signature_file_method(transactions_file, queries_file, query_number);
            break;

        case 2:
            method_results[EXACT_BITSLICE_SIGNATURE_FILE] = exact_bitslice_signature_file(transactions_file,queries_file,query_number);
            break;

        case 3:
            method_results[INVERTED_FILE] = inverted_file_with_intersection(transactions_file,queries_file,query_number);
            break;

        case -1:
            method_results[NAIVE] = naive_method(transactions_file, queries_file, query_number);
            method_results[SIGNATURE_FILE] = signature_file_method(transactions_file, queries_file, query_number);
            method_results[EXACT_BITSLICE_SIGNATURE_FILE] = exact_bitslice_signature_file(transactions_file,queries_file,query_number);
            method_results[INVERTED_FILE] = inverted_file_with_intersection(transactions_file,queries_file,query_number);
            break;

        default:
            std::cerr << "Invalid method: " << method_number << std::endl;
            exit(-1);
            break;
    }
    return method_results;
}

/**
 * Parses a file containing item sets and loads them into a vector of vectors of integers.
 * Each line in the file represents a transaction, with integer items separated in a suitable format.
 * Ignores non-numeric characters like '[', ']', ',', and spaces while parsing.
 *
 * If the file cannot be opened, the program terminates with an error message.
 *
 * @param item_sets_file The name or path of the file containing item sets to be loaded.
 * @return A vector of transactions, where each transaction is a vector of integers representing items.
 */
std::vector<std::vector<int>> load_item_sets_from_file(const std::string& item_sets_file) {
    std::ifstream file(item_sets_file);
    if (!file) {
        std::cerr << "Could not open file " << item_sets_file << std::endl;
        std::exit(-1);
    }

    std::vector<std::vector<int>> transactions;
    std::string line;

    while (std::getline(file, line)) {
        std::vector<int> transaction;
        int number = 0;
        bool building = false;

        for (char c : line) {
            if (c >= '0' && c <= '9') {
                number = number * 10 + (c - '0');   // Convert digit character to int
                building = true;
            } else if (building) {
                transaction.push_back(number);
                number = 0;
                building = false;
            }
            // ignore anything else: '[', ']', ',', ' '
        }

        if (building) {
            transaction.push_back(number);
        }

        transactions.push_back(transaction);
    }

    return transactions;
}

/**
 * Prints the IDs of items in the result set of a query, formatted as a comma-separated list.
 *
 * @param method_name The name of the query method being used.
 * @param item_ids A set of integers representing the IDs of items resulting from the query.
 */
inline void print_query_resulted_item_ids(const std::string& method_name, const std::unordered_set<int>& item_ids) {
    std::cout << method_name << " Method result:" << std::endl << "{";

    size_t comma_count = !item_ids.empty() ? item_ids.size() - 1 : 0;
    for (const int t_id : item_ids)
        std::cout << t_id << (comma_count-- > 0 ? "," : "");

    std::cout << "}" << std::endl;
}



////
///Naive Method

/**
 * Processes queries using the naive method by comparing each query against all transactions.
 *
 * The function executes a naive query processing approach where each query is
 * compared against every transaction in the dataset. For a specific query number,
 * the result of that query is printed and stored. If all queries need to be processed,
 * the method iterates through all queries and computes results for each one.
 *
 * Additionally, the method performs timing measurement and prints the
 * total computation time required to process the query or queries.
 *
 * @param transactions_file The file path containing the transactions data.
 * @param queries_file The file path containing the queries data.
 * @param query_number The identifier for the specific query to process.
 *                     If -1 is passed, all queries are processed.
 * @return A QueryResult which maps query indices to the set of matched transaction IDs.
 */
QueryResult naive_method(const std::string& transactions_file, const std::string& queries_file, const int query_number) {
    const std::vector<std::vector<int>> transactions = load_item_sets_from_file(transactions_file);
    const std::vector<std::vector<int>> queries = load_item_sets_from_file(queries_file);
    QueryResult query_results;

    const auto start = std::chrono::high_resolution_clock::now();

    if(query_number == -1) {
        for(int i = 0; i < queries.size(); i++)
            process_single_query_naive(transactions,queries[i], i, query_results);

    }else {
        process_single_query_naive(transactions,queries[query_number],query_number,query_results);
        print_query_resulted_item_ids("Naive", query_results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double> duration = end - start;
    std::cout << "Naive Method computation time = " << duration.count() << " seconds\n";
    return query_results;
}

/**
 * Processes a single query against a list of transactions using the naive method.
 *
 * This method checks if all items in the query are present in each transaction.
 * For each transaction containing all items from the query, the transaction's
 * identifier is added to the results for the query. The results are updated
 * in the provided query_results map for the given query number.
 *
 * @param transactions A vector of transactions where each transaction is a vector of integers representing item IDs.
 * @param query A vector of integers representing the items in the query to process.
 * @param query_number The index of the query, used for storing results in the query_results map.
 * @param query_results A reference to a map where each query number maps to a set of transaction IDs
 *                      that satisfy the query.
 */
inline void process_single_query_naive(
    const std::vector<std::vector<int>>& transactions,
    const std::vector<int>& query, const int query_number,
    QueryResult& query_results) {

    for (int transaction_id = 0; transaction_id < transactions.size(); transaction_id++) {
        bool all_found = true;

        for (const int query_item_id : query) {
            bool found = false;

            for (const int transaction_item_id : transactions[transaction_id]) {
                if (query_item_id == transaction_item_id) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                all_found = false;
                break;
            }
        }

        if (all_found)
            query_results[query_number].insert(transaction_id);
    }
}


///
//Signature File

/**
 * Computes a binary signature for a given set of items.
 *
 * The method generates a compact representation of an item set
 * as a vector of 64-bit words. Each bit in the signature corresponds
 * to whether a specific item is present in the input set. The item's
 * index determines the position of the bit in the signature. This
 * representation is useful for efficient set-based operations, such as
 * query matching.
 *
 * @param item_set The set of items for which the signature is to be computed.
 *                 Each item is represented as an integer.
 * @return A signature representing the input item set as a vector of 64-bit integers.
 */
Signature compute_signature(const std::vector<int>& item_set) {
    Signature signature;

    for (const int item : item_set) {
        const size_t word_index = item / 64;
        const size_t bit_index = item % 64;

        if (word_index >= signature.size()) {
            signature.resize(word_index + 1, 0);
        }

        signature[word_index] |= (1ULL << bit_index);
    }
    return signature;
}

/**
 * Checks if a transaction signature covers a query signature.
 *
 * This function determines whether all bits set in the query signature
 * are also set in the corresponding positions of the transaction signature.
 * It ensures that the transaction signature has equal or greater length than
 * the query signature and performs a bitwise check to verify that all bits
 * set in the query are also set in the transaction.

 *
 * @param transaction_sig The signature of the transaction to be checked.
 * @param query_sig The signature of the query being compared to the transaction.
 * @return True if the transaction signature covers the query signature,
 *         false otherwise.
 */
bool transaction_signature_covers_query(const Signature& transaction_sig, const Signature& query_sig) {
    if (transaction_sig.size() < query_sig.size()) return false;

    for (size_t i = 0; i < query_sig.size(); ++i)
        if ((transaction_sig[i] & query_sig[i]) != query_sig[i]) return false;

    return true;
}

/**
 * Processes a single query using the signature file approach.
 *
 * The method compares the query signature against each transaction signature
 * in the provided list of transaction signatures. For each transaction that
 * satisfies the query (i.e., its signature covers the query signature), the
 * transaction ID is added to the result set for the specified query.
 *
 * @param transaction_signatures A vector of signatures representing transactions.
 * @param query_signature The signature of the query to be processed.
 * @param query_number The index of the query being processed in the result set.
 * @param query_results A reference to the data structure storing the query results.
 *                      The method updates this structure by adding the IDs of
 *                      the transactions that match the query.
 */
inline void process_single_query_signature_file(
    const std::vector<Signature> &transaction_signatures,
    const Signature& query_signature,
    const int query_number,
    QueryResult& query_results) {
    for(int t_id = 0 ; t_id < transaction_signatures.size(); t_id++) {
        if(transaction_signature_covers_query(transaction_signatures[t_id], query_signature))
            query_results[query_number].insert(t_id);
    }
}

/**
 * Processes queries against a transaction file using the signature file method.
 *
 * This method generates signatures for transactions and queries, writes the
 * transaction signatures to a file, and matches the queries against the
 * transaction signatures using the signature file method. If the query_number
 * is -1, all queries are processed; otherwise, only the specified query is
 * processed.
 *
 * Additionally, the method performs timing measurement and prints the
 * total computation time required to process the query or queries.
 *
 * @param transactions_file The file path containing transaction data.
 * @param queries_file The file path containing query data.
 * @param query_number The query identifier to process. If -1, all queries are processed.
 * @return The results of the query processing as a QueryResult unordered map,
 *         where the key represents the query identifier and the value is a set
 *         of matching transaction identifiers.
 */
QueryResult signature_file_method(const std::string& transactions_file, const std::string& queries_file, int query_number) {
    const std::vector<std::vector<int>> transactions = load_item_sets_from_file(transactions_file);
    const std::vector<std::vector<int>> queries = load_item_sets_from_file(queries_file);
    QueryResult query_results;


    //Generate signatures for transactions.
    std::vector<Signature> transaction_signatures;
    for (const std::vector<int>& transaction : transactions)
        transaction_signatures.push_back(compute_signature(transaction));

    //Generate signatures for queries.
    std::vector<Signature> query_signatures;
    for(const std::vector<int>& query : queries)
        query_signatures.push_back(compute_signature(query));


    //Write signatures to the file.
    std::ofstream signatures_file("sigfile.txt");
    if(!signatures_file) {
        std::cerr << "Could not open file sigfile.txt" << std::endl;
        std::exit(-1);
    }

    for(const Signature& signature : transaction_signatures) {
        for (uint64_t word : signature)
            signatures_file << word;
        signatures_file << std::endl;
    }




    const auto start = std::chrono::high_resolution_clock::now();

    if(query_number == -1) {
       for(int i = 0; i < query_signatures.size(); i ++)
           process_single_query_signature_file(transaction_signatures,query_signatures[i],i,query_results);

    }else {
        process_single_query_signature_file(transaction_signatures,query_signatures[query_number], query_number,query_results);
        print_query_resulted_item_ids("Signature File",query_results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();


    const std::chrono::duration<double> duration = end - start;
    std::cout << "Signature File computation time = " << duration.count() << " seconds\n";
    return query_results;
}



///
//Exact Bitslice Signature

/**
 * Constructs a mapping between each unique item and a bit-encoded representation
 * of the transactions that contain the item.
 *
 * Each transaction is represented by a bit in a large integer (boost::multiprecision::cpp_int).
 * For a given item, the bits in its associated integer mark the indices of the transactions
 * where the item appears.
 *
 * @param transactions A vector of transactions, where each transaction is represented
 *                     as a vector of integers (item identifiers).
 * @return A map where the key is an item identifier (int) and the value is a
 *         boost::multiprecision::cpp_int, with bits set to indicate the transaction
 *         indices where the item is present.
 */
std::map<int, boost::multiprecision::cpp_int> build_item_transactions_bit_map(const std::vector<std::vector<int>>& transactions) {
    std::map<int, boost::multiprecision::cpp_int> item_transactions_bit_map;

    for (int i = 0; i < transactions.size(); ++i) {
        for (int item : transactions[i]) {
            // Create a big integer with only the i-th bit set (used to mark transaction i).
            const boost::multiprecision::cpp_int bit = boost::multiprecision::cpp_int(1) << i;
            item_transactions_bit_map[item] |= bit; // BitWise OR
        }
    }
    return item_transactions_bit_map;
}

/**
 * Writes the bit-slice signatures for item transactions to an output file.
 *
 * This function iterates over a map of item transactions and their corresponding
 * bit-slice signatures, and writes the item ID along with its signature to
 * the provided file stream.
 *
 * @param item_transactions_bit_map A map where the key is the item ID and
 *                                  the value is the bit-slice signature
 *                                  (represented as a boost::multiprecision::cpp_int).
 * @param bitslice_file The output file stream where the item ID and its
 *                      signature will be written.
 */
void write_bitslice_signatures(const std::map<int, boost::multiprecision::cpp_int>& item_transactions_bit_map,std::ofstream& bitslice_file){
    for (const auto& [item, signature] : item_transactions_bit_map) {
        bitslice_file << item << ": " << signature << '\n';
    }
}

/**
 * Processes a single query against transaction data using the exact bitslice signature method.
 *
 * This method takes a query represented as a set of items and determines which transactions
 * contain all the items in the query. It uses bitwise operations on bitmaps representing
 * item-to-transaction mappings to efficiently compute the results. The resulting transaction
 * IDs are stored in the specified QueryResult object for the given query.
 *
 * @param item_transactions_bit_map A mapping of item identifiers to bitmaps where each bit
 *                                  represents the presence or absence of the item in a transaction.
 * @param query_items_set A vector of item identifiers representing the items in the query.
 * @param query_number The index of the query being processed.
 * @param query_results A reference to the QueryResult object where results of the processed
 *                      query are stored.
 */
inline void process_single_query_exact_bitslice(
    const std::map<int,
    boost::multiprecision::cpp_int>& item_transactions_bit_map,
    const std::vector<int>& query_items_set,
    const int query_number,
    QueryResult& query_results){

    //Query is empty, just in case ;)
    if (query_items_set.empty()) return;

    // Initialize the result with the bitmap of the first item
    const int query_first_item = query_items_set[0];
    auto it = item_transactions_bit_map.find(query_first_item);

    //Item doesnt exist at any transaction
    if (it == item_transactions_bit_map.end()) return;


    //Begin with the bitmap of the first item of the query.
    boost::multiprecision::cpp_int result_bitmap = it->second;

    //for the rest of the query items.
    for (size_t i = 1; i < query_items_set.size(); ++i) {
        it = item_transactions_bit_map.find(query_items_set[i]);
        if (it == item_transactions_bit_map.end()) {
            result_bitmap = 0;  // If any item not found, there is no transaction that exists all query items.
            break;
        }

        result_bitmap &= it->second;
    }


    // Extract transaction ids from result_bitmap
    for (int t_id = 0; result_bitmap != 0; ++t_id) {
        if ((result_bitmap & 1) != 0) {
            query_results[query_number].insert(t_id);
        }
        result_bitmap >>= 1;
    }
}

/**
 * Processes a query using the exact bitslice signature file method.
 *
 * This method reads transaction and query datasets from their respective
 * files, builds a bitslice signature representation for the transactions,
 * and processes the specified query or all queries, depending on the input.
 * The results are computed and stored in a QueryResult data structure.
 *
 * Additionally, the method performs timing measurement and prints the
 * total computation time required to process the query or queries.
 *
 * @param transactions_file The file path containing the transactions data.
 * @param queries_file The file path containing the queries data.
 * @param query_number The identifier for the specific query to process.
 *                     If -1, all queries are processed.
 * @return A QueryResult object containing the results of the processed
 *         query or queries.
 */
QueryResult exact_bitslice_signature_file(const std::string& transactions_file, const std::string& queries_file, int query_number) {
    const std::vector<std::vector<int>> transactions = load_item_sets_from_file(transactions_file);
    const std::vector<std::vector<int>> queries = load_item_sets_from_file(queries_file);
    QueryResult query_results;

    const auto item_transactions_bit_map = build_item_transactions_bit_map(transactions);

    std::ofstream bitslice_file("bitslice.txt");
    if (!bitslice_file) {
        std::cerr << "Could not open bitslice.txt for writing\n";
        std::exit(-1);
    }

    write_bitslice_signatures(item_transactions_bit_map, bitslice_file);


    const auto start = std::chrono::high_resolution_clock::now();

    if (query_number == -1) {
        for (int i = 0; i < queries.size(); i++)
            process_single_query_exact_bitslice(item_transactions_bit_map, queries[i], i, query_results);
    } else {
        process_single_query_exact_bitslice(item_transactions_bit_map, queries[query_number], query_number, query_results);
        print_query_resulted_item_ids("Exact Bitslice Signature", query_results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = end - start;
    std::cout << "Exact Bitslice Signature computation time = " << duration.count() << " seconds\n";
    return query_results;
}



//
//Inverted Index

/**
 * Constructs an inverted index from a given list of transactions.
 *
 * The method processes a collection of transactions and builds an inverted
 * index mapping each item to the set of transaction indices where the item
 * is present. This index is useful for efficiently processing queries.
 *
 * @param transactions A vector of vectors where each inner vector represents
 *                      a transaction containing a list of items.
 * @return A map where each key is an item from the transactions, and the
 *         corresponding value is a set of transaction indices in which the
 *         item appears.
 */
std::map<int, std::set<int>> build_inverted_index(const std::vector<std::vector<int>>& transactions) {
    std::map<int, std::set<int>> inverted_index;

    for (int i = 0; i < transactions.size(); i++)
        for (int item : transactions[i])
            inverted_index[item].insert(i);

    return inverted_index;
}

/**
 * Writes the contents of an inverted index to a file named "invfile.txt".
 *
 * The function serializes the provided inverted index structure, where each
 * key corresponds to an item and its associated set of transaction IDs.
 * The results are written to the file in a human-readable format, with each
 * line representing an item and the list of transaction IDs.
 *
 * @param inverted_index A map representing the inverted index, where each key is
 *                       an item, and the value is a set of transaction IDs
 *                       associated with that item. Each item is mapped to
 *                       all transactions in which it appears.
 */
void write_inverted_index_to_file(const std::map<int, std::set<int>>& inverted_index) {
    std::ofstream outfile("invfile.txt");
    if (!outfile) {
        std::cerr << "Could not open " << "invfile.txt" << " for writing\n";
        std::exit(-1);
    }

    for (const auto& [item, txn_ids] : inverted_index) {
        outfile << item << ": [";
        for (auto it = txn_ids.begin(); it != txn_ids.end(); ++it) {
            outfile << *it;
            if (std::next(it) != txn_ids.end()) outfile << ", ";
        }
        outfile << "]\n";
    }
}

/**
 * Executes query processing using the inverted file with intersection method.
 *
 * This method builds an inverted index from the given transactions file and
 * processes queries from the queries file. It either processes a single query
 * identified by the `query_number` or processes all queries if `query_number`
 * is -1. The inverted index is stored in a file for inspection, and the
 * computation time for processing is logged.
 *
 * @param transactions_file The file path containing the transactions data.
 * @param queries_file The file path containing the queries data.
 * @param query_number The identifier for the specific query to process.
 *                     Use -1 to process all queries.
 * @return A QueryResult object containing query result sets, where each
 *         key corresponds to a query ID and values are the sets of
 *         transaction IDs resulting from the query.
 */
QueryResult inverted_file_with_intersection(const std::string& transactions_file, const std::string& queries_file, int query_number) {
    const std::vector<std::vector<int>> transactions = load_item_sets_from_file(transactions_file);
    const std::vector<std::vector<int>> queries = load_item_sets_from_file(queries_file);
    const std::map<int, std::set<int>> inverted_index = build_inverted_index(transactions);

    write_inverted_index_to_file(inverted_index);

    QueryResult query_results;

    const auto start = std::chrono::high_resolution_clock::now();

    if (query_number == -1) {
        for (int i = 0; i < queries.size(); i++)
            process_single_query_inverted_index(inverted_index, queries[i], i, query_results);
    } else {
        process_single_query_inverted_index(inverted_index, queries[query_number], query_number, query_results);
        print_query_resulted_item_ids("Inverted File", query_results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = end - start;
    std::cout << "Inverted File computation time = " << duration.count() << " seconds\n";

    return query_results;
}

/**
 * Processes a single query using an inverted index to find matching transactions.
 *
 * This method checks for transactions that contain all items in the given query
 * using the inverted index. It uses a sorted intersection algorithm to efficiently
 * identify the transactions that satisfy the query. The results are stored in the
 * provided query results structure for the given query number.
 *
 * @param inverted_index The pre-built inverted index mapping item IDs to sets of
 *                       transaction IDs that contain these items.
 * @param query_items_set A vector containing the item IDs specified in the query.
 *                        These items are processed to find intersections.
 * @param query_number The identifier for the specific query being processed.
 *                     Used to store results in the correct position.
 * @param query_results The structure where the identified transaction IDs for
 *                      the query are stored. Results are indexed by query number.
 */
inline void process_single_query_inverted_index(
    const std::map<int, std::set<int>>& inverted_index,
    const std::vector<int>& query_items_set,
    const int query_number,
    QueryResult& query_results){

    //Edge case, a query is empty.
    if (query_items_set.empty()) return;


    const int query_first_item_id = query_items_set[0];
    auto it = inverted_index.find(query_first_item_id);
    if (it == inverted_index.end()) return;//If it doesn't exist means no transaction exists with this item id.


    // Sets in C++ are implemented as red-black trees, so their elements are always sorted.
    // Copying them to vectors preserves the sorted order.
    // This allows us to run the sorted input lists intersection algorithm from exercise 1.


    //Current vector = transaction id list of the first query item.
    std::vector current(it->second.begin(), it->second.end());


    //for the rest of the query items.
    for (size_t i = 1; i < query_items_set.size(); ++i) {
        it = inverted_index.find(query_items_set[i]);
        if (it == inverted_index.end()) {
            current.clear();
            break;//Same as before. If it doesn't exist means no transaction exists with this item id.
        }

        //Candidate vector = transaction id list of the ith query item
        std::vector candidate(it->second.begin(), it->second.end());


        std::vector<int> temp;
        size_t v1_pointer = 0;
        size_t v2_pointer = 0;

        while (v1_pointer < current.size() && v2_pointer < candidate.size()) {
            if (current[v1_pointer] < candidate[v2_pointer]) {
                ++v1_pointer;
            } else if (current[v1_pointer] > candidate[v2_pointer]) {
                ++v2_pointer;
            } else {
                temp.push_back(current[v1_pointer]); //No duplicates because of the set. No extra actions are needed.
                ++v1_pointer;
                ++v2_pointer;
            }
        }

        current = std::move(temp);
        if (current.empty()) break; //Intersection is empty
    }

    for (int transaction_id : current) {
        query_results[query_number].insert(transaction_id);
    }
}



