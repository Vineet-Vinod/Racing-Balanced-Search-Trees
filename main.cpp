#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <functional>
#include <memory>

// --- C++ Tree Headers ---
#include "B_Trees/btree.h"
#include "RB_Trees/rbtree.h"
#include "Splay_Trees/splay_tree.h"
#include "AVL_Trees/avl_tree.h"

// --- Configuration ---
const int NUM_ELEMENTS = 100'000;
const int B_TREE_ORDER = 16; // A reasonable order for in-memory B-Trees

// =================================================================================================
// 1. UNIFIED INTERFACE & WRAPPERS
//
// We define an abstract base class to ensure all trees can be benchmarked with the same code.
// Then, we create wrapper classes for each tree implementation to adhere to this interface.
// =================================================================================================

class IBenchmarkableTree
{
public:
    virtual ~IBenchmarkableTree() = default;
    virtual const std::string &name() const = 0;
    virtual bool add(int value) = 0;
    virtual bool find(int value) = 0;
    virtual bool remove(int value) = 0;
    virtual void clear() = 0;
};

/**
 * @brief A generic template wrapper for the C++ tree implementations.
 * Uses `if constexpr` to handle classes that might not have a public `clear()` method (like BTree).
 */
template <typename TreeType>
class CppTreeWrapper : public IBenchmarkableTree
{
private:
    TreeType tree;
    std::string tree_name;

    // SFINAE trait to detect if a `clear()` method exists.
    // AI Generated - idk what's happening
    template <typename T, typename = void>
    struct has_clear_method : std::false_type
    {
    };
    template <typename T>
    struct has_clear_method<T, std::void_t<decltype(std::declval<T &>().clear())>> : std::true_type
    {
    };

public:
    explicit CppTreeWrapper(std::string name) : tree_name(std::move(name)) {}
    ~CppTreeWrapper() override = default;

    const std::string &name() const override { return tree_name; }
    bool add(int value) override { return tree.add(value); }
    bool find(int value) override { return tree.find(value); }
    bool remove(int value) override { return tree.remove(value); }

    void clear() override
    {
        if constexpr (has_clear_method<TreeType>::value)
        {
            tree.clear();
        }
        else
        {
            // For classes without `clear()` (like BTree), we destruct and reconstruct.
            tree = TreeType();
        }
    }
};

// =================================================================================================
// 2. BENCHMARKING FRAMEWORK
// =================================================================================================

struct BenchmarkResults
{
    std::chrono::duration<double, std::milli> insert_time;
    std::chrono::duration<double, std::milli> find_hit_time;
    std::chrono::duration<double, std::milli> find_miss_time;
    std::chrono::duration<double, std::milli> remove_time;
};

void run_benchmark(IBenchmarkableTree &tree, const std::vector<int> &insert_data, const std::vector<int> &search_miss_data, BenchmarkResults &results)
{

    tree.clear();

    // 1. Benchmark Insertion
    auto start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.add(val);
    }
    auto end = std::chrono::high_resolution_clock::now();
    results.insert_time = end - start;

    // 2. Benchmark Successful Search (Find Hit)
    start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.find(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.find_hit_time = end - start;

    // 3. Benchmark Unsuccessful Search (Find Miss)
    start = std::chrono::high_resolution_clock::now();
    for (int val : search_miss_data)
    {
        tree.find(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.find_miss_time = end - start;

    // 4. Benchmark Deletion
    start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.remove(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.remove_time = end - start;
}


void run_benchmark(const std::vector<int> &insert_data, const std::vector<int> &search_miss_data, BenchmarkResults &results)
{
    std::set<int> tree;

    // 1. Benchmark Insertion
    auto start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.insert(val);
    }
    auto end = std::chrono::high_resolution_clock::now();
    results.insert_time = end - start;

    // 2. Benchmark Successful Search (Find Hit)
    start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.find(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.find_hit_time = end - start;

    // 3. Benchmark Unsuccessful Search (Find Miss)
    start = std::chrono::high_resolution_clock::now();
    for (int val : search_miss_data)
    {
        tree.find(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.find_miss_time = end - start;

    // 4. Benchmark Deletion
    start = std::chrono::high_resolution_clock::now();
    for (int val : insert_data)
    {
        tree.erase(val);
    }
    end = std::chrono::high_resolution_clock::now();
    results.remove_time = end - start;
}


void print_results(const std::string &tree_name, const BenchmarkResults &results)
{
    std::cout << "| " << std::left << std::setw(15) << tree_name
              << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(2) << results.insert_time.count() << " ms "
              << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(2) << results.find_hit_time.count() << " ms "
              << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(2) << results.find_miss_time.count() << " ms "
              << "| " << std::right << std::setw(10) << std::fixed << std::setprecision(2) << results.remove_time.count() << " ms |"
              << std::endl;
}

// =================================================================================================
// 3. MAIN EXECUTION
// =================================================================================================

int main()
{
    // --- Data Preparation ---
    std::vector<int> random_data(NUM_ELEMENTS);
    std::vector<int> sorted_data(NUM_ELEMENTS);
    std::vector<int> search_miss_data(NUM_ELEMENTS);

    std::mt19937 gen(1337); // Fixed seed for reproducibility
    std::uniform_int_distribution<> distrib(0, NUM_ELEMENTS * 5);

    for (int i = 0; i < NUM_ELEMENTS; ++i)
    {
        sorted_data[i] = i;
        random_data[i] = i;
        search_miss_data[i] = distrib(gen); // Values likely not in the trees
    }
    std::shuffle(random_data.begin(), random_data.end(), gen);
    std::vector<int> random_delete_data = random_data;
    std::shuffle(random_delete_data.begin(), random_delete_data.end(), gen);

    // --- Tree Instantiation ---
    std::vector<std::unique_ptr<IBenchmarkableTree>> trees;
    trees.push_back(std::make_unique<CppTreeWrapper<AVLTree<int>>>("AVL Tree"));
    trees.push_back(std::make_unique<CppTreeWrapper<RBTree<int>>>("RB Tree"));
    trees.push_back(std::make_unique<CppTreeWrapper<SplayTree<int>>>("Splay Tree"));
    trees.push_back(std::make_unique<CppTreeWrapper<BTree<int, B_TREE_ORDER>>>(
        "B-Tree (N=" + std::to_string(B_TREE_ORDER) + ")"));

    // --- Run Benchmarks ---
    auto run_test_set = [&](const std::string &test_name, const std::vector<int> &data_set)
    {
        std::cout << "\n--- Benchmarking on " << test_name << " (" << NUM_ELEMENTS << " elements) ---\n";
        std::cout << "-----------------------------------------------------------------------------\n";
        std::cout << "| Tree Type       |      Insert |   Find (Hit) |  Find (Miss) |       Remove |\n";
        std::cout << "-----------------------------------------------------------------------------\n";

        for (const auto &tree : trees)
        {
            BenchmarkResults results;
            run_benchmark(*tree, data_set, search_miss_data, results);
            print_results(tree->name(), results);
        }

        BenchmarkResults results;
        run_benchmark(data_set, search_miss_data, results);
        print_results("std::set", results);
        std::cout << "-----------------------------------------------------------------------------\n";
    };

    run_test_set("Randomly Ordered Data", random_data);
    run_test_set("Sequentially Ordered Data", sorted_data);

    return 0;
}
