#ifndef __RBTREE_H__
#define __RBTREE_H__

#include <utility>
#include <functional>
#include <stack>

enum color_t
{
    RED,
    BLACK
};

enum dir_t
{
    LEFT = 0,
    RIGHT
};

template <typename T, typename Compare = std::less<T>>
class RBTree
{
public:
    // Constructors
    RBTree() : node(nullptr) {}
    RBTree(const T &val)
    {
        node = new TreeNode(val);
    }

    // Destructor
    ~RBTree()
    {
        clear(node);
    }

    // Copy
    RBTree(const RBTree &other)
    {
        auto copy = [](auto copy, const TreeNode *root, const TreeNode *parent)
        {
            if (root == nullptr)
                return nullptr;
            TreeNode *ret = new TreeNode;
            ret->parent = parent;
            ret->val = root->val;
            ret->color = root->color;
            ret->children[LEFT] = copy(copy, root->children[LEFT], ret);
            ret->children[RIGHT] = copy(copy, root->children[RIGHT], ret);
            return ret;
        };
        node = copy(copy, other.node, nullptr);
	less_than = other.less_than;
    }

    RBTree &operator=(const RBTree &other)
    {
        if (this == &other)
            return *this;

        RBTree new_tree(other);
        std::swap(node, new_tree.node);
        std::swap(less_than, new_tree.less_than);
        return *this;
    }

    // Move
    RBTree(RBTree &&other) noexcept : node(other.node), less_than(std::move(other.less_than))
    {
        other.node = nullptr;
    }

    RBTree &operator=(RBTree &&other) noexcept
    {
        if (this == &other)
            return *this;

        clear(node);
        node = other.node;
	less_than = std::move(less_than);
        other.node = nullptr;
        return *this;
    }

    // Search
    bool find(const T &val) const
    {
        for (TreeNode *search = node; search; search = search->children[look(val, search)])
            if (search->val == val)
                return true;

        return false;
    }

    // Insert
    bool add(const T &val)
    {
        if (find(val))
            return false;

        // Insert
        TreeNode *ins_par = nullptr;
        for (TreeNode *ins = node; ins; ins_par = ins, ins = ins->children[look(val, ins)])
            ;

        TreeNode *ins_node = new TreeNode(val);
        ins_node->parent = ins_par;

        if (ins_par == nullptr) // No nodes - Case 0
        {
            node = ins_node;
            node->color = BLACK;
            return true;
        }
        else // Insert as child to parent
            ins_par->children[look(val, ins_par)] = ins_node;

        // Balance
        // Case 1 -> parent is root and is red
        // Case 2 -> uncle is red
        // Case 3 -> uncle is black and triangle
        // Case 4 -> uncle is black and line
        // If parent is black, no balancing
        while (is_red(ins_par))
        {
            if (ins_par->parent == nullptr) // Case 1
            {
                ins_par->color = BLACK;
            }

            else
            {
                TreeNode *ins_uncle = sibling(ins_par);
                if (is_red(ins_uncle)) // Case 2
                {
                    ins_uncle->color = BLACK;
                    ins_par->color = BLACK;
                    ins_par->parent->color = RED;
                    ins_node = ins_par->parent;
                    ins_par = ins_node->parent;
                }

                else
                {
                    if (ins_par->children[LEFT] == ins_node && ins_par->parent->children[LEFT] == ins_par) // Case 4A
                    {
                        ins_par->color = BLACK;
                        ins_par->parent->color = RED;
                        right_rotate(ins_par->parent->parent, ins_par->parent, ins_par);
                    }

                    else if (ins_par->children[RIGHT] == ins_node && ins_par->parent->children[RIGHT] == ins_par) // Case 4B
                    {
                        ins_par->color = BLACK;
                        ins_par->parent->color = RED;
                        left_rotate(ins_par->parent->parent, ins_par->parent, ins_par);
                    }

                    else if (ins_par->children[LEFT] == ins_node && ins_par->parent->children[RIGHT] == ins_par) // Case 3A
                    {
                        right_rotate(ins_par->parent, ins_par, ins_node);
                        ins_node = ins_par;
                        ins_par = ins_node->parent;
                    }

                    else // Case 3B
                    {
                        left_rotate(ins_par->parent, ins_par, ins_node);
                        ins_node = ins_par;
                        ins_par = ins_node->parent;
                    }
                }
            }
        }

        if (ins_par == nullptr)
            ins_node->color = BLACK;
        return true;
    }

    // Delete
    bool remove(const T &val)
    {
        TreeNode *del_node = node;
        for (; del_node && !(del_node->val == val); del_node = del_node->children[look(val, del_node)])
            ;

        if (del_node == nullptr) // Value not in tree
            return false;

        // 2 children
        if (del_node->children[LEFT] && del_node->children[RIGHT])
        {
            TreeNode *inord = del_node->children[RIGHT];
            for (; inord->children[LEFT]; inord = inord->children[LEFT])
                ;
            std::swap(del_node->val, inord->val);
            del_node = inord;
        }

        // 1 child
        auto one_child_policy = [&](dir_t child_dir)
        {
            if (del_node == node)
            {
                node = del_node->children[child_dir];
                del_node->children[child_dir]->parent = nullptr;
            }

            else if (del_node->parent->children[LEFT] == del_node)
            {
                del_node->parent->children[LEFT] = del_node->children[child_dir];
                del_node->children[child_dir]->parent = del_node->parent;
            }

            else
            {
                del_node->parent->children[RIGHT] = del_node->children[child_dir];
                del_node->children[child_dir]->parent = del_node->parent;
            }

            del_node->children[child_dir]->color = BLACK;
            delete del_node;
            return true;
        };

        if (del_node->children[LEFT] != nullptr && del_node->children[RIGHT] == nullptr)
            return one_child_policy(LEFT);

        else if (del_node->children[RIGHT] != nullptr && del_node->children[LEFT] == nullptr)
            return one_child_policy(RIGHT);

        // No children
        else
        {
            // Root
            if (del_node == node)
            {
                node = nullptr;
                delete del_node;
                return true;
            }

            // Red
            else if (is_red(del_node))
            {
                del_node->parent->children[del_node->parent->children[LEFT] == del_node ? LEFT : RIGHT] = nullptr;
                delete del_node;
                return true;
            }

            // Black
            else
            {
                black_leaf_delete(del_node);
                delete del_node;
                return true;
            }
        }
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
        color_t color;
        TreeNode *children[2];
        TreeNode *parent;

        TreeNode() : val(), color(RED), parent(nullptr)
        {
            children[LEFT] = nullptr;
            children[RIGHT] = nullptr;
        }

        TreeNode(const T &val) : val(val), color(RED), parent(nullptr)
        {
            children[LEFT] = nullptr;
            children[RIGHT] = nullptr;
        }
    };

    TreeNode *node;
    Compare less_than;

private: // Functions
    inline dir_t look(const T &val, TreeNode *node) const
    {
        return less_than(val, node->val) ? LEFT : RIGHT;
    }

    static inline bool is_red(TreeNode *node)
    {
        return node && node->color == RED;
    }

    static inline TreeNode *sibling(TreeNode *node)
    {
        return node->parent->children[node->parent->children[LEFT] == node ? RIGHT : LEFT];
    }

    void clear(TreeNode *node)
    {
        if (node == nullptr)
            return;

        std::stack<TreeNode *> st;
        st.push(node);

        while (!st.empty())
        {
            TreeNode *top = st.top();
            if (top->children[LEFT] || top->children[RIGHT])
            {
                if (top->children[LEFT])
                {
                    st.push(top->children[LEFT]);
                    top->children[LEFT] = nullptr;
                }

                if (top->children[RIGHT])
                {
                    st.push(top->children[RIGHT]);
                    top->children[RIGHT] = nullptr;
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
            gp->children[gp->children[RIGHT] == p ? RIGHT : LEFT] = n;
            n->parent = gp;
        }

        p->children[RIGHT] = n->children[LEFT];
        if (n->children[LEFT])
            n->children[LEFT]->parent = p;
        n->children[LEFT] = p;
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
            gp->children[gp->children[RIGHT] == p ? RIGHT : LEFT] = n;
            n->parent = gp;
        }

        p->children[LEFT] = n->children[RIGHT];
        if (n->children[RIGHT])
            n->children[RIGHT]->parent = p;
        n->children[RIGHT] = p;
        p->parent = n;
    }

    void black_leaf_delete(TreeNode *N)
    {
        // Only problem - violates RB Tree Properties - FIX!!
        TreeNode *P = N->parent;
        TreeNode *S, *C, *D;
        dir_t dir = P->children[LEFT] == N ? LEFT : RIGHT;
        P->children[dir] = nullptr;
        goto start;

        do
        {
            dir = P->children[LEFT] == N ? LEFT : RIGHT;
        start:
            S = P->children[1 - dir];
            D = S->children[1 - dir];
            C = S->children[dir];

            if (is_red(S))
            {
                if (dir == LEFT)
                    left_rotate(P->parent, P, S);
                else
                    right_rotate(P->parent, P, S);

                P->color = RED;
                S->color = BLACK;
                S = C;

                D = S->children[1 - dir];
                if (is_red(D))
                    goto D6;

                C = S->children[dir];
                if (is_red(C))
                    goto D5;

                S->color = RED;
                P->color = BLACK;
                return;
            }

            if (is_red(D))
                goto D6;

            if (is_red(C))
                goto D5;

            if (is_red(P))
            {
                S->color = RED;
                P->color = BLACK;
                return;
            }

            S->color = RED;
            N = P;
        } while (P = N->parent);

    if (!P) // Only change from Wikipedia article - fitting to be on 437
        return;

    D5:
        if (dir == RIGHT)
            left_rotate(P, S, C);
        else
            right_rotate(P, S, C);
        S->color = RED;
        C->color = BLACK;
        D = S;
        S = C;

    D6:
        if (dir == LEFT)
            left_rotate(P->parent, P, S);
        else
            right_rotate(P->parent, P, S);
        S->color = P->color;
        P->color = BLACK;
        D->color = BLACK;
        return;
    }

private: // Test Suite Class
    friend class RBTreeTest;
};

#endif
