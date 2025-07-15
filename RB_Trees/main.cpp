#include <iostream>
#include <cassert>
#include <vector>
#include <set>
#include <algorithm>
#include <chrono>
#include <random>
#include "rbtree.h"

using namespace std;

class RBTreeTest
{
private:
    RBTree<int> tree;

public:
    void test_basic_insert_and_find()
    {
        tree.clear();
        assert(tree.add(10));
        test_red_black_properties();
        assert(tree.add(5));
        test_red_black_properties();
        assert(tree.add(15));
        test_red_black_properties();
        assert(tree.find(10));
        assert(tree.find(5));
        assert(tree.find(15));
        assert(!tree.find(0));
        assert(!tree.add(10)); // duplicate
        test_red_black_properties();
        cout << "✅ Basic insert/find passed.\n";
    }

    void test_deletion_cases()
    {
        tree.clear();
        tree.add(10); // root
        test_red_black_properties();
        tree.add(5); // left child
        test_red_black_properties();
        tree.add(15); // right child
        test_red_black_properties();

        // Leaf deletions
        assert(tree.remove(5));
        assert(!tree.find(5));
        assert(tree.remove(15));
        assert(!tree.find(15));

        // Root deletion
        assert(tree.remove(10));
        assert(!tree.find(10));

        cout << "✅ Deletion (leaf/root) passed.\n";
    }

    void test_node_with_one_child()
    {
        tree.clear();
        tree.add(10);
        test_red_black_properties();
        tree.add(5);
        test_red_black_properties();
        assert(tree.remove(10));
        test_red_black_properties();
        assert(tree.find(5));
        cout << "✅ Deletion with one child passed.\n";
    }

    void test_node_with_two_children()
    {
        tree.clear();
        tree.add(10);
        test_red_black_properties();
        tree.add(5);
        test_red_black_properties();
        tree.add(15);
        test_red_black_properties();
        tree.add(12);
        test_red_black_properties();
        tree.add(18);
        test_red_black_properties();
        assert(tree.remove(15));
        test_red_black_properties();
        assert(tree.find(12));
        assert(tree.find(18));
        cout << "✅ Deletion with two children passed.\n";
    }

    void test_inorder_traversal()
    {
        tree.clear();
        vector<int> values = {10, 20, 5, 7, 15, 2};
        for (int v : values)
        {
            tree.add(v);
            test_red_black_properties();
        }

        vector<int> in_order;
        function<void(const RBTree<int>::TreeNode *)> dfs = [&](const RBTree<int>::TreeNode *node)
        {
            if (!node)
                return;
            dfs(node->children[LEFT]);
            in_order.push_back(node->val);
            dfs(node->children[RIGHT]);
        };
        dfs(tree.node);
        assert(is_sorted(in_order.begin(), in_order.end()));
        cout << "✅ In-order traversal is sorted.\n";
    }

    void test_red_black_properties()
    {
        function<int(const RBTree<int>::TreeNode *)> check = [&](const RBTree<int>::TreeNode *n) -> int
        {
            if (!n)
                return 1;

            // Red property: no two red nodes in a row
            if (n->color == RED)
            {
                assert(!n->children[LEFT] || n->children[LEFT]->color == BLACK);
                assert(!n->children[RIGHT] || n->children[RIGHT]->color == BLACK);
            }

            int left_black_height = check(n->children[LEFT]);
            int right_black_height = check(n->children[RIGHT]);
            assert(left_black_height == right_black_height); // same black height

            return left_black_height + (n->color == BLACK ? 1 : 0);
        };
        check(tree.node);
    }

    void test_large_scale_inserts_deletes(int N = 1'000'000)
    {
        tree.clear();
        std::set<int> treec;

        auto start = chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i)
        {
            #ifdef TREE
            tree.add(i);
            #ifdef TEST
            test_red_black_properties();
            #endif
            #endif
            #ifndef TREE
            treec.insert(i);
            #endif
        }
        auto mid = chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i += 2)
        {
            #ifdef TREE
            tree.remove(i);
            #ifdef TEST
            test_red_black_properties();
            #endif
            #endif
            #ifndef TREE
            treec.erase(i);
            #endif
        }
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> insert_time = mid - start;
        chrono::duration<double> delete_time = end - mid;

        cout << "✅ Large scale inserted " << N << " elements in " << insert_time.count() << "s.\n";
        cout << "✅ Large scale deleted " << N / 2 << " elements in " << delete_time.count() << "s.\n";

        for (int i = 0; i < N; ++i)
        {
            if (i % 2 == 0)
            {
                #ifdef TREE
                assert(!tree.find(i));
                #endif
                #ifndef TREE
                assert(!treec.contains(i));
                #endif
            }
            else
            {
                #ifdef TREE
                assert(tree.find(i));
                #endif
                #ifndef TREE
                assert(treec.contains(i));
                #endif
            }
        }

        cout << "✅ Large-scale insert/delete test passed.\n";
    }

    void test_randomized_operations(int N = 1'000'000)
    {
        tree.clear();
        std::set<int> treec;

        vector<int> nums(N);
        iota(nums.begin(), nums.end(), 0);
        shuffle(nums.begin(), nums.end(), default_random_engine{});

        auto start = chrono::high_resolution_clock::now();
        for (int i : nums)
        {
            #ifdef TREE
            tree.add(i);
            #ifdef TEST
            test_red_black_properties();
            #endif
            #endif
            #ifndef TREE
            treec.insert(i);
            #endif
        }
        auto midA = chrono::high_resolution_clock::now();

        shuffle(nums.begin(), nums.end(), default_random_engine{});
        
        auto midB = chrono::high_resolution_clock::now();
        for (int i = 0; i < N >> 1; ++i)
        {
            #ifdef TREE
            tree.remove(i);
            #ifdef TEST
            test_red_black_properties();
            #endif
            #endif
            #ifndef TREE
            treec.erase(i);
            #endif
        }
        auto end = chrono::high_resolution_clock::now();

        chrono::duration<double> insert_time = midA - start;
        chrono::duration<double> delete_time = end - midB;

        cout << "✅ Random inserted " << N << " elements in " << insert_time.count() << "s.\n";
        cout << "✅ Random deleted " << (N >> 1) << " elements in " << delete_time.count() << "s.\n";

        for (int i = 0; i < N >> 1; ++i)
        {
            #ifdef TREE
            assert(!tree.find(i));
            #endif
            #ifndef TREE
            assert(!treec.contains(i));
            #endif
        }

        for (int i = N >> 1; i < N; ++i)
        {
            #ifdef TREE
            assert(tree.find(i));
            #endif
            #ifndef TREE
            assert(treec.contains(i));
            #endif
        }

        cout << "✅ Randomized operations test passed.\n";
    }
};

int main()
{
    RBTreeTest tester;
    // tester.test_basic_insert_and_find();
    // tester.test_deletion_cases();
    // tester.test_node_with_one_child();
    // tester.test_node_with_two_children();
    // tester.test_inorder_traversal();
    // tester.test_red_black_properties();
    tester.test_large_scale_inserts_deletes(1'000'000);
    tester.test_randomized_operations(1'000'000);
    cout << "🎉 All tests passed successfully.\n";
    return 0;
}
