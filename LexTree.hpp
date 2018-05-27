#ifndef LEXTREE_HPP_INCLUDED
#define LEXTREE_HPP_INCLUDED

/***********************************************************
From: Marco Qiaofeng Liu (Email: qiaofengmarco@outlook.com),
      School of Computer Science and Engineering,
	  Southeast University, Jiulonghu Campus,
	  Nanjing, China
************************************************************/

#include "Headers.h"

//LexTree Node Type
#define NUL  0
#define CAT  1
#define STAR 2
#define OR   3
#define LEAF 4

struct BinNode
{
	int type, id;
    unsigned int content;
	bool nullable = false;
	set<int> firstpos, lastpos, followpos;
    struct BinNode *lchild, *rchild;
    BinNode()
    {
        type = NUL;
        content = 130;
    }
};
class LexTree
{
public:
	LexTree(){}
    LexTree(string a)
    {
        buildTree(a);
    }

	void buildTree(const string &a)
	{
		lex = a;
		root = new BinNode();
        buildTree(a + "$", root);
		updateNodes(root);
	}

    void printTree()
    {
        display(root);
		cout << endl;
    }

	void printFollow()
	{
		BinNode **b = new BinNode*();
		for (int i = 1; i <= nodeSize; i++)
		{
			indexNode(root, i, b);
			for (set<int>::iterator i = (*b)->followpos.begin(); i != (*b)->followpos.end(); i++)
				cout << *i << " ";
			cout << endl;
		}
        delete b;
	}

	static bool indexNode(BinNode*a, int index, BinNode** ans)
	{
		if ((index > a->id) || (a->type == NUL)) return false;

		if ((a->type == LEAF) && (a->id == index))
		{
			*ans = a;
			return true;
		}

		if (indexNode(a->lchild, index, ans)) return true;

		return indexNode(a->rchild, index, ans);
	}

	string lex;
	BinNode* root;

private:
    int nodeSize = 0;

	void buildTree(const string &a, BinNode* r)
	{

		int length = a.size(), left, right, slash1 = 0, slash2;

		right = length - 1;
        left = length - 1;

		r->rchild = new BinNode();
		r->lchild = new BinNode();

        if (right >= 1)
            slash1 = count_slash(a, right - 1) & 1;

		if (length == 0)
        {
            r->type = LEAF;
            r->content = 129;
            return;
        }
        else if (length == 1) //For example: a
        {
            r->type = LEAF;
            r->content = a[right];
            return;
        }
        else if ((length == 2) && (a[0] == '\\') && ((a[1] == '*') || (a[1] == ')') || (a[1] == '(') || (a[1] == '|') || (a[1] == '\\')))
        {
            r->type = LEAF;
            r->content = a[1];
            return;
        }
        else if ((a[right] == ')') && (!slash1))
		{
            left = matchParenthese(a, right);

			if (left == 0) //For example: (ab)
			{
				buildTree(a.substr(1, right - 1), r);
				return;
            }

			//For example: a(bc)
			buildTree(a.substr(left + 1, right - left - 1), r->rchild);
		}
        else if ((a[right] == '*') && (!slash1))
		{
            //For example: c* or (ab)c* or ab\)*
            slash2 = 0;
            if (right >= 2)
                slash2 = count_slash(a, right - 2) & 1;
			if ((a[right - 1] != ')') || ((right >= 2) && (a[right - 1] == ')') && (slash2)))
			{
                if (a[right - 1] != ')')
                {
                    if (right - 1 == 0) //For example c*
                    {
				        r->type = STAR;
			            r->lchild->type = LEAF;
			            r->lchild->content = a[right - 1];
				        return;
                    }

                    //For example: (ab)c*
                    left = right - 1; 
                    buildTree(a.substr(left, 2), r->rchild);
                }
                else
                {
                    if (right - 2 == 0) //For example \)*
                    {
                        r->type = STAR;
                        r->lchild->type = LEAF;
                        r->lchild->content = a[right - 1];
                        return;
                    }
                    
                    //For example: (ab)\)*
                    left = right - 2;
                    buildTree(a.substr(left, 3), r->rchild);
                }
			}
            else if ((right >= 2) && (a[right - 1] == ')') && (!slash2)) //For example: (abc)* or a(bc)*
			{
                left = matchParenthese(a, right - 1); //match parenthese

                if (left == 0) //For example: (abc)*
                { 
				    r->type = STAR;
				    buildTree(a.substr(left + 1, right - left - 2), r->lchild);
				    return;
                }
                
                //For example: a(bc)*
                buildTree(a.substr(left, right - left + 1), r->rchild);
			}
		}
		else if (right - 1 >= 0) //For example: (a)*b
		{
			r->rchild->type = LEAF;
			r->rchild->content = a[right];
            if (slash1)
                left = right - 1;
            else
                left = right;
		}

        if (left == 0) return;

		r->type = CAT;

        if ((a[left - 1] == '|') && ((left == 1) || ((left >= 2) && !(count_slash(a, left - 2) & 1))))
		{
			r->type = OR;
			left--;
		}

		buildTree(a.substr(0, left), r->lchild); //build the left child
	}

	void updateNodes(BinNode *a)
	{
		if (a->type == NUL)
		{
			a->nullable = true;
			return;
		}
		else if (a->type == LEAF)
		{
            a->id = ++nodeSize;

            if (a->content == 129) //epsilon node
            {
                a->nullable = true;
                return;
            }

			a->nullable = false;
			a->firstpos.insert(a->id);
			a->lastpos.insert(a->id);
			return;
		}

		updateNodes(a->lchild);
		updateNodes(a->rchild);

        a->id = nodeSize;

        BinNode** b = new BinNode*();

		if (a->type == OR)
		{
			a->nullable = a->lchild->nullable || a->rchild->nullable;
			setUnion(a->firstpos, a->lchild->firstpos);
			setUnion(a->firstpos, a->rchild->firstpos);
			setUnion(a->lastpos, a->lchild->lastpos);
			setUnion(a->lastpos, a->rchild->lastpos);
		}
		else if (a->type == CAT)
		{
			a->nullable = a->lchild->nullable && a->rchild->nullable;

			if (a->lchild->nullable)
			{
				setUnion(a->firstpos, a->lchild->firstpos);
				setUnion(a->firstpos, a->rchild->firstpos);
			}
			else
				setUnion(a->firstpos, a->lchild->firstpos);
			if (a->rchild->nullable)
			{
				setUnion(a->lastpos, a->lchild->lastpos);
				setUnion(a->lastpos, a->rchild->lastpos);
			}
			else
				setUnion(a->lastpos, a->rchild->lastpos);

            for (set<int>::iterator i = a->lchild->lastpos.begin(); i != a->lchild->lastpos.end(); i++)
            {
                indexNode(a->lchild, *i, b);
                setUnion((*b)->followpos, a->rchild->firstpos);
            }
		}
		else if (a->type == STAR)
		{
			a->nullable = true;
			setUnion(a->firstpos, a->lchild->firstpos);
			setUnion(a->lastpos, a->lchild->lastpos);

            for (set<int>::iterator i = a->lastpos.begin(); i != a->lastpos.end(); i++)
            {
                indexNode(a, *i, b);
                setUnion((*b)->followpos, a->firstpos);
            }
		}

        delete b;
	}

	void display(const BinNode* const r)
	{
		if (r->type == LEAF)
		{
			cout << r->content << " ";
			return;
		}

		if (r->lchild->type != NUL)
		{
			if ((r->type == OR) || (r->type == STAR))
				cout << "( ";
			display(r->lchild);
		}

		if (r->type == OR)
			cout << "| ";
		else if (r->type == STAR)
			cout << ")* ";
		else if (r->type == CAT)
			cout << "o ";

		if (r->rchild->type != NUL)
			display(r->rchild);

		if (r->type == OR)
			cout << ") ";
	}

    void setUnion(set<int> &a, const set<int> &b)
    {
        set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(a, a.begin()));
    }

    int matchParenthese(const string &a, const int right)
    {
        if ((right < 0) || (right > a.size()))
        {
            cout << "Error matching parenthese!" << endl;
            system("pause");
            exit(1);
        }
		if (a[right] != ')') return -1;
        int pcounter = 0, counter = right, slashes = 0;
        while (counter >= 0)
        {
            if (counter >= 1)
            {
                slashes = count_slash(a, counter - 1) & 1;
            }
            if ((a[counter] == '(') && ((counter == 0) || ((counter >=  1) && (!slashes))))
            {
                pcounter--;
                if (pcounter == 0) return counter;
            }
            else if ((counter >= 1) && (a[counter] == ')') && (!slashes))
            {
                pcounter++;
            }
            counter--;
        }
        return -1;
    }

    int count_slash(const string &a, const int right)
    {
        int counter = 0, pos = right;
        if ((right < 0) || (right > a.size()))
        {
            cout << "Error counting slashes!" << endl;
            system("pause");
            exit(1);
        }
        while ((pos >= 0) && (a[pos] == '\\'))
        {
            counter++;
            pos--;
        }
        return counter;
    }
};

#endif // LEXTREE_HPP_INCLUDED
