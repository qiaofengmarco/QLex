#ifndef MYDFA_HPP_INCLUDED
#define MYDFA_HPP_INCLUDED

/*********************************************************************
From: Marco Qiaofeng Liu (Email: qiaofengmarco@outlook.com),
      School of Computer Science and Engineering,
      Southeast University, Jiulonghu Campus,
      Nanjing, China
*********************************************************************/

#include "Headers.h"
#include "LexTree.hpp"

//state type
//if the state is terminated, then its type >= 0
#define NONTERMINATED -1

//merge type
#define MERGER true
#define MERGEE false

//set union operation
template<typename T>
void setUnion(set<T> &a, const set<T> &b)
{
    set_union(a.begin(), a.end(), b.begin(), b.end(), inserter(a, a.begin()));
}

//simple quick sort
void qsort(vector<int> &a, vector<int> &b, int left, int right)
{
    int i, j, n, m, p;
    if (left > right) return;
    i = left;
    j = right;
    p = (int) floor((left + right) / 2);
    m = a[p];
    n = b[p];
    do
    {
        while ((a[i] < m) || ((a[i] == m) && (b[i] < n))) i++;
        while ((a[j] > m) || ((a[j] == m) && (b[j] > n))) j--;
        if (i <= j)
        {
            swap(a[i], a[j]);
            swap(b[i], b[j]);
            i++;
            j--;
        }
    } while (i <= j);
    qsort(a, b, left, j);
    qsort(a, b, i, right);
}

//link type
struct link
{
    struct state* To;
    bool non_empty;
    link()
    {
        non_empty = false;
    }
};

//DFA state type
struct state
{
    int state_type, state_id;
    set<int> core;
    set<int> action_set;
    link *links;
    state()
    {
        links = new link[130];
    }
    ~state()
    {
        delete[] links;
    }
};

class DFA
{
public:
    DFA() {}

    DFA(string pattern)
    {
         construct(pattern);
    }

    void construct(const string &pattern)
    {
        cout << "Constructing a DFA..." << endl;
        Tree.buildTree(pattern);
        construct();
        cout << "Finish constructing." << endl;
     }

    /*
    Input:        string
    Output:       an integer value
    Return value: -1: no matches 
    otherwise: pattern id
    */
    int match(const string &s)
    {
         return match(start, s + "$", 0);
    }

    //print the LexTree
    void printTree() 
    {
        if (merge_type != MERGER)
            Tree.printTree();
        else
        {
            cerr << "There is no LexTrees in this DFA!" << endl;
            exit(1);
        }
    }

    //print the follow table in the LexTree
    void printFollow() 
    {
        Tree.printFollow();
    }

    //minimize the number of DFA's states using Hopcroft Algorithm 
    void minimize()
    {
        cout << "Start minimizing DFA..." << endl;
        Hopcroft_Algorithm();
        cout << "Finish minimizing." << endl;
    }

    friend void merge(DFA &d1, vector<DFA> dfas); //merge DFAs

    state* start; //the start state

    int this_state_num = 0; //the number of the states in this DFA

    //MERGER: this DFA is exactly what we get after merging DFAs
    //MERGEE: the DFAs to be merged
    bool merge_type = MERGEE; 

    static int pattern_num; //the number of the existed patterns, and also an identifier of the type of the pattern
    static int state_num;  //the number of our final result
    static int terminated_num; //the number of the terminated states in the final state list
    static int state_ids;  //it is used to offer a new state an id to identify itself 
    static hash_map<int, state*> states; //the final state list
    static hash_map<int, state*> states_temp; //use state_id to index a state, it is actually a temporary variable 
    static map<set<int>, state*> states1; //use a core to index a state, it is actually a temporary variable

private:

    LexTree Tree; //LexTree

    void construct()
    {
        this_state_num = 0;
        states1.clear();
        start = new state();
        setUnion(start->core, Tree.root->firstpos);
        start->state_id = state_ids;
        states_temp[state_ids] = start;
        this_state_num++;
        state_ids++;
        construct(start);
    }

    //construct a DFA
    void construct(state* s)
    {
        int action;
        BinNode** b = new BinNode*();
        bool endFlag = true;

        map<set<int>, state*>::iterator state_it;
        vector< set<int> > act_core;
        act_core.resize(130);

        //get all next actions and cores
        for (set<int>::iterator i = s->core.begin(); i != s->core.end(); i++)
        {
            unsigned int this_action;
            LexTree::indexNode(Tree.root, *i, b);
            this_action = (*b)->content;
            s->action_set.insert(this_action);
            setUnion<int>(act_core[this_action], (*b)->followpos); //get all next core
        }

        //for all possible next actions
        for (set<int>::iterator i = s->action_set.begin(); i != s->action_set.end(); i++)
        {

            action = *i;

            state_it = states1.find(act_core[action]);

            if (state_it == states1.end()) //could not find one, then create a new state
            {
                endFlag = false;
                s->links[action].non_empty = true;
                s->links[action].To = new state();
                setUnion<int>(s->links[action].To->core, act_core[action]);
                states1.insert(map<set<int>, state*>::value_type(s->links[action].To->core, s->links[action].To));

                s->links[action].To->state_id = DFA::state_ids;
                states_temp[state_ids] = s->links[action].To; //use id to index a state

                state_ids++;
                this_state_num++;

                construct(s->links[action].To); //construct a new state recursively
            }
            else //pointing to the existed state
            {
                endFlag = false;
                s->links[action].non_empty = true;
                s->links[action].To = (*state_it).second;
            }
        }

        //just simplify the process for determining whether it is terminated or not 
        if (endFlag) //terminated
        {
            s->state_type = DFA::pattern_num;
            pattern_num++;
        }
        else //nonterminated
            s->state_type = NONTERMINATED;

        vector< set<int> > ().swap(act_core);
        delete b;
    }

    int match(state* s, const string &str, unsigned int start_int)
    {
        int ans = -1;
        state** cur = new state*;
        *cur = s;
        if (start_int > str.size()) return -1;
        for (unsigned int i = start_int; i <= str.size(); i++)
        {
            if (i == str.size())
            {
                ans = (*cur)->state_type;
                delete cur;
                if (ans >= 0)
                    cout << "Match!" << endl;
                return ans;
            }
            if ((*cur)->links[str[i]].non_empty)
            {
                *cur = (*cur)->links[str[i]].To;
                continue;
            }
            delete cur;
            return -1;
        }
        delete cur;
        return -1;
    }

    void Hopcroft_Algorithm()
    {
        int partition_num = 0, real_id;

        hash_map<int, set<int> > partitions; //store the partitions

        //edge_num: number of edges
        //partition_id: use state_id to index the partition id
        //If two states have the same partition id, then they are weakly equivalent.      
        vector<int> edge_num, ids, partition_id;
        edge_num.resize(this_state_num);
        ids.resize(this_state_num);
        partition_id.resize(this_state_num);

        for (int i = 0; i < this_state_num; i++)
        {
            edge_num[i] = 0;
        }

        for (int i = 0; i < this_state_num; i++)
        {
            real_id = start->state_id + i;
            ids[i] = i;
            if (states_temp[real_id]->state_type < 0)
            {
                edge_num[i] = states_temp[real_id]->action_set.size();
                partition_id[i] = 0;
            }
        }

        //sort the number of edge
        //if their numbers of edges are different, then they are not weakly equivalent.
        qsort(edge_num, ids, 0, this_state_num - 1);

        //partition the states with the same number of edges into the same partition
        int pre_max = 0;
        set<int> partition;
        for (int i = 0; i < this_state_num; i++)
        {
            real_id = ids[i] + start->state_id;
            if (edge_num[i] > pre_max)
            {
                pre_max = edge_num[i];

                if (partition.size() > 0)
                {
                    partitions[partition_num] = partition;
                    partition.clear();
                    partition_num++;
                }

                partition.insert(real_id);
                partition_id[ids[i]] = partition_num;
            }
            else if ((edge_num[i] == pre_max) && (pre_max > 0))
            {
                partition.insert(real_id);
                partition_id[ids[i]] = partition_num;
            }
        }

        vector<int>().swap(edge_num);
        vector<int>().swap(ids);
        
        partitions[partition_num] = partition;
        partition.clear();
        partition_num++;        

        for (int i = 0; i < this_state_num; i++)
        {
            real_id = i + start->state_id;
            if (states_temp[real_id]->state_type >= 0)
            {
                partition.insert(real_id);
                partitions[partition_num] = partition;
                partition_id[i] = partition_num;
                partition_num++;
                partition.clear();
            }
        }

        set<int> ().swap(partition); //release memory
        

        //Hopcroft Algorithm
        //referring to: https://en.wikipedia.org/wiki/DFA_minimization

        bool change, non_empty, standard_non_empty;
        int next_state_id, standard_act_id, standard_id, act;
        vector<int> standard_id_list;
        standard_id_list.resize(this_state_num);
   
        do
        {
            change = false;
            for (int i = 0; i < partition_num; i++)
            {
                
                //choose a standard element in this partition as a representative
                //if start state is in this partition, then choose it
                //else choose the first element

                if (partition_id[0] == i)
                    standard_id = start->state_id; 
                else
                    standard_id = *partitions[i].begin();
                
                //the standard element of this partition is the state with its state_id equaling to standard_id
                standard_id_list[i] = standard_id;

                if (partitions[i].size() == 1) continue;

                
                set<int> act_set;
                
                //get all actions in current partition
                for (set<int>::iterator it = partitions[i].begin(); partitions[i].size() > 1 && it != partitions[i].end(); it++)
                {
                    setUnion<int>(act_set, states_temp[*it]->action_set);
                }

                //for each action in the possible action set:
                //  if this action can split the current partition
                //  then split the partition into two sub-partitions:
                //  partition 1: the behavior of this action is identical to the standard element's
                //  partition 2: the behavior of this action is different from the standard element's

                for (set<int>::iterator act_it = act_set.begin(); act_it != act_set.end(); act_it++)
                {
                    set<int> q;
                    act = *act_it;
                    standard_non_empty = states_temp[standard_id]->links[act].non_empty;
                    if (standard_non_empty)
                        standard_act_id = states_temp[standard_id]->links[act].To->state_id - start->state_id;
                    for (set<int>::iterator it = partitions[i].begin(); partitions[i].size() > 1 && it != partitions[i].end(); it++)
                    {
                        if (standard_id == *it) continue;
                        non_empty = states_temp[*it]->links[act].non_empty;
                        if (non_empty)
                            next_state_id = states_temp[*it]->links[act].To->state_id - start->state_id;
                        if ((non_empty != standard_non_empty) || ((non_empty == standard_non_empty) && (non_empty) && (partition_id[next_state_id] != partition_id[standard_act_id])))
                        {
                            change = true; //the partition is still changing
                            q.insert(*it);
                            partition_id[*it - start->state_id] = partition_num;
                            partitions[i].erase(it); //remove the current one
                            it = partitions[i].begin();
                        }
                    }
                    
                    //if set q is nonempty, add a new partition
                    if (q.size() > 0)
                        partitions[partition_num++] = q;                 
                }              
            }          
        }
        while (change); //repeat if the partitions are still changing

        state** cur = new state*();
        int partition_id_temp;
        bool dead;
        terminated_num = 0;

        //for all partitions
        for (int i = 0; i < partition_num; i++)
        {
            dead = true;
            *cur = states_temp[standard_id_list[partition_id[i]]];
            for (set<int>::iterator act_it = (*cur)->action_set.begin(); act_it != (*cur)->action_set.end(); act_it++)
            {
                partition_id_temp = partition_id[(*cur)->links[*act_it].To->state_id - start->state_id];
                (*cur)->links[*act_it].To = states_temp[standard_id_list[partition_id_temp]];

                //if the standard element of this partition can be transisted to another partition
                //then it is marked active (dead = false) 
                if (partition_id_temp != partition_id[i])
                    dead = false;
            }
            
            //if it is not a dead state or it is a terminated state
            //then it is added to the final state vector
            if ((!dead) || ((*cur)->state_type >= 0))
            {
                states[i] = *cur;
                if ((*cur)->state_type >= 0)
                    terminated_num++;
            }
        }

        this_state_num = partition_num;
        state_num = partition_num;

        delete cur;       
        vector<int> ().swap(partition_id); //release memeory
        vector<int> ().swap(standard_id_list); //release memory

    }

    friend void epsilon_clousure(state* s); //use epsilon_clousure method to merge DFAs

};

//static variables' initialization
int DFA::pattern_num = 0;
int DFA::state_ids = 0;
int DFA::state_num = 0;
int DFA::terminated_num = 0;
hash_map<int, state*> DFA::states{};
hash_map<int, state*> DFA::states_temp{};
map<set<int>, state*> DFA::states1{};

//merge DFAs in the vector dfas to form bigger DFA new_d 
void merge(DFA &new_d, vector<DFA> dfas)
{
    int this_type = NONTERMINATED, temp_type;
    DFA::states1.clear();
    DFA::state_num = 0;
    DFA::terminated_num = 0;

    if (new_d.merge_type == MERGER) return;

    cout << "Start merging..." << endl;
    
    new_d.start = new state();
    new_d.start->state_id = DFA::state_ids;
    new_d.merge_type = MERGER;
    DFA::state_ids++;
    for (unsigned int i = 0; i < dfas.size(); i++)
    {
        new_d.start->core.insert(dfas[i].start->state_id);
        temp_type = dfas[i].start->state_type;
        if ((this_type < 0) || ((this_type >= 0) && (temp_type >= 0) && (temp_type < this_type)))
            this_type = temp_type;
    }
    if (this_type >= 0)
        DFA::terminated_num++;
    new_d.start->state_type = this_type;
    DFA::states_temp[new_d.start->state_id] = new_d.start;
    DFA::state_num++;

    epsilon_clousure(new_d.start);

    new_d.this_state_num = DFA::state_num;  

    cout << "Finish merging." << endl;
}

//first, turn DFAs into a big NFA
//then, use epsilon clousure method to turn NFA into DFA
void epsilon_clousure(state* s)
{
    int next_type, temp_type;
    int action;
    map<set<int>, state*>::iterator it;
    for (set<int>::iterator i = s->core.begin(); i != s->core.end(); i++)
    {
        setUnion<int>(s->action_set, DFA::states_temp[*i]->action_set);
    }
    for (set<int>::iterator act_it = s->action_set.begin(); act_it != s->action_set.end(); act_it++)
    {   
        next_type = NONTERMINATED;
        action = *act_it;
        set<int> U;
        for (set<int>::iterator core_it = s->core.begin(); core_it != s->core.end(); core_it++)
        {
            if (DFA::states_temp[*core_it]->links[action].non_empty)
            {
                U.insert(DFA::states_temp[*core_it]->links[action].To->state_id); //get the next core
                //determine which pattern this state belongs to, if it is a terminated state
                temp_type = DFA::states_temp[*core_it]->links[action].To->state_type;
                if ((next_type < 0) || ((temp_type >= 0) && (next_type >= 0) && (temp_type < next_type)))
                    next_type = temp_type;
            }
        }
        it = DFA::states1.find(U);
        if (it == DFA::states1.end()) //a new state
        {
            s->links[action].To = new state;
            s->links[action].non_empty = true;
            setUnion(s->links[action].To->core, U);
            s->links[action].To->state_type = next_type;
            s->links[action].To->state_id = DFA::state_ids;
            DFA::states_temp[DFA::state_ids] = s->links[action].To;
            DFA::state_ids++;
            DFA::states1.insert(map<set<int>, state*>::value_type(U, s->links[action].To));
            DFA::states[DFA::state_num] = s->links[action].To;
            DFA::state_num++;
            DFA::terminated_num++;
            epsilon_clousure(s->links[action].To);
        }
        else //an existed state
        {
            s->links[action].non_empty = true;
            s->links[action].To = (*it).second;
            s->links[action].To->state_type = next_type;
        }  
    }
}

#endif