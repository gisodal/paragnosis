#ifndef MULTIGRAPH_TABLE_H
#define MULTIGRAPH_TABLE_H

#include "multigraphdef.h"
#include "hash.h"
#include <unordered_map>
#include <unordered_set>
namespace bnc {

inline Hash MultiNodeHash(const MultiGraphDef::Node *n){
    Hasher hasher;
    hasher.Seed(n->variable);
    assert(n->size > 1);

    const auto kEdgeEnd = &(n->edges[n->size]);
    auto edge = &(n->edges[0]);
    while(edge != kEdgeEnd){
        hasher.AddHash((size_t)edge->to);
        const auto kWeightEnd = &(edge->weights[edge->size]);
        auto *weight = &(edge->weights[0]);
        while(weight != kWeightEnd){
            hasher.AddHash(*weight);
            ++weight;
        }
        ++edge;
    }

    return hasher.GetHash();
}

struct MultiGraphHashPtr {
    Hash operator()(const MultiGraphDef::Node *n) const {
        return MultiNodeHash(n);
    }
};

struct MultiGraphEqPtr {
    bool operator()(const MultiGraphDef::Node *m, const MultiGraphDef::Node *n) const {
        return *m == *n;
    }
};

struct MultiGraphComputedTable : public std::unordered_set<MultiGraphDef::Node*, MultiGraphHashPtr, MultiGraphEqPtr> {
    inline MultiGraphDef::Node* find_or_insert(MultiGraphDef::Node *n){
        auto succes = insert(n);
        if(succes.second)
            return NULL;
        return *succes.first;
    }

    inline size_t count_collisions() const {
        size_t collisions = 0;
        for (size_t bucket = 0; bucket != bucket_count(); bucket++){
            size_t size = bucket_size(bucket);
            if(size > 1){
                collisions += size-1;
            }
        }
        //for (size_t bucket = 0; bucket != bucket_count(); ++bucket)
        //    if (bucket_size(bucket) > 1)
        //        ++collisions;

        return collisions;
    }

    inline void stats() const {
        size_t oversized_buckets = 0;
        size_t active_buckets = 0;
        size_t max_bucket_entries = 0;
        size_t largest_bucket = 0;
        for (size_t bucket = 0; bucket != bucket_count(); bucket++){
            size_t size = bucket_size(bucket);
            if(size > 0){
                active_buckets++;
                if(size > 1)
                    oversized_buckets++;
                if(size > max_bucket_entries){
                    max_bucket_entries = size;
                    largest_bucket = bucket;
                }
            }
        }

        #ifdef DEBUG
        printf("\nHashmap distribution:\n");
        printf("=============\n");
        for (size_t bucket = 0; bucket != bucket_count(); bucket++){
            size_t size = bucket_size(bucket);
            printf("%3u] %4lu | ", bucket,size);
            for(auto it = begin(bucket); it != end(bucket); it++){
                MultiGraphDef::Node *node = *it;
                printf("[%lu TO: ",node->variable);
                for(auto edge_it = &(node->edges[0]); edge_it != &(node->edges[node->size]); edge_it++)
                    printf("%lu ",edge_it->to);

                printf("W: ");
                for(auto edge_it = &(node->edges[0]); edge_it != &(node->edges[node->size]); edge_it++)
                    for(auto w_it = &(edge_it->weights[0]); w_it != &(edge_it->weights[edge_it->size]); w_it++)
                        printf("%lu ",*w_it);
                printf("] ");
            }
            printf("\n");
        }
        printf("=============\n");
        #endif

        printf("Entries         : %lu\n", size());
        printf("Buckets         : %lu\n", bucket_count());
        printf("Active buckets  : %lu\n", active_buckets);
        printf("Buckets > 1     : %lu\n", oversized_buckets);
        printf("Most entries/B  : %lu in bucket %lu\n", max_bucket_entries,largest_bucket);
        printf("Collisions      : %lu\n", count_collisions());
        printf("Load factor     : %lf\n", load_factor());
        printf("Active load     : %lf\n", (double)size()/(double)active_buckets);
        printf("Max load factor : %lf\n", max_load_factor());

        printf("\n");
    }




};

}

#endif

