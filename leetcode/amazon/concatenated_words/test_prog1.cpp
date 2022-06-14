#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <string.h>

using namespace std;

static void dump_vector(vector<string> v) {
    for (int i = 0; i < v.size(); i++)
        cout << v[i] << " ";
    cout << endl;
}

class TrieNode {
private:
    void prefix_match_cb(TrieNode *root, string& curr, string& word, int nwords, bool& is_concat, bool *stop) {
        string substr;
        //cout << "prefix_match_cb: " << curr << endl;
        if (curr == word) {
            *stop = true;
            if (nwords > 0)
                is_concat = true;
            //cout << "stopping" << endl;
            return;
        }
        substr = word.substr(curr.size());
        iterate_prefix_matches(root, substr, nwords + 1, is_concat, stop);
    }
    
    void iterate_prefix_matches(TrieNode *root, string& word, int nwords, bool& is_concat, bool *stop) {
        string curr = "";
        TrieNode *cn; // curr node
        int ind;
        bool _stop;
        
        //cout << "iterate_prefix_matches: " << word << endl;
        
        cn = root->nodes[word[0] - 'a'];
        ind = 0;
        _stop = false;
        while (cn) {
            curr.push_back(word[ind]);
            if (cn->is_word) {
                prefix_match_cb(root, curr, word, nwords, is_concat, &_stop);
                if (_stop) {
                    if (stop)
                        *stop = true;
                    return;
                }
            }
            ind++;
            if (ind >= word.size())
                break;
            cn = cn->nodes[word[ind] - 'a'];
        }
    }
    
public:
    bool is_word;
    TrieNode *nodes[26];
    
    //TrieNode() {
    //    memset(nodes, 0, sizeof(TrieNode *) * 26);
    //}
    
    int insert(string word) {
        if (!this->nodes[word[0] - 'a'])
            this->nodes[word[0] - 'a'] = new TrieNode();
        if (word.size() == 1)
            this->nodes[word[0] - 'a']->is_word = true;
        else
            this->nodes[word[0] - 'a']->insert(word.substr(1));
        return 0;
    }
    
    bool is_concat(TrieNode *root, string& word) {
        bool _is_concat = false;
        iterate_prefix_matches(root, word, 0, _is_concat, NULL);
        return _is_concat;
    }
    
    void _dump(string& curr) {
        for (int i = 0; i < 26; i++) {
            if (this->nodes[i]) {
                curr.push_back('a' + i);
                if (this->nodes[i]->is_word)
                    cout << curr << endl;
                this->nodes[i]->_dump(curr);
                curr.pop_back();
            }
        }
    }
    
    void dump() {
        string curr = "";
        _dump(curr);
    }
};

vector<string> findAllConcatenatedWordsInADict(vector<string>& words) {
    TrieNode *t;
    vector<string> out;

    t = new TrieNode();
    
    for (int i = 0; i < words.size(); i++)
        t->insert(words[i]);
    
    //t->dump();

    for (int i = 0; i < words.size(); i++) {
        if (t->is_concat(t, words[i])) {
            out.push_back(words[i]);
        }
    }

    return out;
}

static void test_prog1_specific_case1(void) {
    vector<string> words;
    
    words = {"cat","cats","catsdogcats","dog","dogcatsdog","hippopotamuses","rat","ratcatdogcat"};
    dump_vector(findAllConcatenatedWordsInADict(words));
}

static void test_prog1(void) {
    test_prog1_specific_case1();
}

int main(int argc, const char *argv[]) {
    test_prog1();
    return 0;
}
