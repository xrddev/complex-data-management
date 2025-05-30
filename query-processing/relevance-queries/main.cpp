#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <map>
#include <ranges>
#include <set>
#include <unordered_map>
#include <boost/multiprecision/cpp_int.hpp>

/**
 * @file main.cpp
 * @brief Implementation of a transaction search and ranking system using inverted indices.
 *
 * This system implements two methods for searching and ranking transactions based on query relevance:
 * 1. Naive Method - A direct comparison approach that scans all transactions for each query.
 * 2. Inverted Index Method - An optimized approach using pre-built inverted indices.
 *
 * The system uses Transaction Rarity Factor (TRF) weights to score the relevance of items,
 * where items appearing in fewer transactions receive higher weights. Results are ranked
 * by their overall relevance scores, which combine item frequencies and TRF weights.
 *
 * Key Components:
 * - Inverted Index: Maps items to their occurrences in transactions
 * - TRF Weights: Stores rarity-based weights for each item
 * - Query Processing: Both naive and index-based methods for query evaluation
 * - Result Ranking: Scores and sorts transactions by relevance to queries
 **/


// Inverted Index: maps each item_id to a list of (transaction_id, occurrence_count).
// Enables efficient lookup of all transactions containing a given item.
using InvertedIndex = std::map<int, std::vector<std::pair<int, int>>>;

// TRF Weights: maps each item_id to its Transaction Rarity Factor (TRF).
// Computed as: total_transactions / number_of_transactions_containing_the_item.
using TRFWeights = std::map<int, double>;

// Relevance Inverted Index: pairs the inverted index with TRF weights.
// Used for scoring transactions based on query relevance.
using RelevanceInvertedIndex = std::pair<InvertedIndex, TRFWeights>;

// RelevanceScoreList: a list of (relevance_score, transaction_id) pairs,
// sorted in descending order of relevance.
using RelevanceScoreList = std::vector<std::pair<double, int>>;

// QueryResult: maps each query_id to its list of top relevant transactions.
using QueryResult = std::unordered_map<int, RelevanceScoreList>;

// TransactionOccurrencesList: list of (transaction_id, occurrence_count) pairs
// used when merging posting lists for items.
using TransactionOccurrencesList = std::vector<std::pair<int, int>>;


constexpr int NAIVE_INDEX = 0;
constexpr int INVERTED_INDEX = 1;

std::vector<std::vector<int>> load_item_sets_from_file(const std::string& item_sets_file);
RelevanceInvertedIndex build_relevance_inverted_index(const std::vector<std::vector<int>>& transactions, size_t total_transactions);
void write_inverted_file_occ(const std::string& filename, const InvertedIndex& index, const TRFWeights& trf_weights);

QueryResult run_inverted_method(const std::vector<std::vector<int>>& queries, const InvertedIndex& inverted_index, const TRFWeights& trf_weights, int query_number,int top_k);
RelevanceScoreList run_inverted_single(const std::vector<int>& query, const InvertedIndex& inverted_index, const TRFWeights& trf_weights, int top_k);
void print_query_result(const std::string& method_name, const RelevanceScoreList& result);

std::vector<QueryResult> run_method(const std::string& transactions_file, const std::string& queries_file, int query_number , int method_number, int top_k);


TransactionOccurrencesList union_two_transaction_occurrence_lists(const TransactionOccurrencesList& list_a, const TransactionOccurrencesList& list_b);
std::map<int, std::unordered_map<int, int>> build_detailed_map_from_union(const std::vector<int>& query_items,const InvertedIndex& index);



QueryResult run_naive_method(const std::vector<std::vector<int>>& queries,const std::vector<std::vector<int>>& transactions,const TRFWeights& trf_weights,int query_number,int top_k);
RelevanceScoreList run_naive_single(const std::vector<int>& query,const std::vector<std::vector<int>>& transactions,const TRFWeights& trf_weights,int top_k);





int main(const int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Invalid number of arguments" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <transactions.txt> <queries.txt> <qnum> <method> <k> " << std::endl;
        return 1;
    }

    std::vector<QueryResult> results =
        run_method(argv[1], argv[2], std::stoi(argv[3]), std::stoi(argv[4]), std::stoi(argv[5]));
}


/**
 * @brief Loads item sets from a file and parses them into a nested vector of integers.
 *
 * The function reads a file specified by the input parameter `item_sets_file`, where
 * each line in the file represents a transaction defined by a list of integers. Line by
 * line, the function parses the integers from the input file, ignoring non-numeric
 * characters (e.g., '[', ']', ',', and spaces), and constructs a vector representing
 * the items in that transaction. The resulting set of transactions is stored in a
 * nested vector and returned.
 *
 * @param item_sets_file The file path of the input file containing item sets as lines of integers.
 * @return A vector of transactions, where each transaction is itself represented as
 *         a vector of integers.
 *
 * @throws If the file cannot be opened, the function writes an error message to
 *         `std::cerr` and terminates the program with an exit code -1.
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
 * @brief Executes a specific method to process queries over transactions and calculates relevance scores.
 *
 * The function orchestrates the loading of transaction and query data from files, builds an
 * inverted index from the transactions, and processes the specified query or queries using
 * a chosen method (either naive or inverted index). The relevance scores for the queries are
 * calculated and stored in a structured result set. The choice of method is determined by
 * the parameter `method_number`, which can execute the naive method, inverted method, or both.
 *
 * @param transactions_file The file path for the input file containing the transactions.
 * @param queries_file The file path for the input file containing the queries.
 * @param query_number The index of the query to execute.
 * @param method_number Specifies the method to use:
 *                      0 for naive method,
 *                      1 for inverted index method,
 *                      -1 for both methods.
 * @param top_k The maximum number of results to return for each query (top k results).
 * @return A vector containing query results indexed by method (NAIVE_INDEX and INVERTED_INDEX).
 *
 * @throws If the specified method number is invalid, the function prints an error message
 *         to `std::cerr` and terminates the program with an exit code -1.
 * @note The function writes the inverted index data and term relevance factor (TRF) weights
 *       to an output file named "invfileocc.txt".
 */
std::vector<QueryResult> run_method(
    const std::string& transactions_file,
    const std::string& queries_file,
    const int query_number,
    const int method_number,
    const int top_k) {

    const std::vector<std::vector<int>> transactions = load_item_sets_from_file(transactions_file);
    const std::vector<std::vector<int>> queries = load_item_sets_from_file(queries_file);
    const size_t total_transactions = transactions.size();

    auto [relevance_inverted_index, trf_weights] =
        build_relevance_inverted_index(transactions, total_transactions);

    write_inverted_file_occ("invfileocc.txt", relevance_inverted_index, trf_weights);


    std::vector<QueryResult> results(2);

    switch (method_number) {
        case 0:
            results[NAIVE_INDEX] = run_naive_method(queries, transactions, trf_weights, query_number, top_k);
            break;

        case 1:
            results[INVERTED_INDEX] = run_inverted_method(queries, relevance_inverted_index, trf_weights, query_number, top_k);
            break;

        case -1:
            results[NAIVE_INDEX] = run_naive_method(queries, transactions, trf_weights, query_number, top_k);;
            results[INVERTED_INDEX] = run_inverted_method(queries, relevance_inverted_index, trf_weights, query_number, top_k);
            break;

        default:
            std::cerr << "Invalid method: " << method_number << std::endl;
            exit(-1);
            break;
    }
    return results;
}


//
//Inverted index build


/**
 * @brief Builds a relevance-based inverted index and computes TRF (Transaction Rarity Factor) weights.
 *
 * This function generates an inverted index from a collection of transactions, where each transaction is
 * a list of integers representing item IDs. Additionally, it computes TRF weights for items to quantify
 * their rarity across transactions. The inverted index associates each item ID with a posting list of
 * transaction IDs and the corresponding frequency of the item within those transactions. TRF weights are
 * calculated as the ratio of the total number of transactions to the number of transactions containing the item.
 *
 * @param transactions A nested vector representing the list of transactions, where each transaction is a vector of item IDs.
 * @param total_transactions The total number of transactions in the dataset.
 * @return A pair consisting of:
 *      - An inverted index that maps each item ID to a vector of pairs (transaction ID, item frequency).
 *      - A map of TRF weights, where each item ID is associated with its computed TRF weight.
 */
RelevanceInvertedIndex build_relevance_inverted_index(
    const std::vector<std::vector<int>>& transactions,
    const size_t total_transactions) {

    InvertedIndex inverted_index;
    TRFWeights trf_weights;

    // Stores how many distinct transactions each item appears in
    std::unordered_map<int, int> transactions_per_item;


    // Iterate over each transaction.
    for (int tid = 0; tid < transactions.size(); tid++) {
        // Count the frequency of each item within the current transaction
        std::unordered_map<int, int> item_frequency_in_transaction;
        for (int item : transactions[tid])
            item_frequency_in_transaction[item]++;

        // Update the inverted index and transaction count for each unique item
        for (const auto& [item_id, frequency] : item_frequency_in_transaction) {
            // Add (transaction_id, frequency) to the posting list of the item
            inverted_index[item_id].emplace_back(tid, frequency);
            // Increment the number of transactions in which this item appears
            transactions_per_item[item_id]++;
        }
    }


    // Compute TRF (Transaction Rarity Factor) weights for each item:
    // TRF = total_transactions / number_of_transactions_containing_the_item
    // Items that appear in fewer transactions get higher weight
    for (const auto& [item, count] : transactions_per_item) {
        trf_weights[item] = static_cast<double>(total_transactions) / count;
    }

    // Sort each posting list by transaction ID
    for (auto &postings: inverted_index | std::views::values) {
        std::ranges::sort(postings);
    }

    return {inverted_index, trf_weights};
}

/**
 * @brief Writes the contents of the inverted index and TRF weights to a file.
 *
 * This function takes an inverted index and its associated TRF weights and writes
 * them to the specified file in a human-readable format. Each item in the index is
 * written on a separate line along with its corresponding TRF weight and posting
 * list. The posting list consists of pairs of integers representing document IDs
 * and associated scores.
 *
 * @param filename The name of the file to write the inverted index and TRF weights to.
 * @param index The inverted index, where the key is an item ID, and the value is
 *              a vector of document ID and score pairs.
 * @param trf_weights A mapping of item IDs to their corresponding TRF weights.
 */
void write_inverted_file_occ(const std::string& filename, const InvertedIndex& index, const TRFWeights& trf_weights) {
    std::ofstream out(filename);
    if (!out) {
        std::cerr << "Failed to open " << filename << " for writing.\n";
        return;
    }


    out << std::fixed << std::setprecision(16);
    for (const auto& [item_id, postings] : index) {
        out << item_id << ": " << trf_weights.at(item_id) << ", [";
        for (size_t i = 0; i < postings.size(); i++) {
            out << "[" << postings[i].first << ", " << postings[i].second << "]";
            if (i != postings.size() - 1) out << ", ";
        }
        out << "]\n";
    }
    out.close();
}

/**
 * @brief Prints the result of a query processing method in a formatted manner.
 *
 * This function outputs the relevance scores of a query result associated with a specific method.
 * Each relevance score is presented as a pair of a floating-point score and an integer identifier.
 * The output is formatted with a fixed-point notation and a precision of 13 decimal places.
 *
 * @param method_name The name of the query processing method whose result is being printed.
 * @param result A list of relevance scores represented as a vector of pairs,
 *               where each pair contains a double (score) and an int (identifier).
 */
void print_query_result(const std::string& method_name, const RelevanceScoreList& result) {
    std::cout << std::fixed << std::setprecision(13);
    std::cout << method_name << " result:\n[";
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "[" << result[i].first << ", " << result[i].second << "]";
        if (i != result.size() - 1) std::cout << ", ";
    }
    std::cout << "]\n";
}



//
//Naive method


/**
 * @brief Executes the naive method for query matching against a collection of transactions.
 *
 * This function computes the relevance scores for queries against a set of transactions
 * using the naive method, which iterates through all transactions for each query.
 * The matching process can be performed for all queries or for a specific query based
 * on the parameter `query_number`. The results include the top-k most relevant transactions
 * for each query.
 *
 * @param queries A vector of queries, where each query is represented as a vector of integers.
 * @param transactions A vector of transactions, where each transaction is represented as a vector of integers.
 * @param trf_weights A map of transaction relevance factor (TRF) weights used to adjust the relevance scores of items.
 * @param query_number The index of the query to process. If set to -1, all queries are processed. Otherwise, only the query at `query_number` is processed.
 * @param top_k The number of top relevant transactions to return per query. If set to 0 or a negative value, all results are returned.
 * @return A map where the key is the query index, and the value is a vector of pairs representing the relevance score
 *         and transaction index for the top-k transactions.
 */
QueryResult run_naive_method(
    const std::vector<std::vector<int>>& queries,
    const std::vector<std::vector<int>>& transactions,
    const TRFWeights& trf_weights,
    const int query_number,
    const int top_k) {

    QueryResult results;


    const auto start = std::chrono::high_resolution_clock::now();


    if (query_number == -1) {
        for (int i = 0; i < queries.size(); ++i) {
            results[i] = run_naive_single(queries[i], transactions, trf_weights, top_k);
        }
    } else {
        results[query_number] = run_naive_single(queries[query_number], transactions, trf_weights, top_k);
        print_query_result("Naive Method", results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Naive Method computation time = "
              << std::chrono::duration<double>(end - start).count()
              << " seconds\n";

    return results;
}

/**
 * @brief Computes relevance scores for a given query against a set of transactions.
 *
 * This function evaluates how relevant each transaction is for a given query based
 * on term frequency-relevance (TRF) weights. For each transaction, it counts how
 * many times query items occur in the transaction. Then, it computes a relevance
 * score by summing the occurrence counts of matching items weighted by the provided
 * TRF weights. Transactions with a relevance score of zero are ignored. The results
 * are sorted in descending order of relevance scores, and the size of the results
 * is constrained to `top_k` if a positive value is provided.
 *
 * @param query The query, represented as a vector of integers, where each integer
 *              corresponds to an item being searched for in the transactions.
 * @param transactions A collection of transactions, each represented as a vector
 *                     of integers.
 * @param trf_weights A map containing TRF weights for individual items. Keys are
 *                    item identifiers, and values are the associated weights.
 * @param top_k The maximum number of results to return. If set to a positive
 *              number, only the top `top_k` results are included in the output.
 *              If set to 0 or a negative number, all matching transactions are
 *              included.
 * @return A vector of pairs, where each pair consists of a relevance score (double)
 *         and a transaction ID (int) sorted in descending order of relevance scores.
 */
RelevanceScoreList run_naive_single(
    const std::vector<int>& query,
    const std::vector<std::vector<int>>& transactions,
    const TRFWeights& trf_weights,
    const int top_k){

    RelevanceScoreList scores;

    // For each transaction:
    for (int tid = 0; tid < transactions.size(); ++tid) {

        // Count the number of times each item in the query appears in this transaction.
        std::unordered_map<int, int> occ;
        for (int item : transactions[tid]) {
            if (std::ranges::find(query, item) != query.end()) {
                occ[item]++;
            }
        }


       // Compute the relevance score for this transaction.
        double relevance = 0.0;
        for (const auto& [item, count] : occ) {
            if (auto it = trf_weights.find(item); it != trf_weights.end())
                relevance += count * it->second;
        }


        // Filter out transactions with a relevance score of 0.0. So no relationship between query and transaction.
        if (relevance > 0.0)
            scores.emplace_back(relevance, tid);
    }


    // Sort the list of relevance scores in descending order.
    std::ranges::sort(scores, std::greater<>());


    //Limit the number of results to top_k.
    if (top_k > 0 && scores.size() > static_cast<size_t>(top_k))
        scores.resize(top_k);

    return scores;
}


/**
 * @brief Executes the inverted index search method to process queries and retrieve results.
 *
 * This function processes a set of queries against a given relevance inverted index
 * and term-relevance-frequency (TRF) weights. The function supports running all queries
 * or a specific query based on the `query_number` argument. For each query, the relevant
 * results are computed using the `run_inverted_single` method. If a specific query is
 * requested, the function optionally prints the result for that query. Computation time
 * is measured and printed to the console for performance diagnostics.
 *
 * @param queries A vector of queries, where each query is itself represented as a vector of integers.
 * @param inverted_index A relevance inverted index mapping item IDs to a list of document IDs
 *                       with relevance scores.
 * @param trf_weights A map storing term-relevance-frequency weights for each item.
 * @param query_number The index of the query to process. If set to -1, all queries are processed.
 * @param top_k The maximum number of top results to retrieve for each query.
 * @return A QueryResult object, which maps each query index to a list of relevance scores,
 *         where each score is paired with a document ID.
 */
QueryResult run_inverted_method(
    const std::vector<std::vector<int>>& queries,
    const InvertedIndex& inverted_index,
    const TRFWeights& trf_weights,
    const int query_number,
    const int top_k) {

    QueryResult results;
    const auto start = std::chrono::high_resolution_clock::now();

    if (query_number == -1) {
        for (int i = 0; i < queries.size(); i++) {
            results[i] = run_inverted_single(queries[i], inverted_index, trf_weights, top_k);
        }
    } else {
        results[query_number] = run_inverted_single(queries[query_number], inverted_index, trf_weights, top_k);
        print_query_result("Inverted File", results[query_number]);
    }

    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> duration = end - start;
    std::cout << "Inverted File computation time = " << duration.count() << " seconds\n";

    return results;
}


/**
 * @brief Merges two transaction occurrence lists into a single list, combining occurrences of matching transactions.
 *
 * This function processes two sorted lists of transaction occurrences, represented as pairs of transaction IDs and
 * their respective occurrence counts. It merges the two lists, ensuring the resulting list is also sorted by transaction IDs.
 * If a transaction ID appears in both input lists, their occurrence counts are summed and included as a single entry.
 * Transaction IDs appearing only in one of the lists are included in the result without modification.
 *
 * @param list_a The first sorted list of transaction occurrences, where each element is a pair of a transaction ID and its count.
 * @param list_b The second sorted list of transaction occurrences, where each element is a pair of a transaction ID and its count.
 * @return A sorted list of transaction occurrences, where matching transaction IDs from the input lists are combined
 *         by summing their occurrence counts.
 */
TransactionOccurrencesList union_two_transaction_occurrence_lists(
    const TransactionOccurrencesList& list_a,
    const TransactionOccurrencesList& list_b) {
    TransactionOccurrencesList result;
    size_t p1 = 0;
    size_t p2 = 0;

    while (p1 < list_a.size() && p2 < list_b.size()) {
        if (list_a[p1].first < list_b[p2].first) {
            result.push_back(list_a[p1]);
            p1++;
        } else if (list_a[p1].first > list_b[p2].first) {
            result.push_back(list_b[p2]);
            p2++;
        } else {
            result.emplace_back(list_a[p1].first, list_a[p1].second + list_b[p2].second);
            p1++;
            p2++;
        }
    }

    while (p1 < list_a.size()) result.push_back(list_a[p1++]);
    while (p2 < list_b.size()) result.push_back(list_b[p2++]);

    return result;
}

/**
 * @brief Constructs a detailed map of transaction IDs to item occurrences based on query items.
 *
 * This function processes a query consisting of item IDs and retrieves their associated data
 * from the provided inverted index. For each item in the query, it identifies the transactions
 * in which the item occurs, combines these transactions, and constructs a nested map where
 * transaction IDs map to another map of item IDs with their occurrence counts within the transaction.
 *
 * @param query_items A vector of integers representing the query items to retrieve data for.
 * @param index The inverted index mapping item IDs to a vector of pairs, where each pair
 *              consists of a transaction ID and the occurrence count of the item within that transaction.
 * @return A map where each key is a transaction ID and each value is an unordered map of item IDs
 *         with their respective occurrence counts in that transaction.
 */
std::map<int, std::unordered_map<int, int>> build_detailed_map_from_union(
    const std::vector<int>& query_items,
    const InvertedIndex& index) {

    std::map<int, std::unordered_map<int, int>> result;

    for (int item : query_items) {
        auto it = index.find(item);
        if (it == index.end()) continue;

        for (const auto& [tid, occ] : it->second) {
            result[tid][item] += occ;
        }
    }
    return result;
}

/**
 * @brief Computes relevance scores for a single query using an inverted index.
 *
 * The function processes a query by retrieving occurrences of query items from the
 * provided inverted index, accumulating weights based on their occurrences and
 * predefined weights. The resulting relevance scores are sorted in descending order,
 * and optionally trimmed to only include the top_k results.
 *
 * @param query A vector of integers representing the query items.
 * @param inverted_index The inverted index mapping item IDs to transaction occurrences
 *        and their occurrence counts.
 * @param trf_weights A mapping of item IDs to their associated weights.
 * @param top_k The maximum number of most relevant results to return. If top_k <= 0,
 *        all results are returned.
 * @return A vector of pairs where each pair consists of a relevance score (double)
 *         and the corresponding transaction ID (int), sorted in descending order by
 *         relevance score.
 */
RelevanceScoreList run_inverted_single(const std::vector<int>& query,
    const InvertedIndex& inverted_index,
    const TRFWeights& trf_weights,
    const int top_k) {

    // For all query items, retrieve their occurrences lists from the inverted index,
    // and union merge them into a map: transaction_id -> (item_id -> occurrence count).
    auto merged = build_detailed_map_from_union(query, inverted_index);

    RelevanceScoreList scores;


    // For each item in this transaction that also appears in the query,
    // multiply its occurrence count by its weight and add to the relevance score
    for (const auto& [tid, item_map] : merged) {
        double relevance = 0.0;

        for (const auto& [item_id, occ] : item_map) {
            auto it = trf_weights.find(item_id);
            if (it != trf_weights.end())
                relevance += occ * it->second;
        }

        //Keep only relevance ones.
        if (relevance > 0.0)
            scores.emplace_back(relevance, tid);
    }

    // Sort transactions by decreasing relevance score
    std::ranges::sort(scores, std::greater<>());

    // Keep only top_k results if required
    if (top_k > 0 && scores.size() > static_cast<size_t>(top_k))
        scores.resize(top_k);

    return scores;
}