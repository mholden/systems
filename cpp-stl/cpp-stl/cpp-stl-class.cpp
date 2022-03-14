#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <unordered_set>
#include <map>
#include <unordered_map>

namespace myns {

class Key {
public:
    int k_key;
    
    Key(int);
    
    bool operator==(const Key& k2) const {
        if (this->k_key == k2.k_key)
            return true;
        else
            return false;
    }
    
    bool operator<(const Key& k2) const {
        return this->k_key < k2.k_key;
    }
};

Key::Key(int k) {
    this->k_key = k;
}

class Value {
public:
    int v_value;
    Value(int);
};

Value::Value(int v) {
    this->v_value = v;
}

}

// specialize std::hash for our class
namespace std {
template<>
class hash<myns::Key> {
public:
    size_t operator()(const myns::Key& k) const {
        return std::hash<int>()(k.k_key);
    }
};
}

int main(int argc, char **argv) {
    int err = 0, num;
    std::unordered_set<myns::Key> keys;
    std::map<myns::Key, myns::Value> m;
    std::unordered_map<myns::Key, myns::Value> um;
    std::chrono::steady_clock::time_point tstart, tend;
    std::chrono::duration<double, std::milli> mit(0), mlt(0), umit(0), umlt(0);
    std::chrono::duration<double, std::micro> avg;
    
    if (argc != 2) {
        std::cout << "usage: " << argv[0] << " <num>" << std::endl;
        return -1;
    }
    
    std::cout << argv[0] << " " << argv[1] << std::endl;
    
    num = std::stoi(argv[1], nullptr);
    
    srand((int)time(NULL));
    
    //std::cout << "*** setup ***\n";
    
    //std::cout << "setup: start\n";
    // build a set of 'num' random integers
    for (int i = 0; i < num; i++) {
        myns::Key k(rand());
        auto [it, success] = keys.insert(k);
        while (!success) {
            k.k_key = rand();
            auto [nit, nsuccess] = keys.insert(k);
            success = nsuccess;
        }
    }
    //std::cout << "keys.size(): " << keys.size() << std::endl;
    //std::cout << "setup: done\n";

#if 0
    // dump set
    std::cout << "*** set ***\n";
    for (auto it = begin(keys); it != end(keys); it++) {
        std::cout << "keys[" << std::distance(keys.begin(), it) << "]: " << it->k_key << std::endl;
    }
#endif
    
    //std::cout << "mit: " << mit.count() << " mlt: " << mlt.count() << " umit: " << umit.count() << " umlt: " << umlt.count() << "\n";
    
    // *********** maps **********
    std::cout << "*** maps ***\n";
    
    // inserts
    //std::cout << "maps: starting inserts\n";
    for (auto it = begin(keys); it != end(keys); it++) {
        myns::Key k(it->k_key);
        myns::Value v(it->k_key);
        tstart = std::chrono::steady_clock::now();
        auto [iit, success] = m.insert({k, v});
        tend = std::chrono::steady_clock::now();
        mit += tend - tstart;
        if (!success) {
            std::cout << "m.insert failed\n" << std::endl;
            goto out;
        }
    }
    //std::cout << "maps: done inserts\n";
    avg = mit / num;
    std::cout << "maps insert time: " << mit.count() << "ms" << " avg: " << avg.count() << "us\n";
    
#if 0
    // dump map
    std::cout << "*** map ***\n";
    for (auto& [k, v] : m) {
        std::cout << "m[" << k.k_key << "]: " << v.v_value << std::endl;
    }
#endif
    
    // lookups
    //std::cout << "maps: starting lookups\n";
    for (int i = 0; i < num; i++) {
        auto bno = rand() % keys.bucket_count();
        while (!keys.bucket_size(bno)) { // find a non-empty bucket
            bno++;
            if (bno >= keys.bucket_count())
                bno = 0;
        }
        auto it = keys.begin(bno);
        std::advance(it, rand() % keys.bucket_size(bno));
        //std::cout << "looking up key " << *it << "\n";
        tstart = std::chrono::steady_clock::now();
        auto record = m.find(*it);
        tend = std::chrono::steady_clock::now();
        mlt += tend - tstart;
        if (record == m.end()) {
            std::cout << "m.find failed\n" << std::endl;
            goto out;
        }
        //std::cout << "found m[" << record->first << "]: " << record->second << "\n";
    }
    //std::cout << "maps: done lookups\n";
    avg = mlt / num;
    std::cout << "maps lookup time: " << mlt.count() << "ms" << " avg: " << avg.count() << "us\n";
    
    // *********** unordered maps **********
    std::cout << "*** unordered maps ***\n";
    
    // inserts
    //std::cout << "umaps: starting inserts\n";
    for (auto it = begin(keys); it != end(keys); it++) {
        myns::Key k(it->k_key);
        myns::Value v(it->k_key);
        tstart = std::chrono::steady_clock::now();
        auto [iit, success] = um.insert({k, v});
        tend = std::chrono::steady_clock::now();
        umit += tend - tstart;
        if (!success) {
            std::cout << "um.insert failed\n" << std::endl;
            goto out;
        }
    }
    //std::cout << "umaps: done inserts\n";
    avg = umit / num;
    std::cout << "umaps insert time: " << umit.count() << "ms" << " avg: " << avg.count() << "us\n";
    
#if 0
    // dump umap
    std::cout << "*** umap ***\n";
    for (auto& [k, v] : um) {
        std::cout << "um[" << k.k_key << "]: " << v.v_value << std::endl;
    }
#endif
    
    // lookups
    //std::cout << "umaps: starting lookups\n";
    for (int i = 0; i < num; i++) {
        auto bno = rand() % keys.bucket_count();
        while (!keys.bucket_size(bno)) { // find a non-empty bucket
            bno++;
            if (bno >= keys.bucket_count())
                bno = 0;
        }
        auto it = keys.begin(bno);
        std::advance(it, rand() % keys.bucket_size(bno));
        //std::cout << "looking up key " << *it << "\n";
        tstart = std::chrono::steady_clock::now();
        auto record = um.find(*it);
        tend = std::chrono::steady_clock::now();
        umlt += tend - tstart;
        if (record == um.end()) {
            std::cout << "um.find failed\n" << std::endl;
            goto out;
        }
        //std::cout << "found m[" << record->first << "]: " << record->second << "\n";
    }
    //std::cout << "maps: done lookups\n";
    avg = umlt / num;
    std::cout << "umaps lookup time: " << umlt.count() << "ms" << " avg: " << avg.count() << "us\n";

out:
    return err;
}
