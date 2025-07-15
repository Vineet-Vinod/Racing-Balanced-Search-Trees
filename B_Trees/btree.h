#ifndef __BTREE_H__
#define __BTREE_H__

#include <utility>
#include <functional>
#include <stack>
#include <cstdint>

template <typename T, std::size_t N, typename Compare = std::less<T>>
requires (N > 1)
class BTree
{
public:
    // Constructor
    BTree() : root(nullptr) {}

    // Destructor
    ~BTree()
    {
        clear(root);
    }

    // Copy
    BTree(const BTree &other)
    {
        auto copy = [](auto copy, Node *root) -> Node *
        {
            if (root == nullptr)
                return nullptr;

            Node *ret = new Node;
            for (int i = 0; i < 2 * N - 1; i++)
                ret->keys[i] = root->keys[i];
            ret->leaf = root->leaf;
            ret->num_keys = root->num_keys;
            for (int i = 0; i < 2 * N; i++)
                ret->children[i] = copy(copy, root->children[i]);

            return ret;
        };

        less_than = other.less_than;
        root = copy(copy, other.root);
    }

    BTree &operator=(const BTree &other)
    {
        if (this == &other)
            return *this;

        BTree new_tree(other);
        std::swap(root, new_tree.root);
        std::swap(less_than, new_tree.less_than);
        return *this;
    }

    // Move
    BTree(BTree &&other) noexcept : root(other.root), less_than(std::move(other.less_than))
    {
        other.root = nullptr;
    }

    BTree &operator=(BTree &&other) noexcept
    {
        if (this == &other)
            return *this;

        clear(root);
        root = other.root;
        less_than = std::move(less_than);
        other.root = nullptr;
        return *this;
    }

    // Search
    bool find(const T &val)
    {
        Node *node = root;

        while (node != nullptr)
        {
            int idx = bin_search(node, val);
            if (idx < 0)
                node = node->children[0];
            else if (node->keys[idx] == val)
                return true;
            else
                node = node->children[idx + 1];
        }

        return false;
    }

    // Insert
    bool add(const T &val)
    {
        // No nodes
        if (root == nullptr)
        {
            Node *node = new Node;
            node->leaf = true;
            node->keys[0] = val;
            node->num_keys++;
            root = node;
            return true;
        }

        // Full root - create new root
        if (root->num_keys == 2 * N - 1)
        {
            Node *adj_node = new Node, *new_root = new Node, *curr = root;

            // Initialize new root
            new_root->leaf = false;
            new_root->num_keys++;
            new_root->keys[0] = std::move(curr->keys[N - 1]);
            new_root->children[0] = curr;
            new_root->children[1] = adj_node;
            root = new_root;
            
            split_divide(curr, adj_node);
        }

        // Iteratively preemptively split and insert
        Node *curr = root;

        while (curr)
        {
            int i = bin_search(curr, val);

            // Duplicate entry
            if (i >= 0 && val == curr->keys[i])
                return false;

            i++;

            if (curr->leaf)
            {
                for (int idx = curr->num_keys++; idx > i; idx--)
                    curr->keys[idx] = std::move(curr->keys[idx - 1]);
                
                curr->keys[i] = val;
                curr = nullptr;
            }

            else
            {
                if (curr->children[i]->num_keys == 2 * N - 1)
                {
                    Node *adj_node = new Node, *curr_node = curr->children[i];

                    // Initialize median of child here
                    for (int idx = curr->num_keys++; idx > i; idx--)
                    {
                        curr->keys[idx] = std::move(curr->keys[idx - 1]);
                        curr->children[idx+1] = curr->children[idx];
                    }
                    curr->keys[i] = curr_node->keys[N-1];
                    curr->children[i+1] = adj_node;
                    
                    split_divide(curr_node, adj_node);

                    if (less_than(val, curr->keys[i]))
                        curr = curr->children[i];
                    else if (val == curr->keys[i])
                        return false;
                    else
                        curr = curr->children[i+1];
                }

                else
                    curr = curr->children[i];
            }
        }
        
        return true;
    }

    // Delete
    bool remove(const T &val)
    {
        if (root == nullptr)
            return false;

        // If root has 1 key and left keys == right keys == N - 1 => only then does height decrease (new root needed)
        if (root->num_keys == 1 && root->children[0] && root->children[0]->num_keys == N - 1 && root->children[1]->num_keys == N - 1)
        {
            merge(root->children[0], root->children[1], std::move(root->keys[0]));
            delete root->children[1];
            Node *del = root;
            root = root->children[0];
            delete del;
        }

        Node *node = root;

        while (!node->leaf)
        {
            int idx = bin_search(node, val);

            // 2. In internal node
            if (idx >= 0 && node->keys[idx] == val)
            {
                // 2a. internal node - child with predecessor has at least N keys
                if (node->children[idx]->num_keys >= N)
                {
                    // Find inorder predecessor
                    Node *src = node->children[idx];
                    for (; src->children[src->num_keys]; src = src->children[src->num_keys])
                        ;
    
                    std::swap(node->keys[idx], src->keys[src->num_keys - 1]);
                    node = node->children[idx];
                }
    
                // 2b. internal node - child with successor has at least N keys
                else if (node->children[idx + 1]->num_keys >= N)
                {
                    // Find inorder predecessor
                    Node *src = node->children[idx + 1];
                    for (; src->children[0]; src = src->children[0])
                        ;
    
                    std::swap(node->keys[idx], src->keys[0]);
                    node = node->children[idx + 1];
                }
    
                // 2c. internal node - children with predecessor and successor have N - 1 keys - merge operation
                else
                {
                    merge_right(node, idx);
                    node = node->children[idx];
                }
            }
    
            // 3. Not in internal node
            else
            {
                idx++;
                if (node->children[idx]->num_keys >= N)
                    node = node->children[idx];
    
                // 3a. Child has N - 1 keys - do a "rotation of keys"
                // 3b. Both children have N - 1 keys = merge operation
                // Leftmost child - consider only right sibling
                else
                {
                    if (idx == 0)
                    {
                        if (node->children[idx + 1]->num_keys >= N) // 3a
                        {
                            left_shift(node, idx);
                            node = node->children[idx];
                        }
    
                        // 3b
                        else
                        {
                            merge_right(node, idx);
                            node = node->children[idx];
                        }
                    }
    
                    // Rightmost child - consider only left sibling
                    else if (idx == node->num_keys)
                    {
                        if (node->children[idx - 1]->num_keys >= N) // 3a
                        {
                            right_shift(node, idx);
                            node = node->children[idx];
                        }
    
                        // 3b
                        else
                        {
                            merge(node->children[idx - 1], node->children[idx], std::move(node->keys[idx - 1]));
                            delete node->children[idx];
                            node->children[idx] = nullptr;
                            node->num_keys--;
                            node = node->children[idx - 1];
                        }
                    }
    
                    else
                    {
                        // 3a
                        if (node->children[idx + 1]->num_keys >= N)
                        {
                            left_shift(node, idx);
                            node = node->children[idx];
                        }
    
                        else if (node->children[idx - 1]->num_keys >= N)
                        {
                            right_shift(node, idx);
                            node = node->children[idx];
                        }
    
                        // 3b
                        else
                        {
                            merge_right(node, idx);
                            node = node->children[idx];
                        }
                    }
                }
            }
        }

        int idx = bin_search(node, val);
        if (idx >= 0 && node->keys[idx] == val)
        {
            // Actually delete
            for (int i = idx + 1; i < node->num_keys; i++)
                node->keys[i - 1] = std::move(node->keys[i]);
            node->num_keys--;
            
            if (root->num_keys == 0) // Only happens if node is root
            {
                delete root;
                root = nullptr;
            }

            return true;
        }

        return false;
    }

private: // Attributes
    struct Node
    {
        T keys[2 * N - 1];
        Node *children[2 * N] = {nullptr};
        int num_keys;
        bool leaf;

        Node() : num_keys(0), leaf(false) {}
    };

    Node *root;
    Compare less_than;

private: // Methods
    static void clear(Node *root)
    {
        if (root == nullptr)
            return;

        std::stack<Node *> st;
        st.push(root);

        while (!st.empty())
        {
            Node *top = st.top();
            if (!top->leaf)
            {
                top->leaf = true;
                for (int i = 0; i <= top->num_keys; i++)
                    st.push(top->children[i]);
            }

            else
            {
                delete top;
                st.pop();
            }
        }
    }

    int bin_search(Node *node, const T &val)
    {
        int l = 0, r = node->num_keys - 1;

        while (l <= r)
        {
            int m = (l + r) / 2;
            if (less_than(val, node->keys[m]))
                r = m - 1;
            else
                l = m + 1;
        }

        return r;
    }

    static void split_divide(Node *curr, Node *adj_node)
    {
        // Shift right half of elements to adjacent node
        // Move child pointers too
        adj_node->leaf = curr->leaf;
        adj_node->num_keys = N - 1;
        curr->num_keys = N - 1;

        for (int i = N; i < 2 * N - 1; i++)
        {
            adj_node->keys[i - N] = std::move(curr->keys[i]);
            adj_node->children[i - N] = curr->children[i];
            curr->children[i] = nullptr;
        }

        adj_node->children[N - 1] = curr->children[2 * N - 1];
        curr->children[2 * N - 1] = nullptr;
    }

    static void merge(Node *mer_node, Node *adj_node, T &&median)
    {
        mer_node->keys[mer_node->num_keys++] = std::move(median);

        for (int i = mer_node->num_keys; i < 2 * N - 1; i++, mer_node->num_keys++)
        {
            mer_node->keys[i] = std::move(adj_node->keys[i - N]);
            mer_node->children[i] = adj_node->children[i - N];
        }
        mer_node->children[mer_node->num_keys] = adj_node->children[mer_node->num_keys - N];
    }

    static void left_shift(Node *root, int idx)
    {
        Node *left = root->children[idx], *right = root->children[idx + 1];
        left->keys[left->num_keys++] = std::move(root->keys[idx]);
        left->children[left->num_keys] = right->children[0];
        root->keys[idx] = std::move(right->keys[0]);
        
        right->num_keys--;
        int i;
        for (i = 0; i < right->num_keys; i++)
        {
            right->keys[i] = std::move(right->keys[i + 1]);
            right->children[i] = right->children[i + 1];
        }
        right->children[i] = right->children[i + 1];
    }

    static void right_shift(Node *root, int idx)
    {
        Node *left = root->children[idx - 1], *right = root->children[idx];
        right->num_keys++;
        right->children[right->num_keys] = right->children[right->num_keys - 1];
        for (int i = right->num_keys - 1; i > 0; i--)
        {
            right->keys[i] = std::move(right->keys[i - 1]);
            right->children[i] = right->children[i - 1];
        }
        
        right->keys[0] = std::move(root->keys[idx - 1]);
        right->children[0] = left->children[left->num_keys--];
        root->keys[idx - 1] = std::move(left->keys[left->num_keys]);
    }

    static void merge_right(Node *node, int idx)
    {
        merge(node->children[idx], node->children[idx + 1], std::move(node->keys[idx]));
        delete node->children[idx + 1];
        for (int i = idx + 1; i < node->num_keys; i++)
        {
            node->keys[i - 1] = std::move(node->keys[i]);
            node->children[i] = node->children[i + 1];
        }
        node->children[node->num_keys] = nullptr;
        node->num_keys--;
    }

private: // Friend tester class
    friend class BTreeTester;
};

#endif
