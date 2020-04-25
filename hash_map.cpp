#include <vector>
#include <list>
#include <stdexcept>
#include <utility>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap {
public:
    HashMap(Hash hasher = Hash()) : hasher(hasher) {
        cnt = 0;
        data.resize(SZ);
        used.resize(SZ);
        deleted.resize(SZ);
        elements = std::list<std::pair<const KeyType, ValueType> >();
    }

    void clear() {
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            size_t id = find_id(it->first);
            deleted[id] = true;
        }
        elements.clear();
    }

    HashMap& operator=(HashMap other) {
        cnt = 0;
        data.clear();
        data.resize(SZ);
        used.clear();
        used.resize(SZ);
        deleted.clear();
        deleted.resize(SZ);
        elements.clear();
        hasher = other.hasher;
        for (auto it = other.begin(); it != other.end(); ++it) {
            insert(*it);
        }
        return *this;
    }

    void insert(std::pair<const KeyType, ValueType> p) {
        if (find(p.first) != end()) return;
        add(p, data, used, elements);
        ++cnt;
        if (cnt * LOAD_FACTOR > data.size()) {
            rebuild();
        }
    }

    template<typename iterator>
    HashMap(iterator l,
        iterator r, Hash hasher = Hash()) {
        *this = HashMap(hasher);
        for (auto it = l; it != r; ++it) {
            insert(*it);
        }
    }
    HashMap(const std::initializer_list<std::pair<KeyType, ValueType> >& list, Hash hasher = Hash()) {
        *this = HashMap(hasher);
        for (auto it = list.begin(); it != list.end(); ++it) {
            insert(*it);
        }
    }
    size_t size() const {
        return elements.size();
    }
    bool empty() const {
        return size() == 0;
    }

    class iterator {
    private:
        typename std::list<std::pair<const KeyType, ValueType> >::iterator it;
    public:
        iterator() {}
        iterator(typename std::list<std::pair<const KeyType, ValueType> >::iterator it) : it(it) {}
        iterator operator++() {
            ++it;
            return *this;
        }
        iterator operator++(int) {
            auto tmp = it;
            ++it;
            return tmp;
        }
        std::pair<const KeyType, ValueType> operator*() {
            return *it;
        }
        std::pair<const KeyType, ValueType>* operator->() {
            return &(*it);
        }
        bool operator==(const HashMap::iterator& other) {
            return it == other.it;
        }
        bool operator!=(const HashMap::iterator& other) {
            return it != other.it;
        }
    };

    class const_iterator {
    private:
        typename std::list<std::pair<const KeyType, ValueType> >::const_iterator it;
    public:
        const_iterator() {}
        const_iterator(typename std::list<std::pair<const KeyType, ValueType> >::const_iterator it) : it(it) {}
        const_iterator operator++() {
            ++it;
            return *this;
        }
        const_iterator operator++(int) {
            auto tmp = it;
            ++it;
            return tmp;
        }
        const std::pair<const KeyType, ValueType> operator*() const {
            return *it;
        }
        const std::pair<const KeyType, ValueType>* operator->() const {
            return &(*it);
        }
        bool operator==(const HashMap::const_iterator& other) const {
            return it == other.it;
        }
        bool operator!=(const HashMap::const_iterator& other) const {
            return it != other.it;
        }
    };

    HashMap::iterator begin() {
        return HashMap::iterator(elements.begin());
    }
    HashMap::iterator end() {
        return HashMap::iterator(elements.end());
    }
    HashMap::const_iterator begin() const {
        return HashMap::const_iterator(elements.begin());
    }
    HashMap::const_iterator end() const {
        return HashMap::const_iterator(elements.end());
    }

    iterator find(KeyType key) {
        size_t id = find_id(key);
        if (id != data.size()) {
            return iterator(data[id]);
        }
        return end();
    }

    const_iterator find(KeyType key) const {
        size_t id = find_id(key);
        if (id != data.size()) {
            return const_iterator(data[id]);
        }
        return end();
    }

    void erase(KeyType key) {
        size_t id = find_id(key);
        if (id != data.size() && get_key(id) == key) {
            deleted[id] = true;
            elements.erase(data[id]);
        }
    }

    ValueType& operator[](KeyType key) {
        size_t id = find_id(key);
        if (id == data.size()) {
            insert(std::make_pair(key, ValueType()));
            id = find_id(key);
        }
        return data[id]->second;
    }

    const ValueType& at(KeyType key) const {
        auto it = find(key);
        if (it == end()) {
            throw std::out_of_range("durka");
        }
        return it->second;
    }

    Hash hash_function() const {
        return hasher;
    }

private:
    const size_t SZ = 3;
    const size_t LOAD_FACTOR = 3;
    size_t cnt;
    std::vector<typename std::list<std::pair<const KeyType, ValueType> >::iterator> data;
    std::vector<bool> used;
    std::vector<bool> deleted;
    std::list<std::pair<const KeyType, ValueType> > elements;
    Hash hasher;

    size_t find_id(KeyType key) const {
        size_t id = hasher(key) % data.size();
        while (used[id] && (deleted[id] || !(get_key(id) == key))) {
            ++id;
            if (id == data.size()) {
                id = 0;
            }
        }
        if (used[id] && !deleted[id]) {
            return id;
        }
        return data.size();
    }

    size_t find_next(size_t i, std::vector<bool>& used) {
        while (used[i]) {
            ++i;
            if (i == used.size()) {
                i = 0;
            }
        }
        return i;
    }

    void add(std::pair<const KeyType, ValueType> p,
        std::vector<typename std::list<std::pair<const KeyType, ValueType> >::iterator>& data, std::vector<bool>& used,
        std::list<std::pair<const KeyType, ValueType> >& elements) {
        size_t id = hasher(p.first);
        id = id % data.size();
        id = find_next(id, used);
        elements.push_front(p);
        data[id] = elements.begin();
        used[id] = true;
    }

    void rebuild() {
        size_t len = data.size() * 2;
        std::vector<typename std::list<std::pair<const KeyType, ValueType> >::iterator> new_data(len);
        std::vector<bool> new_used(len);
        std::list<std::pair<const KeyType, ValueType> > new_elements = std::list<std::pair<const KeyType, ValueType> >();
        for (auto it = elements.begin(); it != elements.end(); ++it) {
            add({ it->first, it->second }, new_data, new_used, new_elements);
        }
        swap(data, new_data);
        swap(used, new_used);
        swap(elements, new_elements);
        deleted.clear();
        deleted.resize(len);
    }

    KeyType get_key(size_t id) const {
        auto t = data[id];
        return t->first;
    }
    ValueType get_value(size_t id) const {
        auto t = data[id];
        return t->second;
    }
};
