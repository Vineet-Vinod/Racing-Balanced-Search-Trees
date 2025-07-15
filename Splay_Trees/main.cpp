#include <iostream>
#include <cassert>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include "splay_tree.h"

using namespace std;

class SplayTreeTester
{
private:
    template <typename T, typename Compare>
    static bool is_splay_tree_valid(const SplayTree<T, Compare> &tree)
    {
        return is_bst_valid<T, Compare>(tree.node) && is_parent_pointers_valid<T, Compare>(tree.node, nullptr);
    }

    template <typename T, typename Compare>
    static bool is_bst_valid(const typename SplayTree<T, Compare>::TreeNode *node)
    {
        if (node == nullptr)
            return true;

        if (node->children[LEFT] && node->children[LEFT]->val > node->val)
            return false;
        if (node->children[RIGHT] && node->children[RIGHT]->val < node->val)
            return false;

        return is_bst_valid<T, Compare>(node->children[LEFT]) && is_bst_valid<T, Compare>(node->children[RIGHT]);
    }

    template <typename T, typename Compare>
    static bool is_parent_pointers_valid(const typename SplayTree<T, Compare>::TreeNode *node, const typename SplayTree<T, Compare>::TreeNode *parent)
    {
        if (node == nullptr)
            return true;

        if (node->parent != parent)
            return false;

        return is_parent_pointers_valid<T, Compare>(node->children[LEFT], node) && is_parent_pointers_valid<T, Compare>(node->children[RIGHT], node);
    }

public:
    static void test_all()
    {
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
        cout << "All SplayTree tests passed!" << endl;
    }

    static void test_constructor_and_destructor()
    {
        SplayTree<int> st1;
        assert(st1.node == nullptr);
        assert(is_splay_tree_valid(st1));

        SplayTree<int> st2(10);
        assert(st2.node != nullptr);
        assert(st2.node->val == 10);
        assert(is_splay_tree_valid(st2));

        cout << "test_constructor_and_destructor passed." << endl;
    }

    static void test_copy_constructor()
    {
        SplayTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);

        SplayTree<int> copied = original;
        assert(copied.node != nullptr);
        assert(copied.node != original.node); // Should be a deep copy
        assert(copied.node->val == original.node->val);
        assert(is_splay_tree_valid(copied));
        assert(is_splay_tree_valid(original));

        // Modify copied and check original is unchanged
        copied.add(60);
        assert(original.find(60) == false);
        assert(copied.find(60) == true);

        cout << "test_copy_constructor passed." << endl;
    }

    static void test_copy_assignment()
    {
        SplayTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);

        SplayTree<int> assigned;
        assigned.add(100);
        assigned = original;

        assert(assigned.node != nullptr);
        assert(assigned.node != original.node); // Should be a deep copy
        assert(assigned.node->val == original.node->val);
        assert(is_splay_tree_valid(assigned));
        assert(is_splay_tree_valid(original));

        // Modify assigned and check original is unchanged
        assigned.add(60);
        assert(original.find(60) == false);
        assert(assigned.find(60) == true);

        cout << "test_copy_assignment passed." << endl;
    }

    static void test_move_constructor()
    {
        SplayTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);
        typename SplayTree<int>::TreeNode *original_node_ptr = original.node;

        SplayTree<int> moved = std::move(original);
        assert(moved.node == original_node_ptr); // Should be a shallow copy
        assert(original.node == nullptr);        // Original should be null
        assert(is_splay_tree_valid(moved));

        // Ensure moved tree is functional
        assert(moved.find(50));
        assert(moved.find(30));
        assert(moved.find(70));
        moved.add(60);
        assert(moved.find(60));

        cout << "test_move_constructor passed." << endl;
    }

    static void test_move_assignment()
    {
        SplayTree<int> original;
        original.add(50);
        original.add(30);
        original.add(70);
        typename SplayTree<int>::TreeNode *original_node_ptr = original.node;

        SplayTree<int> assigned;
        assigned.add(100);
        assigned = std::move(original);

        assert(assigned.node == original_node_ptr); // Should be a shallow copy
        assert(original.node == nullptr);           // Original should be null
        assert(is_splay_tree_valid(assigned));

        // Ensure assigned tree is functional
        assert(assigned.find(50));
        assert(assigned.find(30));
        assert(assigned.find(70));
        assigned.add(60);
        assert(assigned.find(60));

        cout << "test_move_assignment passed." << endl;
    }

    static void test_add_and_find()
    {
        SplayTree<int> st;
        assert(st.add(10));
        assert(st.node->val == 10);
        assert(is_splay_tree_valid(st));

        assert(st.add(5));
        assert(st.node->val == 5); // 5 should be the new root after splay
        assert(is_splay_tree_valid(st));

        assert(st.add(15));
        assert(st.node->val == 15); // 15 should be the new root
        assert(is_splay_tree_valid(st));

        assert(!st.add(10)); // 10 already exists
        assert(is_splay_tree_valid(st));

        assert(st.find(5));
        assert(st.node->val == 5); // 5 should be the root after find
        assert(is_splay_tree_valid(st));

        assert(st.find(15));
        assert(st.node->val == 15); // 15 should be the root after find
        assert(is_splay_tree_valid(st));

        assert(!st.find(20)); // 20 does not exist
        assert(is_splay_tree_valid(st));

        cout << "test_add_and_find passed." << endl;
    }

    static void test_remove()
    {
        SplayTree<int> st;
        st.add(50);
        st.add(30);
        st.add(70);
        st.add(20);
        st.add(40);
        st.add(60);
        st.add(80);
        assert(is_splay_tree_valid(st));

        // Remove a leaf (20)
        assert(st.remove(20));
        assert(!st.find(20));
        assert(is_splay_tree_valid(st));

        // Remove a node with one child (40)
        st.add(35); // Make 40 have one child (35)
        assert(st.remove(40));
        assert(!st.find(40));
        assert(is_splay_tree_valid(st));

        // Remove a node with two children (30)
        assert(st.remove(30));
        assert(!st.find(30));
        assert(is_splay_tree_valid(st));

        // Remove root (50)
        assert(st.remove(50));
        assert(!st.find(50));
        assert(is_splay_tree_valid(st));

        // Remove non-existent element
        assert(!st.remove(100));
        assert(is_splay_tree_valid(st));

        // Remove last element
        st.clear();
        st.add(10);
        assert(st.remove(10));
        assert(st.node == nullptr);
        assert(is_splay_tree_valid(st));

        cout << "test_remove passed." << endl;
    }

    static void test_clear()
    {
        SplayTree<int> st;
        st.add(10);
        st.add(5);
        st.add(15);
        assert(st.node != nullptr);
        assert(is_splay_tree_valid(st));

        st.clear();
        assert(st.node == nullptr);
        assert(is_splay_tree_valid(st));

        // Clear an empty tree
        st.clear();
        assert(st.node == nullptr);
        assert(is_splay_tree_valid(st));

        cout << "test_clear passed." << endl;
    }

    static void test_large_data_set()
    {
        SplayTree<int> st;
        std::set<int> std_set;
        const int num_elements = 10000;

        // Add elements
        for (int i = 0; i < num_elements; ++i)
        {
            assert(st.add(i));
            std_set.insert(i);
            assert(is_splay_tree_valid(st));
        }

        // Find elements
        for (int i = 0; i < num_elements; ++i)
        {
            assert(st.find(i));
            assert(is_splay_tree_valid(st));
        }

        // Remove elements
        for (int i = 0; i < num_elements; i += 2) // Remove half the elements
        {
            assert(st.remove(i));
            std_set.erase(i);
            assert(is_splay_tree_valid(st));
        }

        // Verify remaining elements
        for (int i = 0; i < num_elements; ++i)
        {
            if (std_set.count(i))
            {
                assert(st.find(i));
            }
            else
            {
                assert(!st.find(i));
            }
            assert(is_splay_tree_valid(st));
        }

        cout << "test_large_data_set passed." << endl;
    }

    static void test_random_operations()
    {
        SplayTree<int> st;
        std::set<int> std_set;
        const int num_operations = 10000;
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> dist(0, num_operations * 2);

        for (int i = 0; i < num_operations; ++i)
        {
            int val = dist(rng);
            int op = dist(rng) % 3; // 0: add, 1: find, 2: remove

            if (op == 0) // Add
            {
                bool added_st = st.add(val);
                bool added_set = std_set.insert(val).second;
                assert(added_st == added_set);
            }
            else if (op == 1) // Find
            {
                bool found_st = st.find(val);
                bool found_set = std_set.count(val);
                assert(found_st == found_set);
            }

            else // Remove
            {
                bool removed_st = st.remove(val);
                bool removed_set = std_set.erase(val);
                assert(removed_st == removed_set);
            }
            assert(is_splay_tree_valid(st));
        }

        // Verify all remaining elements
        for (int val : std_set)
        {
            assert(st.find(val));
            assert(is_splay_tree_valid(st));
        }

        cout << "test_random_operations passed." << endl;
    }

    static void test_performance_comparison()
    {
        cout << "\n--- Performance Comparison (SplayTree vs std::set) ---" << endl;
        const int num_elements = 50000;
        vector<int> data;
        data.reserve(num_elements);

        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<int> dist(0, num_elements * 5);
        for (int i = 0; i < num_elements; ++i)
        {
            data.push_back(dist(rng));
        }

        auto time_function = [](const std::string &name, auto func)
        {
            auto start = std::chrono::high_resolution_clock::now();
            func();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> duration = end - start;
            std::cout << name << ": " << duration.count() << " ms" << std::endl;
        };

        // SplayTree performance
        SplayTree<int> splay_tree_add;
        time_function("SplayTree Add", [&]()
                      {
            for (int x : data) {
                splay_tree_add.add(x);
            } });

        SplayTree<int> splay_tree_find = splay_tree_add; // Copy for find test
        time_function("SplayTree Find", [&]()
                      {
            for (int x : data) {
                splay_tree_find.find(x);
            } });

        SplayTree<int> splay_tree_remove = splay_tree_add; // Copy for remove test
        time_function("SplayTree Remove", [&]()
                      {
            for (int x : data) {
                splay_tree_remove.remove(x);
            } });

        // std::set performance
        std::set<int> std_set_add;
        time_function("std::set Add", [&]()
                      {
            for (int x : data) {
                std_set_add.insert(x);
            } });

        std::set<int> std_set_find = std_set_add; // Copy for find test
        time_function("std::set Find", [&]()
                      {
            for (int x : data) {
                std_set_find.count(x);
            } });

        std::set<int> std_set_remove = std_set_add; // Copy for remove test
        time_function("std::set Remove", [&]()
                      {
            for (int x : data) {
                std_set_remove.erase(x);
            } });

        std::cout << "--- Performance Comparison End ---\n"
                  << std::endl;
    }

};

int main()
{
    SplayTreeTester::test_all();
    return 0;
}
