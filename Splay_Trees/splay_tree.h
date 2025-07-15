#ifndef __SPLAY_TREE_H__
#define __SPLAY_TREE_H__

#include <utility>
#include <functional>
#include <stack>
#include "cassert"

enum Direction
{
    D_LEFT,
    D_RIGHT
};

template <typename T, typename Compare = std::less<T>>
class SplayTree
{
public:
    // Constructors
    SplayTree() : node(nullptr) {}
    SplayTree(const T &val)
    {
        node = new TreeNode(val);
    }

    // Destructor
    ~SplayTree()
    {
        clear(node);
    }

    // Copy
    SplayTree(const SplayTree &other)
    {
        auto copy = [](auto copy, const TreeNode *root, TreeNode *parent) -> TreeNode *
        {
            if (root == nullptr)
                return nullptr;
            TreeNode *ret = new TreeNode;
            ret->parent = parent;
            ret->val = root->val;
            ret->children[D_LEFT] = copy(copy, root->children[D_LEFT], ret);
            ret->children[D_RIGHT] = copy(copy, root->children[D_RIGHT], ret);
            return ret;
        };
        node = copy(copy, other.node, nullptr);
        less_than = other.less_than;
    }

    SplayTree &operator=(const SplayTree &other)
    {
        if (this == &other)
            return *this;

        SplayTree new_tree(other);
        std::swap(node, new_tree.node);
        std::swap(less_than, new_tree.less_than);
        return *this;
    }

    // Move
    SplayTree(SplayTree &&other) noexcept : node(other.node), less_than(std::move(other.less_than))
    {
        other.node = nullptr;
    }

    SplayTree &operator=(SplayTree &&other) noexcept
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
        for (; root && root->val != val; root = root->children[less_than(val, root->val) ? D_LEFT : D_RIGHT])
            ;

        if (root == nullptr)
            return false;

        fix(root);
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

        TreeNode *root = node, *root_par = nullptr;
        for (; root && root->val != val; root_par = root, root = root->children[less_than(val, root->val) ? D_LEFT : D_RIGHT])
            ;

        if (root != nullptr)
            return false;

        TreeNode *ins_node = new TreeNode(val);
        root_par->children[less_than(val, root_par->val) ? D_LEFT : D_RIGHT] = ins_node;
        ins_node->parent = root_par;
        fix(ins_node);
        return true;
    }

    // Delete
    bool remove(const T &val)
    {
        if (!find(val))
            return false;

        assert(node->val == val);
        TreeNode *left = node->children[D_LEFT], *right = node->children[D_RIGHT];
        node->children[D_LEFT] = node->children[D_RIGHT] = nullptr;
        delete node;
        node = nullptr;

        if (left == nullptr)
        {
            node = right;
            if (node)
                node->parent = nullptr;
        }
        else
        {
            TreeNode *in_ord_suc = left;
            node = left;
            node->parent = nullptr;
            for (; in_ord_suc->children[D_RIGHT]; in_ord_suc = in_ord_suc->children[D_RIGHT])
                ;
            
            fix(in_ord_suc);
            assert(in_ord_suc->children[D_RIGHT] == nullptr);
            in_ord_suc->children[D_RIGHT] = right;
            if (right)
                right->parent = in_ord_suc;
            in_ord_suc->parent = nullptr;
            node = in_ord_suc;
        }

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
        TreeNode *children[2];
        TreeNode *parent;

        TreeNode() : val(), parent(nullptr)
        {
            children[D_LEFT] = nullptr;
            children[D_RIGHT] = nullptr;
        }

        TreeNode(const T &val) : val(val), parent(nullptr)
        {
            children[D_LEFT] = nullptr;
            children[D_RIGHT] = nullptr;
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
            if (top->children[D_LEFT] || top->children[D_RIGHT])
            {
                if (top->children[D_LEFT])
                {
                    st.push(top->children[D_LEFT]);
                    top->children[D_LEFT] = nullptr;
                }

                if (top->children[D_RIGHT])
                {
                    st.push(top->children[D_RIGHT]);
                    top->children[D_RIGHT] = nullptr;
                }
            }

            else
            {
                delete top;
                st.pop();
            }
        }
    }

    void left_rotate(TreeNode *gp, TreeNode *p, TreeNode *n)
    {
        if (gp == nullptr) // Parent is root
        {
            node = n;
            n->parent = nullptr;
        }
        else
        {
            gp->children[gp->children[D_RIGHT] == p ? D_RIGHT : D_LEFT] = n;
            n->parent = gp;
        }

        p->children[D_RIGHT] = n->children[D_LEFT];
        if (n->children[D_LEFT])
            n->children[D_LEFT]->parent = p;
        n->children[D_LEFT] = p;
        p->parent = n;
    }

    void right_rotate(TreeNode *gp, TreeNode *p, TreeNode *n)
    {
        if (gp == nullptr) // Parent is root
        {
            node = n;
            n->parent = nullptr;
        }
        else
        {
            gp->children[gp->children[D_RIGHT] == p ? D_RIGHT : D_LEFT] = n;
            n->parent = gp;
        }

        p->children[D_LEFT] = n->children[D_RIGHT];
        if (n->children[D_RIGHT])
            n->children[D_RIGHT]->parent = p;
        n->children[D_RIGHT] = p;
        p->parent = n;
    }

    void fix(TreeNode *root)
    {
        while (root != node)
        {
            TreeNode *p = root->parent;
            TreeNode *gp = p != nullptr ? p->parent : nullptr;
            if (gp == nullptr)
            {
                if (p->children[D_LEFT] == root)
                    right_rotate(gp, p, root);
                else
                    left_rotate(gp, p, root);
            }

            else
            {
                // Left
                if (gp->children[D_LEFT] == p)
                {
                    // Left
                    if (p->children[D_LEFT] == root)
                    {
                        right_rotate(gp->parent, gp, p);
                        right_rotate(p->parent, p, root);
                    }

                    // Right
                    else
                    {
                        left_rotate(gp, p, root);
                        right_rotate(gp->parent, gp, root);
                    }
                }

                // Right
                else
                {
                    // Left
                    if (p->children[D_LEFT] == root)
                    {
                        right_rotate(gp, p, root);
                        left_rotate(gp->parent, gp, root);
                    }

                    // Right
                    else
                    {
                        left_rotate(gp->parent, gp, p);
                        left_rotate(p->parent, p, root);
                    }
                }
            }
        }
    }

private:
    friend class SplayTreeTester;
};

#endif
