#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <unistd.h>

struct Node {
    std::string str;
    FILE* fp;
};

struct Cmp {
    bool reverse;

    bool operator()(const Node& a, const Node& b) const {
        return reverse ? a.str < b.str : a.str > b.str;
    }
};

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

bool separate(std::vector<FILE*>& files, std::vector<std::string>& str_lst, bool reverse) {
    if (str_lst.empty()) return true;

    auto cmp = [reverse](const std::string& a, const std::string& b) {
        return reverse ? a > b : a < b;
    };
    std::sort(str_lst.begin(), str_lst.end(), cmp);

    FILE* fp = tmpfile();
    if (!fp) return false;

    for (const auto& str : str_lst) {
        if (fputs(str.c_str(), fp) == EOF || fputc('\n', fp) == EOF) {
            fclose(fp);
            return false;
        }
    }

    rewind(fp);
    files.push_back(fp);

    str_lst.clear();
    return true;
}

bool merge(std::vector<FILE*>& files, bool reverse) {
    std::priority_queue<Node, std::vector<Node>, Cmp> pq(Cmp{reverse});

    for (FILE* fp : files) {
        std::string str;
        if (fgetline(fp, str))
            pq.push(Node{str, fp});
        else
            fclose(fp);
    }

    while (!pq.empty()) {
        Node cur = pq.top();
        pq.pop();
        std::cout << cur.str << '\n';

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
    while ((opt = getopt(argc, (char *const *)argv, "r")) != -1) {
        switch (opt) {
            case 'r':
                reverse = true;
                break;
            default:
                return 1;
        }
    }
    if (optind != argc) return 1;

    const size_t MEM_LIMIT = 64ULL * 1024 * 1024;
    std::vector<FILE*> files;
    std::vector<std::string> str_lst;
    std::string str;
    size_t len = 0;

    while (std::getline(std::cin, str)) {
        len += sizeof(std::string) + str.size();
        str_lst.push_back(str);

        if (len >= MEM_LIMIT) {
            if (!separate(files, str_lst, reverse)) return 1;
            len = 0;
        }
    }
    if (!separate(files, str_lst, reverse)) return 1;

    if (!merge(files, reverse)) return 1;
}
