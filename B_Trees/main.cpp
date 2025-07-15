#include "btree.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <cassert>
#include <set>
#include <numeric>

class BTreeTester
{
public:
    template <typename T, std::size_t N>
    static void basicOperationsTest()
    {
        BTree<T, N> tree;

        std::vector<T> data = {13, 10, 7, 6, 17, 15};
        for (const auto &val : data)
        {
            assert(tree.add(val));
            assert(tree.find(val));
        }

        for (const auto &val : data)
        {
            assert(tree.find(val));
        }

        for (const auto &val : data)
        {
            assert(tree.remove(val));
            assert(!tree.find(val));
        }

        std::cout << "Passed Basic" << std::endl;
    }

    template <typename T, std::size_t N>
    static void duplicateAddTest()
    {
        BTree<T, N> tree;
        assert(tree.add(42));
        assert(!tree.add(42)); // Should not allow duplicates
        assert(tree.find(42));

        std::cout << "Passed Duplicate" << std::endl;
    }

    template <typename T, std::size_t N>
    static void copyMoveTest()
    {
        BTree<T, N> original;
        std::vector<T> values = {1, 2, 3, 4, 5};
        for (const T &val : values)
            original.add(val);

        BTree<T, N> copied = original; // Copy constructor
        for (const T &val : values)
            assert(copied.find(val));

        BTree<T, N> moved = std::move(original); // Move constructor
        for (const T &val : values)
            assert(moved.find(val));

        BTree<T, N> assigned;
        assigned = moved; // Copy assignment
        for (const T &val : values)
            assert(assigned.find(val));

        BTree<T, N> moveAssigned;
        moveAssigned = std::move(assigned); // Move assignment
        for (const T &val : values)
            assert(moveAssigned.find(val));

        std::cout << "Passed Copy - Move" << std::endl;
    }

    template <typename T, std::size_t N>
    static void largeVolumeTest(size_t volume = 10'000)
    {
        BTree<T, N> tree;
        std::vector<T> values(volume);
        std::iota(values.begin(), values.end(), 1);
        std::shuffle(values.begin(), values.end(), std::mt19937{std::random_device{}()});

        for (const T &val : values)
            assert(tree.add(val));

        std::shuffle(values.begin(), values.end(), std::mt19937{std::random_device{}()});
        for (const T &val : values)
            assert(tree.find(val));

        for (const T &val : values)
        {
            assert(tree.find(val));
            assert(tree.remove(val));
        }

        std::cout << "Passed Large" << std::endl;
    }

    template <typename T, std::size_t N>
    static void randomTest(size_t samples = 1000)
    {
        BTree<T, N> tree;
        std::set<T> model;
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<T> dist(1, 1000000);

        for (int i = 0; i < samples; ++i)
        {
            T val = dist(gen);
            bool added = tree.add(val);
            if (model.find(val) == model.end())
            {
                assert(added);
                model.insert(val);
            }
            else
            {
                assert(!added);
            }
        }

        for (T val : model)
        {
            assert(tree.find(val));
            assert(tree.remove(val));
            assert(!tree.find(val));
        }

        std::cout << "Passed Random" << std::endl;
    }

    template <typename T, std::size_t N>
    static void structureTest()
    {
        BTree<T, N> tree;
        std::vector<T> values = {10, 20, 5, 6, 12, 30, 7, 17};
        for (const auto &val : values)
            tree.add(val);

        validateNode<T, N>(tree.root);

        std::cout << "Passed structure" << std::endl;
    }

private:
    template <typename T, std::size_t N>
    static void validateNode(typename BTree<T, N>::Node *node)
    {
        if (!node)
            return;

        assert(node->num_keys <= static_cast<int>(2 * N - 1));
        if (!node->leaf)
        {
            for (int i = 0; i <= node->num_keys; ++i)
            {
                assert(node->children[i] != nullptr);
                validateNode<T, N>(node->children[i]);
            }
        }
        for (int i = 1; i < node->num_keys; ++i)
        {
            assert(node->keys[i - 1] < node->keys[i]);
        }
    }
};

int main(int argc, char const *argv[])
{
    std::cout << "Running BTree tests..." << std::endl;

    #ifndef TIME
    BTreeTester::basicOperationsTest<int, 3>();
    BTreeTester::duplicateAddTest<int, 3>();
    BTreeTester::copyMoveTest<int, 3>();
    BTreeTester::largeVolumeTest<int, 8>();
    BTreeTester::randomTest<int, 4>();
    BTreeTester::structureTest<int, 3>();
    #endif
    #ifdef TIME
    BTreeTester::randomTest<int, 20>(1'000'000);
    BTreeTester::largeVolumeTest<int, 20>(1'000'000);
    #endif

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}

/*
template <typename T, std::size_t N, typename Compare = std::less<T>>
requires (N > 1)
class BTree
{
public:
    BTree() : root(nullptr) {}

    ~BTree() {}

    BTree(const BTree &other) {}

    BTree &operator=(const BTree &other) {}

    BTree(BTree &&other) noexcept : root(other.root), less_than(std::move(other.less_than)) {}

    BTree &operator=(BTree &&other) noexcept {}

    bool find(const T &val) {}

    bool add(const T &val) {}

    bool remove(const T &val) {}

    private: // Attributes
    struct Node
    {
        T keys[2 * N - 1];
        Node *children[2 * N];
        int num_keys;
        bool leaf;

        Node() : num_keys(0), leaf(false) {}
    };

    Node *root;
    Compare less_than;

private: // Friend tester class
    friend class BTreeTester;
};
*/
