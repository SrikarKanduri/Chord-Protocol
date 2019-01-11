#include <iostream>
#include <fstream>
#include <sstream>
#include <ctype.h>
#include <cmath>
#include <vector>
#include <map>

using namespace std;

unsigned int m;

class Node {
public:
    int id;
    Node* successor;
    Node* predecessor;
    vector<Node*> finger;

    Node(int id) {
        this->id = id;
        successor = nullptr;
        predecessor = nullptr;
        finger = vector<Node*> (m, this);
    }

    // ask node to find the successor of n
    Node* find_successor(int id) {
//        cout<<id<<" "<<successor->id<<endl;
        if(this->id == successor->id ||
        (successor->id > this->id && this->id < id && id <= successor->id) ||
                (successor->id < this->id && (id > this->id || id <= successor->id)))
            return successor;
        Node* next = closest_preceding_node(id);
        return next->find_successor(id);
    }

    // search the local table for the highest predecessor of n
    Node* closest_preceding_node(int id) {
        for(int i = m-1; i >= 0; --i) {
            if(finger[i]->id > this->id && finger[i]->id < id) {
//                cout<<"HAHAHAHA" << finger[i]->id<<endl;
                return finger[i];
            }
        }
        return this;
    }

    // create a new Chord ring.
    void create() {
        successor = this;
        predecessor = nullptr;
    }

    // join a Chord ring containing node n
    void join(Node* n) {
        predecessor = nullptr;
        successor = n->find_successor(id);
    }

    // called periodically. verifies n’s immediate
    // successor, and tells the successor about n
    void stabilize() {
        Node* x = successor->predecessor;
        if(x != nullptr && ((x->id > id && x->id < successor->id) ||
                            (x->id < id && x->id < successor->id) ||
                            (x->id > id && x->id > successor->id) ||
                            (this == successor)))
            successor = x;

        successor->notify(this);
    }

    // n thinks it might be our predecessor.
    void notify(Node* n) {
        if(predecessor == nullptr ||
           (n->id > predecessor->id) ||
           (predecessor->id == this->id)) {
            predecessor = n;
        }
    }

    // called periodically. refreshes finger table entries.
    // next stores the index of the next finger to fix.
    void fix_fingers() {
        int twoPow = (int) pow(2, m);
        for(int next = 1; next <= m; ++next) {
            finger[next-1] = find_successor((id + (int) pow(2, next-1)) % twoPow);
        }
    }
};

map<int, Node*> nodes;

bool error_node_exists(int id) {
    if(nodes.find(id) != nodes.end()) {
        cout << "Node " << id << " exists" << endl;
        return true;
    }
    return false;
}

bool error_node_not_exists(int id) {
    if(nodes.find(id) == nodes.end()) {
        cout << "Node " << id << " does not exist" << endl;
        return true;
    }
    return false;
}

bool error_node_out_of_bounds(int id) {
    int twoPow = (int) pow(2, m);

    if(id >= twoPow) {
        cout << "ERROR: node id must be in [0," << twoPow << ")" << endl;
        return true;
    }
    return false;
}

void error_syntax(const string &cmd, int actual, int expected) {
    cout<<"SYNTAX ERROR: "<<cmd<<" expects "<<expected<<" parameters not "<<actual<<endl;
}

void add(int id) {
    if(error_node_out_of_bounds(id) || error_node_exists(id)) return;

    bool newRing = false;
    if(nodes.empty()) newRing = true;

    nodes[id] = new Node(id);
    if(newRing) { // Fix fingers only for the first node
        nodes[id]->create();
    }

    cout<<"< Added node "<<id<<endl;
}

void drop(int id) {
    if(error_node_out_of_bounds(id) || error_node_not_exists(id)) return;

    nodes[id]->successor->predecessor = nodes[id]->predecessor;

    if(nodes[id]->predecessor)
        nodes[id]->predecessor->successor = nodes[id]->successor;

    delete(nodes[id]);
    nodes.erase(id);
}

void join(int from, int to) {
    if(error_node_out_of_bounds(from) ||
            error_node_not_exists(from) ||
            error_node_out_of_bounds(to) ||
            error_node_not_exists(to)) return;
    if(from == to) cout<<"ERROR : Cannot add node to itself"<<endl;

    nodes[from]->join(nodes[to]);
}

void fix(int id) {
    if(error_node_out_of_bounds(id) || error_node_not_exists(id)) return;

    nodes[id]->fix_fingers();
}

void stab(int id) {
    if(error_node_out_of_bounds(id) || error_node_not_exists(id)) return;

    nodes[id]->stabilize();
}

void list() {
    if(nodes.empty()) {
        cout<<"ERROR: No nodes in the ring"<<endl;
        return;
    }

    auto it = nodes.begin();
    cout<<"Nodes: ";
    while(it != nodes.end()) {
        cout<<it->first;
        if(it++ != prev(nodes.end())) cout<<", ";
    }
    cout<<endl;
}

void show(int id) {
    if(error_node_out_of_bounds(id) || error_node_not_exists(id)) return;

    cout<<"< Node "<<id;
    cout<<": suc "<<nodes[id]->successor->id;
    cout<<", pre "<<((nodes[id]->predecessor == nullptr) ? "None" : to_string(nodes[id]->predecessor->id));
    cout<<": finger ";

    Node* n = nodes[id];
    for(int i = 0; i < m-1; ++i) {
        cout<<n->finger[i]->id<<",";
    }
    cout<<n->finger[m-1]->id<<endl;
}

void delete_all_nodes() {
    for(auto it: nodes)
        delete(it.second);
}

bool is_number(string& s)
{
    return !s.empty() && find_if(s.begin(),
                                      s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

void parse_input() {
    string cmd;
    string token;
    string par;

    do {
        bool validInput = true;

        vector<int> tokens;
        getline(cin, token, '\n');
        istringstream iss(token);
        iss>>cmd;

        while(!iss.eof()) {
            iss>>par;
            if(!is_number(par)) {
                validInput = false;
                break;
            }
            tokens.push_back(stoi(par));
        }

        if(!validInput) {
            cout<<"ERROR: invalid integer "<<par<<endl;
            continue;
        }

        if(cmd == "end") {
            delete_all_nodes();
            break;
        }

        if(cmd == "add") {
            if(tokens.size() != 1)
                error_syntax(cmd, (int) tokens.size(), 1);
            else
                add(tokens[0]);
        } else if(cmd == "drop") {
            if(tokens.size() != 1)
                error_syntax(cmd, (int) tokens.size(), 1);
            else
                drop(tokens[0]);
        } else if(cmd == "join") {
            if(tokens.size() != 2)
                error_syntax(cmd, (int) tokens.size(), 2);
            else
                join(tokens[0], (int) tokens[1]);
        } else if(cmd == "fix") {
            if(tokens.size() != 1)
                error_syntax(cmd, (int) tokens.size(), 1);
            else
                fix(tokens[0]);
        } else if(cmd == "stab") {
            if(tokens.size() != 1)
                error_syntax(cmd, (int) tokens.size(), 1);
            else
                stab(tokens[0]);
        } else if(cmd == "list") {
            if(tokens.empty())
                error_syntax(cmd, (int) tokens.size(), 0);
            else
                list();
        } else if(cmd == "show") {
            if(tokens.size() != 1)
                error_syntax(cmd, (int) tokens.size(), 1);
            else
                show(tokens[0]);
        }

    } while(true);
}

int main(int argc, char* argv[]) {
    string cmd;

    m = stoi(argv[1]);
    parse_input();

    return 0;
}