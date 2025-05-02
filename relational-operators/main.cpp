/**
 * This program performs various relational algebra operations (merge join, union, intersection,
 * difference, group-by with aggregation) on TSV files. The files are assumed to be sorted on the key
 * (first column). Outputs are written to new TSV files.
 *
 * Usage:
 *   ./a.out <R_sorted.tsv> <S_sorted.tsv> <R.tsv>
 *
 * The operations performed:
 *   - Merge Join: Joins R and S on the key.
 *   - Union: Produces the union of R and S without duplicates.
 *   - Intersection: Finds common rows between R and S.
 *   - Difference: Computes R - S.
 *   - Group-By: Groups R by key and sums the integer values.
 *
 * The Record struct represents a row with:
 *   - column_1: key (std::string)
 *   - column_2: value (int)
 */



#include <algorithm>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>


struct Record {
    std::string column_1;
    int column_2;
};


void merge_join(const std::string& r_file_name , const std::string& s_file_name, const std::string& join_file_name);
void union_(const std::string& r_file_name , const std::string& s_file_name, const std::string& union_file_name);
void intersection(const std::string& r_file_name , const std::string& s_file_name, const std::string& intersection_file_name);
void R_difference_S(const std::string& r_file_name , const std::string& s_file_name, const std::string& difference_file_name);
void groupBy_with_aggregation(const std::string& r_file_name , const std::string& groupBy_with_sum_file);
std::vector<Record> merge_sort(std::vector<Record>::const_iterator begin, std::vector<Record>::const_iterator end);
std::vector<Record> merge_with_aggregation(const std::vector<Record>& left, const std::vector<Record>& right);



int main(int argc, char *argv[]) {
    if(argc != 4) {
        std::cerr << "Error: Three TSV file paths must be provided as input.\n";
        std::cerr << "Usage: ./a.out <R_sorted_path> <S_sorted_path> <R_path>" << std::endl;
        return 1;
    }

    const std::string r_sorted = argv[1];
    const std::string s_sorted = argv[2];
    const std::string r = argv[3];

    merge_join(r_sorted, s_sorted, "RjoinS.tsv");
    union_(r_sorted, s_sorted, "RunionS.tsv");
    intersection(r_sorted, s_sorted, "RintersectionS.tsv");
    R_difference_S(r_sorted, s_sorted, "RdifferenceS.tsv");
    groupBy_with_aggregation(r, "Rgroupby.tsv");

    return 0;
}




/**
 * Performs a merge join operation between two sorted TSV files on their first column (key).
 * Creates all possible combinations of rows when multiple rows share the same key.
 * @param r_file_name Path to the first sorted TSV file (R)
 * @param s_file_name Path to the second sorted TSV file (S)
 * @param join_file_name Path where the join result will be saved
 */
void merge_join(const std::string& r_file_name , const std::string& s_file_name, const std::string& join_file_name) {
    std::ifstream r_sorted(r_file_name);
    std::ifstream s_sorted(s_file_name);
    std::ofstream r_join_s(join_file_name);

    if(!r_sorted.is_open() || !s_sorted.is_open() || !r_join_s.is_open()) {
        std::cerr << "Failed to open one or more files!" << std::endl;
        return;
    }

    std::string r_line, s_line;
    std::string r_key, s_key;

    std::vector<std::string> buffer_s;
    size_t buffer_max_size_reached = 0;


    //Read first records
    bool r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
    bool s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));


    //while r_file and s_file not empty
    while(r_read_success && s_read_success) {
        r_key = r_line.substr(0, r_line.find('\t'));
        s_key = s_line.substr(0, s_line.find('\t'));

        std::string temp;

        if(r_key == s_key) {
            do {
                //save s columns to buffer (expect the key for memory efficiency , we already have it saved)
                buffer_s.push_back(s_line.substr(s_line.find('\t') + 1));

                //advance s pointer.
                s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
                if(s_read_success)
                    s_key = s_line.substr(0, s_line.find('\t'));

            }//if next s record exists and s_key match again with r_key repeat.
            while(s_read_success && s_key == r_key);


            temp = r_key;
            do {
                std::string r_column = r_line.substr(r_line.find('\t') + 1);
                for(const auto &s_columns: buffer_s) {
                    r_join_s << temp << "\t"
                            << r_column << "\t"
                            << s_columns
                            << std::endl;
                }

                r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
                if(r_read_success)
                    temp = r_line.substr(0, r_line.find('\t'));
                else
                    break;
            }// For each row in R that has again the same key, generate all possible combinations with the buffered rows from S
            while(temp == r_key);


            //Keeping record of buffer max size. Clearing the buffer also after each use.
            buffer_max_size_reached = std::max(buffer_max_size_reached, buffer_s.size());
            buffer_s.clear();
        }else if(r_key < s_key){
            //advance r pointer.
            r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
        }
        else {
            //advance s pointer.
            s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
        }
    }

    std::cout << "Merge Join Completed." << std::endl;
    std::cout << "Buffer max size reached: " << buffer_max_size_reached << std::endl;
    std::cout << "--------" << std::endl;
}

/**
 * Performs a set union operation between two sorted TSV files.
 * Eliminates duplicates in the result.
 * @param r_file_name Path to the first sorted TSV file (R)
 * @param s_file_name Path to the second sorted TSV file (S)
 * @param union_file_name Path where the union result will be saved
 */
void union_(const std::string& r_file_name , const std::string& s_file_name, const std::string& union_file_name) {
    std::ifstream r_sorted(r_file_name);
    std::ifstream s_sorted(s_file_name);
    std::ofstream r_union_s(union_file_name);


    if(!r_sorted.is_open() || !s_sorted.is_open() || !r_union_s.is_open()) {
        std::cerr << "Failed to open one or more files!" << std::endl;
        return;
    }

    std::string r_line, s_line;
    std::string last_written_record;

    bool r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
    bool s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));


    //while r_file has records still or s_file has records still
    while(r_read_success || s_read_success) {
        std::string record_to_write;

        //if both files have records
        if(r_read_success && s_read_success) {

            if(r_line < s_line) {
                //add r record and advance r.
                record_to_write = r_line;
                r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
            }else if(r_line > s_line) {
                //add s record and advance s.
                record_to_write = s_line;
                s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
            }else {
                //add r record (it's the same with s) and advance both.
                record_to_write = r_line;
                r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
                s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
            }

        }
        else if(r_read_success) {
            //s is finished at this point, so we add all the r records left.
            record_to_write = r_line;
            r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
        }
        else {
            //r is finished at this point, so we add all the s records left.
            record_to_write = s_line;
            s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
        }

        // Ensure no duplicate records are added. Duplicate records appear consecutively, so comparing with the last written record is enough.
        if(record_to_write != last_written_record) {
            r_union_s << record_to_write << std::endl;
            last_written_record = record_to_write;
        }
    }
    std::cout << "Union Completed." << std::endl;
    std::cout << "--------" << std::endl;
}

/**
 * Finds common records between two sorted TSV files.
 * Eliminates duplicates in the result.
 * @param r_file_name Path to the first sorted TSV file (R)
 * @param s_file_name Path to the second sorted TSV file (S)
 * @param intersection_file_name Path where the intersection result will be saved
 */
void intersection(const std::string& r_file_name , const std::string& s_file_name, const std::string& intersection_file_name) {
    std::ifstream r_sorted(r_file_name);
    std::ifstream s_sorted(s_file_name);
    std::ofstream r_intersection_s(intersection_file_name);

    if(!r_sorted.is_open() || !s_sorted.is_open() || !r_intersection_s.is_open()) {
        std::cerr << "Failed to open one or more files!" << std::endl;
        return;
    }


    std::string r_line, s_line;
    std::string last_written_record;

    bool r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
    bool s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));


    //while r not empty and s not empty
    while(r_read_success && s_read_success) {

        if(r_line < s_line) {
            //Not a common record. Advance r
            r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
        }else if(r_line > s_line) {
            //not a common record. advance s
            s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
        }else {
            // Ensure no duplicate records are added.
            // Duplicate records appear consecutively, so comparing with the last written record is enough.
            if(r_line != last_written_record) {
                r_intersection_s << r_line << std::endl;
                last_written_record = r_line;
            }

            //after adding the common record, advance both.
            r_read_success = static_cast<bool>(std::getline(r_sorted, r_line));
            s_read_success = static_cast<bool>(std::getline(s_sorted, s_line));
        }
    }

    std::cout << "Intersection Completed." << std::endl;
    std::cout << "--------" << std::endl;
}

/**
 * Computes the set difference R - S between two sorted TSV files.
 * Returns records that exist in R but not in S.
 * @param r_file_name Path to the first sorted TSV file (R)
 * @param s_file_name Path to the second sorted TSV file (S)
 * @param difference_file_name Path where the difference result will be saved
 */
void R_difference_S(const std::string& r_file_name , const std::string& s_file_name, const std::string& difference_file_name) {
    std::ifstream r_sorted(r_file_name);
    std::ifstream s_sorted(s_file_name);
    std::ofstream r_difference_s(difference_file_name);

    if(!r_sorted.is_open() || !s_sorted.is_open() || !r_difference_s.is_open()) {
        std::cerr << "Failed to open one or more files!" << std::endl;
        return;
    }

    std::string r_line, s_line;
    std::string last_written_record;

    bool r_read_succeed = static_cast<bool>(std::getline(r_sorted, r_line));
    bool s_read_succeed = static_cast<bool>(std::getline(s_sorted, s_line));

    //while r is not empty
    while(r_read_succeed) {
        //if s is empty or s record != r record
        if(!s_read_succeed || r_line < s_line) {
            // Add r record.
            // Ensure no duplicate records are added.
            // Duplicate records appear consecutively, so comparing with the last written record is enough.
            if(r_line != last_written_record) {
                r_difference_s << r_line << std::endl;
                last_written_record = r_line;
            }
            //advance r pointer.
            r_read_succeed = static_cast<bool>(std::getline(r_sorted, r_line));
        }else if(r_line > s_line) {
            //Advance s
            s_read_succeed = static_cast<bool>(std::getline(s_sorted, s_line));
        }else {
            //Advance both.
            r_read_succeed = static_cast<bool>(std::getline(r_sorted, r_line));
            s_read_succeed = static_cast<bool>(std::getline(s_sorted, s_line));
        }
    }

    std::cout << "Difference Completed." << std::endl;
    std::cout << "--------" << std::endl;
}

/**
 * Groups records by the first column and sums the second column values.
 * Uses merge sort for sorting and aggregation.
 * @param r_file_name Path to the input TSV file
 * @param groupBy_with_sum_file Path where the grouped and aggregated result will be saved
 */
void groupBy_with_aggregation(const std::string& r_file_name , const std::string& groupBy_with_sum_file) {
    std::ifstream r_file(r_file_name);
    std::ofstream groupBy_with_sum(groupBy_with_sum_file);

    if(!r_file.is_open() || !groupBy_with_sum.is_open()) {
        std::cerr << "Failed to open one or more files!" << std::endl;
        return;
    }

    std::vector<Record> records;

    //Load records to memory
    std::string line;
    while(std::getline(r_file, line)) {
        std::string key = line.substr(0, line.find('\t'));
        int value = std::stoi(line.substr(line.find('\t') + 1));
        records.push_back({key, value});
    }

    std::vector<Record> sorted_records_with_sum = merge_sort(records.begin(), records.end());

    //Write records to file
    for(const auto&[column_1, column_2]: sorted_records_with_sum) {
        groupBy_with_sum << column_1 << "\t" << column_2 << std::endl;
    }

    std::cout << "Group By with column 2 sum Completed." << std::endl;
    std::cout << "--------" << std::endl;;
}

/**
 * Implements merge sort algorithm for Records.
 * Used by groupBy_with_aggregation to sort records by key.
 * @param begin Iterator to the beginning of the range to sort
 * @param end Iterator to the end of the range to sort
 * @return Sorted vector of Records
 */
std::vector<Record> merge_sort(const std::vector<Record>::const_iterator begin, const std::vector<Record>::const_iterator end) {
    if (std::distance(begin, end) <= 1)
        return std::vector(begin, end);

    const auto middle = begin + std::distance(begin, end) / 2;
    const std::vector<Record> left = merge_sort(begin, middle);
    const std::vector<Record> right = merge_sort(middle, end);

    return merge_with_aggregation(left, right);
}

/**
 * Merges two sorted vectors of Records while aggregating values for identical keys.
 * Used by merge_sort to combine sorted sub-arrays.
 * @param left First sorted vector of Records
 * @param right Second sorted vector of Records
 * @return Merged and aggregated vector of Records
 */
std::vector<Record> merge_with_aggregation(const std::vector<Record>& left, const std::vector<Record>& right) {
    std::vector<Record> merged;
    size_t i = 0, j = 0;

    //while both lists have records
    while(i < left.size() && j < right.size()) {
        //if left record key < right record key
        if(left[i].column_1 < right[j].column_1) {
            merged.push_back(left[i]);
            i++;
        }//if left record key > right record key
        else if(left[i].column_1 > right[j].column_1) {
            merged.push_back(right[j]);
            j++;
        }//if left record key == right record key we do the sum.
        else{
            Record record = {left[i].column_1, left[i].column_2 + right[j].column_2};
            merged.push_back(record);
            i++;
            j++;
        }
    }

    //if the left list has records
    if(i < left.size()) {
        merged.insert(merged.end(), left.begin() + i, left.end());
    }
    //if the right list has records
    if(j < right.size()) {
        merged.insert(merged.end(), right.begin() + j, right.end());
    }

    return merged;
}
