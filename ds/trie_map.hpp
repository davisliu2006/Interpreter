#pragma once

#include <array>
#include <string>

namespace ds {
    template <class type>
    struct trie_map {
        static constexpr int RANGE = 1<<8;

        struct node {
            std::array<node*,RANGE> children = {};
            node* parent;
            uint8_t radix;
            bool leaf;
            type val = type();
            node(node* parent, uint8_t radix, bool leaf): parent(parent), radix(radix), leaf(leaf) {}
            ~node() {
                for (node* nd: children) {delete nd;}
            }
        };

        node* root;

        trie_map() {root = new node(NULL, 0, false);}
        ~trie_map() {delete root;}
    
        node* find(const char* begin, int size, node* _root = NULL) {
            const char* end = begin+size;
            node* curr = (_root? _root : root);
            while (begin < end) {
                uint8_t radix = *begin;
                if (!(curr = curr->children[radix])) {return NULL;}
                begin++;
            }
            return curr;
        }
        node* insert(const char* begin, int size, node* _root = NULL) {
            const char* end = begin+size;
            node* curr = (_root? _root : root);
            while (begin < end) {
                uint8_t radix = *begin;
                node*& trg = curr->children[radix];
                if (!trg) {trg = new node(curr, radix, false);}
                curr = trg;
                begin++;
            }
            curr->leaf = true;
            return curr;
        }
        void prune(node* nd) {
            node* parent = nd->parent;
            parent->children[nd->radix] = NULL;
            delete nd;
        }
        void erase(node* nd) {
            nd->leaf = false;
        }
        node* find(const std::string& str) {return find(str.c_str(), str.size());}
        node* insert(const std::string& str) {return insert(str.c_str(), str.size());}

        std::pair<std::string, node*> get_longest_prefix(const char* begin, node* _root = NULL) {
            const char* end = begin;
            node* curr = (_root? _root : root);
            const char* end_final = begin;
            node* curr_final = NULL;
            while (*end) {
                uint8_t radix = *end;
                curr = curr->children[radix];
                if (!curr) {break;}
                end++;
                if (curr->leaf) {
                    end_final = end; curr_final = curr;
                }
            }
            return {std::string(begin, end_final), curr_final};
        }

        private:
        static void copy_subtree(node* dst_par, node*& dst, node* src) {
            if (!src) {return;}
            dst = new node(dst_par, src->radix, src->leaf);
            dst->val = src->val;
            for (int i = 0; i < RANGE; i++) {
                copy_subtree(dst, dst->children[i], src->children[i]);
            }
        }
        public:
        trie_map(const trie_map& tr)  {
            copy_subtree(NULL, root, tr.root);
        }
        trie_map& operator=(const trie_map& tr) {
            if (this == &tr) {return *this;}
            delete root;
            copy_subtree(NULL, root, tr.root);
            return *this;
        }
    };
}