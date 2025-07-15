#include <iostream>
#include <cassert>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include <iomanip>
#include "avl_tree.h"

using namespace std;

class AVLTreeTester {
private:
    // --- VALIDATION LOGIC ---

    // The main validation function. Checks all properties of a valid AVL tree.
    template <typename T, typename Compare>
    static bool is_avl_tree_valid(const AVLTree<T, Compare>& tree) {
        // We use the friend class access to get the root node.
        const typename AVLTree<T, Compare>::TreeNode* root = tree.node;

        if (!is_bst_valid<T, Compare>(root, nullptr, nullptr)) {
            cerr << "\n--- Validation Failed: Not a valid BST. ---\n";
            return false;
        }

        bool is_balanced_and_heights_correct = true;
        check_height_and_balance<T, Compare>(root, is_balanced_and_heights_correct);
        if (!is_balanced_and_heights_correct) {
            cerr << "\n--- Validation Failed: Heights or balance factors are incorrect. ---\n";
            return false;
        }

        return true;
    }

    // 1. Checks if the tree adheres to the Binary Search Tree property recursively.
    template <typename T, typename Compare>
    static bool is_bst_valid(const typename AVLTree<T, Compare>::TreeNode* node, const T* min_val, const T* max_val) {
        if (node == nullptr) {
            return true;
        }

        Compare less;
        // Check if current node's value is within the valid range [min_val, max_val]
        if ((min_val && (less(node->val, *min_val) || !less(*min_val, node->val) /* node->val == *min_val */)) ||
            (max_val && (less(*max_val, node->val) || !less(node->val, *max_val) /* node->val == *max_val */))) {
            return false;
        }
        
        // Recursively check left and right subtrees with updated bounds.
        return is_bst_valid<T, Compare>(node->left, min_val, &node->val) && is_bst_valid<T, Compare>(node->right, &node->val, max_val);
    }

    // 2. Recursively checks height correctness and the AVL balance property.
    // Returns the true height of the subtree.
    template <typename T, typename Compare>
    static int check_height_and_balance(const typename AVLTree<T, Compare>::TreeNode* node, bool& is_valid) {
        if (!is_valid) return 0; // Stop early if an error was found elsewhere
        if (node == nullptr) return 0;

        int left_height = check_height_and_balance<T, Compare>(node->left, is_valid);
        int right_height = check_height_and_balance<T, Compare>(node->right, is_valid);

        // Check the AVL balance factor property
        if (abs(left_height - right_height) > 1) {
            is_valid = false;
        }
        
        // Check if the node's stored height is correct
        if (node->height != 1 + max(left_height, right_height)) {
            is_valid = false;
        }
        
        return 1 + max(left_height, right_height);
    }

public:
    static void test_all() {
        test_constructor_and_destructor();
        test_copy_constructor();
        test_copy_assignment();
        test_move_constructor();
        test_move_assignment();
        test_add_and_find();
        test_remove();
        test_clear();
        test_large_data_set();
        test_random_operations();
        test_performance_comparison();
        cout << "\nAll AVLTree tests passed successfully!" << endl;
    }

    static void test_constructor_and_destructor() {
        cout << "Testing Constructors & Destructor... ";
        AVLTree<int> tree1;
        assert(is_avl_tree_valid(tree1));

        AVLTree<int> tree2(10);
        assert(is_avl_tree_valid(tree2));
        assert(tree2.find(10));
        cout << "PASSED" << endl;
    }

    static void test_copy_constructor() {
        cout << "Testing Copy Constructor... ";
        AVLTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);

        AVLTree<int> copied = original;
        assert(is_avl_tree_valid(copied) && is_avl_tree_valid(original));
        assert(copied.find(50) && copied.find(30) && copied.find(70));
        
        // Ensure it's a deep copy
        assert(original.node != copied.node);
        if (original.node && copied.node) {
             assert(original.node->left != copied.node->left);
        }

        // Modify copied and check original is unchanged
        copied.add(60);
        assert(original.find(60) == false);
        assert(copied.find(60) == true);
        cout << "PASSED" << endl;
    }

    static void test_copy_assignment() {
        cout << "Testing Copy Assignment... ";
        AVLTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);

        AVLTree<int> assigned;
        assigned.add(100);
        assigned = original;

        assert(is_avl_tree_valid(assigned) && is_avl_tree_valid(original));
        assert(assigned.find(50) && assigned.find(30) && assigned.find(70));
        assert(assigned.find(100) == false);

        // Ensure it's a deep copy
        assert(original.node != assigned.node);

        // Self-assignment
        assigned = assigned;
        assert(is_avl_tree_valid(assigned));

        cout << "PASSED" << endl;
    }

    static void test_move_constructor() {
        cout << "Testing Move Constructor... ";
        AVLTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);
        auto* original_node_ptr = original.node;

        AVLTree<int> moved = std::move(original);
        assert(moved.node == original_node_ptr); // Should be a shallow copy
        assert(original.node == nullptr);       // Original should be null
        assert(is_avl_tree_valid(moved));
        assert(moved.find(50) && moved.find(30) && moved.find(70));
        cout << "PASSED" << endl;
    }

    static void test_move_assignment() {
        cout << "Testing Move Assignment... ";
        AVLTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);
        auto* original_node_ptr = original.node;

        AVLTree<int> assigned;
        assigned.add(100);
        assigned = std::move(original);

        assert(assigned.node == original_node_ptr); // Should be a shallow copy
        assert(original.node == nullptr);          // Original should be null
        assert(is_avl_tree_valid(assigned));
        assert(assigned.find(50) && assigned.find(30) && assigned.find(70));
        assert(assigned.find(100) == false);
        cout << "PASSED" << endl;
    }

    static void test_add_and_find() {
        cout << "Testing Add & Find... ";
        AVLTree<int> tree;
        assert(tree.add(10));
        assert(is_avl_tree_valid(tree));
        assert(tree.add(5));
        assert(is_avl_tree_valid(tree));
        assert(tree.add(15)); // This should trigger a rotation
        assert(is_avl_tree_valid(tree));
        assert(!tree.add(10)); // Duplicate

        assert(tree.find(5));
        assert(tree.find(15));
        assert(tree.find(10));
        assert(!tree.find(20));
        cout << "PASSED" << endl;
    }

    static void test_remove() {
        cout << "Testing Remove... ";
        AVLTree<int> tree;
        tree.add(50); tree.add(30); tree.add(70); tree.add(20);
        tree.add(40); tree.add(60); tree.add(80); tree.add(35);
        assert(is_avl_tree_valid(tree));

        // Remove a leaf (20)
        assert(tree.remove(20));
        assert(!tree.find(20));
        assert(is_avl_tree_valid(tree));

        // Remove a node with one child (40, its child is 35)
        assert(tree.remove(40));
        assert(!tree.find(40));
        assert(is_avl_tree_valid(tree));

        // Remove a node with two children (30)
        assert(tree.remove(30));
        assert(!tree.find(30));
        assert(is_avl_tree_valid(tree));

        // Remove root (50)
        assert(tree.remove(50));
        assert(!tree.find(50));
        assert(is_avl_tree_valid(tree));

        // Remove non-existent element
        assert(!tree.remove(100));
        assert(is_avl_tree_valid(tree));

        // Remove until empty
        assert(tree.remove(80)); assert(tree.remove(70));
        assert(tree.remove(60)); assert(tree.remove(35));
        assert(tree.node == nullptr);
        assert(is_avl_tree_valid(tree));
        cout << "PASSED" << endl;
    }

    static void test_clear() {
        cout << "Testing Clear... ";
        AVLTree<int> tree;
        tree.add(10); tree.add(5); tree.add(15);
        assert(tree.node != nullptr);

        tree.clear();
        assert(tree.node == nullptr);
        assert(is_avl_tree_valid(tree));

        // Clear an already empty tree
        tree.clear();
        assert(tree.node == nullptr);
        cout << "PASSED" << endl;
    }

    static void test_large_data_set() {
        cout << "Testing with large data set... ";
        AVLTree<int> tree;
        set<int> std_set;
        const int num_elements = 10000;

        // Add elements sequentially (stress test for balancing)
        for (int i = 0; i < num_elements; ++i) {
            assert(tree.add(i));
            std_set.insert(i);
            assert(is_avl_tree_valid(tree));
        }

        // Find all elements
        for (int i = 0; i < num_elements; ++i) {
            assert(tree.find(i));
        }

        // Remove half the elements
        for (int i = 0; i < num_elements; i += 2) {
            assert(tree.remove(i));
            std_set.erase(i);
            assert(is_avl_tree_valid(tree));
        }

        // Verify remaining elements
        for (int i = 0; i < num_elements; ++i) {
            assert(tree.find(i) == (std_set.count(i) > 0));
        }
        cout << "PASSED" << endl;
    }

    static void test_random_operations() {
        cout << "Testing with random operations... ";
        AVLTree<int> tree;
        set<int> std_set;
        const int num_operations = 20000;
        mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
        uniform_int_distribution<int> dist_val(0, num_operations);
        uniform_int_distribution<int> dist_op(0, 2);

        for (int i = 0; i < num_operations; ++i) {
            int val = dist_val(rng);
            int op = dist_op(rng);

            if (op == 0) { // Add
                assert(tree.add(val) == std_set.insert(val).second);
            } else if (op == 1) { // Find
                assert(tree.find(val) == (std_set.count(val) > 0));
            } else { // Remove
                assert(tree.remove(val) == (std_set.erase(val) > 0));
            }
            // Validating after every operation is slow but thorough.
            // For a faster test, you could validate every N operations.
            if (i % 1000 == 0) {
                assert(is_avl_tree_valid(tree));
            }
        }
        assert(is_avl_tree_valid(tree));
        cout << "PASSED" << endl;
    }
    
    static void test_performance_comparison() {
        cout << "\n--- Performance Comparison (AVLTree vs std::set) ---" << endl;
        const int num_elements = 100000;
        vector<int> data;
        data.reserve(num_elements);

        mt19937 rng(1337); // Fixed seed for reproducibility
        uniform_int_distribution<int> dist(0, num_elements * 5);
        for (int i = 0; i < num_elements; ++i) {
            data.push_back(dist(rng));
        }

        auto time_function = [](const string &name, auto func) {
            auto start = chrono::high_resolution_clock::now();
            func();
            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double, milli> duration = end - start;
            cout << left << setw(18) << name << ": " << fixed << setprecision(2) << duration.count() << " ms" << endl;
        };

        // AVLTree performance
        {
            AVLTree<int> avl_tree;
            time_function("AVLTree Add", [&]() { for (int x : data) avl_tree.add(x); });
            time_function("AVLTree Find", [&]() { for (int x : data) avl_tree.find(x); });
            time_function("AVLTree Remove", [&]() { for (int x : data) avl_tree.remove(x); });
        }

        // std::set performance
        {
            set<int> std_set;
            time_function("std::set Add", [&]() { for (int x : data) std_set.insert(x); });
            time_function("std::set Find", [&]() { for (int x : data) std_set.count(x); });
            time_function("std::set Remove", [&]() { for (int x : data) std_set.erase(x); });
        }
        
        cout << "--- Performance Comparison End ---" << endl;
    }
};

int main() {
    AVLTreeTester::test_all();
    return 0;
}