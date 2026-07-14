#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <unistd.h>

/*
 * Node represents one element in the k-way merge process.
 * - str: the current line from a run (temporary sorted file)
 * - fp:  the file pointer to the run this line comes from
 */
struct Node {
    std::string str;
    FILE* fp;
};

/*
 * Comparator for the priority queue.
 * Determines the ordering of nodes in the heap.
 * 
 * Note:
 * - priority_queue is a max-heap by default
 * - We invert the comparison to simulate a min-heap when needed
 * - reverse == false → ascending order
 * - reverse == true  → descending order
 */
struct Cmp {
    bool reverse;

    bool operator()(const Node& a, const Node& b) const {
        return reverse ? a.str < b.str : a.str > b.str;
    }
};

/*
 * Reads one line from a FILE* into a std::string.
 * 
 * - Uses fgets with a buffer and appends until a full line is read
 * - Supports arbitrarily long lines (via repeated reads)
 * - Removes trailing '\n'
 * 
 * Returns:
 * - true  if a line was successfully read
 * - false if EOF and no data left
 */
bool fgetline(FILE* fp, std::string& out) {
    out.clear();

    char buf[1024];
    while (fgets(buf, sizeof(buf), fp)) {
        out += buf;

        if (!out.empty() && out.back() == '\n') {
            out.pop_back();
            return true;
        }
    }

    return !out.empty();
}

/*
 * Flushes the current chunk (str_lst) into a temporary sorted file (run).
 * 
 * Steps:
 * 1. Sort the current chunk in memory
 * 2. Write all lines to a temporary file (tmpfile)
 * 3. Rewind the file pointer to the beginning for future reading
 * 4. Store the FILE* in 'files'
 * 5. Clear the in-memory chunk
 * 
 * Returns:
 * - true  on success
 * - false on failure (e.g., tmpfile or write error)
 */
bool separate(std::vector<FILE*>& files, std::vector<std::string>& str_lst, bool reverse) {
    if (str_lst.empty()) return true;

    // Define comparator for sorting chunk
    auto cmp = [reverse](const std::string& a, const std::string& b) {
        return reverse ? a > b : a < b;
    };
    std::sort(str_lst.begin(), str_lst.end(), cmp);

    // Create temporary file for this run
    FILE* fp = tmpfile();
    if (!fp) return false;

    // Write sorted lines into file
    for (const auto& str : str_lst) {
        if (fputs(str.c_str(), fp) == EOF || fputc('\n', fp) == EOF) {
            fclose(fp);
            return false;
        }
    }

    // Reset file pointer to beginning for reading phase
    rewind(fp);
    files.push_back(fp);

    // Clear chunk for next batch
    str_lst.clear();
    return true;
}

/*
 * Performs k-way merge over all temporary run files.
 * 
 * Steps:
 * 1. Initialize a priority queue with the first line from each file
 * 2. Repeatedly:
 *    - Extract the smallest (or largest) line
 *    - Output it
 *    - Read the next line from the same file and push it into the queue
 * 3. Close files when exhausted
 * 
 * Returns:
 * - true  if output succeeded
 * - false if output stream failed
 */
bool merge(std::vector<FILE*>& files, bool reverse) {
    std::priority_queue<Node, std::vector<Node>, Cmp> pq(Cmp{reverse});

    // Initialize heap with first line from each run
    for (FILE* fp : files) {
        std::string str;
        if (fgetline(fp, str))
            pq.push(Node{str, fp});
        else
            fclose(fp);
    }

    // Merge process
    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();

        // Output current smallest/largest line
        std::cout << cur.str << '\n';

        // Read next line from same file
        std::string str;
        if (fgetline(cur.fp, str))
            pq.push(Node{str, cur.fp});
        else
            fclose(cur.fp);
    }

    return !std::cout.fail();
}

int main(int argc, const char** argv) {
    bool reverse = false;
    int opt;

    // Parse command line option -r (reverse sort)
    while ((opt = getopt(argc, (char *const *)argv, "r")) != -1) {
        switch (opt) {
            case 'r':
                reverse = true;
                break;
            default:
                return 1;
        }
    }

    // Reject unexpected extra arguments
    if (optind != argc) return 1;

    // Approximate memory limit for in-memory chunk
    const size_t MEM_LIMIT = 64ULL * 1024 * 1024;

    std::vector<FILE*> files;          // list of temporary run files
    std::vector<std::string> str_lst;  // current chunk
    std::string str;
    size_t len = 0;                   // rough memory usage counter

    // Read input line by line from stdin
    while (std::getline(std::cin, str)) {
        // Rough estimate of memory usage
        len += sizeof(std::string) + str.size();

        str_lst.push_back(str);

        // If memory limit reached, flush chunk to disk
        if (len >= MEM_LIMIT) {
            if (!separate(files, str_lst, reverse)) return 1;
            len = 0;
        }
    }

    // Flush remaining data
    if (!separate(files, str_lst, reverse)) return 1;

    // Merge all runs
    if (!merge(files, reverse)) return 1;
}
