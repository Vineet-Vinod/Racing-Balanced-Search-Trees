#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include <utility>
#include <functional>
#include <stack>
#include "cassert"

template <typename T, typename Compare = std::less<T>>
class AVLTree
{
public:
    // Constructors
    AVLTree() : node(nullptr) {}
    AVLTree(const T &val)
    {
        node = new TreeNode(val);
    }

    // Destructor
    ~AVLTree()
    {
        clear(node);
    }

    // Copy
    AVLTree(const AVLTree &other)
    {
        auto copy = [](auto copy, const TreeNode *root) -> TreeNode *
        {
            if (root == nullptr)
                return nullptr;
            TreeNode *ret = new TreeNode(root->val);
            ret->height = root->height;
            ret->left = copy(copy, root->left);
            ret->right = copy(copy, root->right);
            return ret;
        };
        node = copy(copy, other.node);
        less_than = other.less_than;
    }

    AVLTree &operator=(const AVLTree &other)
    {
        if (this == &other)
            return *this;

        AVLTree new_tree(other);
        std::swap(node, new_tree.node);
        std::swap(less_than, new_tree.less_than);
        return *this;
    }

    // Move
    AVLTree(AVLTree &&other) noexcept : node(other.node), less_than(std::move(other.less_than))
    {
        other.node = nullptr;
    }

    AVLTree &operator=(AVLTree &&other) noexcept
    {
        if (this == &other)
            return *this;

        clear(node);
        node = other.node;
        less_than = std::move(other.less_than);
        other.node = nullptr;
        return *this;
    }

    // Search
    bool find(const T &val)
    {
        TreeNode *root = node;
        for (; root && root->val != val; root = less_than(val, root->val) ? root->left : root->right)
            ;

        if (root == nullptr)
            return false;

        return true;
    }

    // Insert
    bool add(const T &val)
    {
        if (node == nullptr)
        {
            node = new TreeNode(val);
            return true;
        }

        TreeNode *root = node;
        std::stack<TreeNode *> st;
        for (; root && root->val != val; root = less_than(val, root->val) ? root->left : root->right)
            st.push(root);

        if (root != nullptr)
            return false;

        root = st.top();
        st.pop();
        TreeNode *ins_node = new TreeNode(val);
        if (less_than(val, root->val))
            root->left = ins_node;
        else
            root->right = ins_node;

        do
        {
            if (st.empty())
            {
                node = balance(root);
                root = nullptr;
            }

            else
            {
                TreeNode *n_root = st.top();

                if (n_root->left == root)
                    n_root->left = balance(root);
                else
                    n_root->right = balance(root);

                st.pop();
                root = n_root;
            }
        } while (root);

        return true;
    }

    // Delete
    bool remove(const T &val)
    {
        TreeNode *root = node;
        std::stack<TreeNode *> st;
        for (; root && root->val != val; root = less_than(val, root->val) ? root->left : root->right)
            st.push(root);

        if (root == nullptr)
            return false;

        if (root->left == nullptr && root->right == nullptr)
        {
            if (st.empty())
            {
                delete node;
                node = nullptr;
                return true;
            }

            else
            {
                TreeNode *top = st.top();

                if (top->left == root)
                    top->left = nullptr;
                else
                    top->right = nullptr;

                delete root;
            }
        }

        else if (root->left == nullptr)
        {
            if (st.empty())
            {
                node = root->right;
                root->right = nullptr;
                delete root;
                return true;
            }

            else
            {
                TreeNode *top = st.top();

                if (top->left == root)
                    top->left = root->right;
                else
                    top->right = root->right;

                root->right = nullptr;
                delete root;
            }
        }

        else if (root->right == nullptr)
        {
            if (st.empty())
            {
                node = root->left;
                root->left = nullptr;
                delete root;
                return true;
            }

            else
            {
                TreeNode *top = st.top();

                if (top->left == root)
                    top->left = root->left;
                else
                    top->right = root->left;

                root->left = nullptr;
                delete root;
            }
        }

        else
        {
            TreeNode *in_ord_suc = root->right;
            st.push(root);
            for (; in_ord_suc->left; in_ord_suc = in_ord_suc->left)
                st.push(in_ord_suc);

            std::swap(root->val, in_ord_suc->val);
            root = st.top();
            if (root->left == in_ord_suc)
                root->left = in_ord_suc->right;
            else
                root->right = in_ord_suc->right;
            in_ord_suc->right = nullptr;
            delete in_ord_suc;
        }

        root = st.top();
        st.pop();

        do
        {
            if (st.empty())
            {
                node = balance(root);
                root = nullptr;
            }

            else
            {
                TreeNode *n_root = st.top();

                if (n_root->left == root)
                    n_root->left = balance(root);
                else
                    n_root->right = balance(root);

                st.pop();
                root = n_root;
            }
        } while (root);

        return true;
    }

    void clear()
    {
        clear(node);
        node = nullptr;
    }

private: // Members
    struct TreeNode
    {
        T val;
        TreeNode *left, *right;
        u_int32_t height;

        TreeNode() : val(), height(1)
        {
            left = nullptr;
            right = nullptr;
        }

        TreeNode(const T &val) : val(val), height(1)
        {
            left = nullptr;
            right = nullptr;
        }
    };

    TreeNode *node;
    Compare less_than;

private: // Functions
    void clear(TreeNode *node)
    {
        if (node == nullptr)
            return;

        std::stack<TreeNode *> st;
        st.push(node);

        while (!st.empty())
        {
            TreeNode *top = st.top();
            if (top->left || top->right)
            {
                if (top->left)
                {
                    st.push(top->left);
                    top->left = nullptr;
                }

                if (top->right)
                {
                    st.push(top->right);
                    top->right = nullptr;
                }
            }

            else
            {
                delete top;
                st.pop();
            }
        }
    }

    void update_height(TreeNode *node)
    {
        node->height = std::max(node->left ? node->left->height : 0, node->right ? node->right->height : 0) + 1;
    }

    TreeNode *left_rotate(TreeNode *l, TreeNode *r)
    {
        l->right = r->left;
        r->left = l;
        update_height(l);
        update_height(r);
        return r;
    }

    TreeNode *right_rotate(TreeNode *r, TreeNode *l)
    {
        r->left = l->right;
        l->right = r;
        update_height(r);
        update_height(l);
        return l;
    }

    TreeNode *balance(TreeNode *root)
    {
        u_int32_t lh = root->left ? root->left->height : 0, rh = root->right ? root->right->height : 0;

        if (lh > 1 + rh)
        {
            TreeNode *left = root->left;
            u_int32_t llh = left->left ? left->left->height : 0, lrh = left->right ? left->right->height : 0;

            // Double rotate
            if (lrh > llh)
            {
                root->left = left_rotate(left, left->right);
            }

            return right_rotate(root, root->left);
        }

        else if (rh > 1 + lh)
        {
            TreeNode *right = root->right;
            u_int32_t rlh = right->left ? right->left->height : 0, rrh = right->right ? right->right->height : 0;

            // Double rotate
            if (rlh > rrh)
            {
                root->right = right_rotate(right, right->left);
            }

            return left_rotate(root, root->right);
        }

        update_height(root);
        return root;
    }

private:
    friend class AVLTreeTester;
};

#endif
