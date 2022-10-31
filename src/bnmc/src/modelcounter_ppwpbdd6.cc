#include <unistd.h>
#include <algorithm>
#include <stack>
#include <bn-to-cnf/config.h>
#include <bn-to-cnf/exceptions.h>
#include <bnc/bayesgraph.h>
#include <bnc/exceptions.h>
#include <climits>
#include "modelcounter.h"
#include "io.h"
#include "options.h"
#include "exceptions.h"
#include "mstack.h"
#include "debug.h"
#include <bnc/xary.h>
#include <pthread.h>
#include <thread>
#include <iostream>
#include <cmath>
#include <queue/singlequeue.h>
#include <queue/blockingmultiqueue.h>
#include <limits.h>
#include "lockfreequeue.h"

namespace bnmc {

using namespace std;
using namespace mstack;
using namespace bnc;

#define N 6

using namespace moodycamel;


#define MAX_THREADS 64

namespace bnmc_6_ {

    struct TaskItem {
        unsigned int node;
        TierId tier;
        bool with_query_variable;
    };

    struct TasksItem {
        uint16_t task_group_id;
        bool with_query_variable;
    };
    const uint16_t kTasksFinished = UINT16_MAX;

    LockFreeQueue< TaskItem, 1024 > task_queue[MAX_THREADS];
    BlockingConcurrentQueue<TasksItem> tasks_queue(1024);

    pthread_barrier_t start_barrier;
    pthread_barrier_t barrier;

    ModelCounter<ModelType::PWPBDD> *gMc;
    const Architecture *gArchitecture;
    const EvidenceList *gEvidenceList;
    const EvidenceList *gEvidenceList2;
    Cache *gCache;
    Cache *gCache2;
    ConditionTierList *gConditionTierList;
    ConditionTierList *gConditionTierList2;
    bool gHasQueryVariable;
    unsigned int gNrThreads;

    __thread unsigned int gThreadId;

}

using namespace bnmc_6_;

template <>
template <>
std::string ModelCounter<ModelType::PWPBDD>::ParallelDescription<N>(){
    return "Based on PWPBDD4 with pthreads. Bottom up traverals of partitions, using pre-initialized evidence cache. Partitions are traversed per tier, starting at the bottom tier.";
}

template<>
template <>
probability_t ModelCounter<ModelType::PWPBDD>::ParallelGetPartitionResult<N>(
        const Architecture &kArchitecture,
        Cache              &cache,
        const EvidenceList &kEvidenceList,
        const unsigned int kTier){

    const size_t kMaxTier = kArchitecture.Size()-1;
    if(kTier <= kMaxTier){
        const Spanning &kSpanning = kArchitecture.GetSpanning();
        const SpanningSet &kSpanningSet = kSpanning.GetSpanningSet(kTier);
        const unsigned int kNodeId = XAry::GetDecimal(manager.bn, kSpanningSet, kEvidenceList);

        Probability &probability = cache.GetCacheEntry(kNodeId, kTier);
        #ifdef DEBUG
        if(probability < 0)
            throw ModelCounterException("Child component was not computed (Node (%u,%u))",kTier,kNodeId);
        #endif
        return probability;
    }
    return 1;
}

template <>
template <>
probability_t ModelCounter<ModelType::PWPBDD>::ParallelTraversePartition<N>(const Architecture &kArchitecture, Cache &cache, EvidenceList &evidence_list, const ConditionTierList &kConditionTierList, const unsigned int kTier, const unsigned int kNodeId){
    #ifdef DEBUG
    bool kWriteDot = false;
    bool kWriteDot2 = false;
    unsigned int kMaxDepth = -1;
    unsigned int depth = 1;
    std::string filename = stringf("debug.pwpbdd.%u.dot", kTier);
    #endif

    const ordering_t &kPartitionOrdering = kArchitecture.GetPartitionOrdering();
    const unsigned int kPartitionId = kPartitionOrdering[kTier];
    const Partition &kPartition = partition_[kPartitionId];
    const Partition::ArithmeticCircuit &kCircuit = kPartition.ac_;
    ProbabilityList &probabilities = cache.GetProbabilityList(gThreadId);

    probabilities[kFalseTerminalIndex] = 0;
    probabilities[kTrueTerminalIndex]  = 1;
    probabilities[kRootIndex] = 0;

    IdentityList &identity = cache.GetIdentityList(gThreadId);
    std::fill(identity.begin(),identity.begin()+kCircuit.size(),0);

    unsigned int unique_id = 1;
    identity[kFalseTerminalIndex] = unique_id;

    StackTop i;
    StackTopInit(i);
    Stack &s = cache.GetStack(gThreadId);
    Push(s,i,kFalseTerminalIndex,kRootIndex);

    bool recompute = false;
    traverse:
    const unsigned int &kParentIndex = s[i-2];
    const unsigned int &kIndex = s[i-1];
    {
        const unsigned int &kParentId = identity[kParentIndex];
        unsigned int &id = identity[kIndex];

        #ifdef DEBUG
        if(kWriteDot)
            WriteDot(filename, kCircuit, evidence_list, probabilities, identity, kTier, kIndex,kMaxDepth,i,&s);
        #endif

        if(kIndex == kTrueTerminalIndex){
            // this is a positive cofactor (implicitly by language definition)
            // true terminal is linked to root of next partition
            const probability_t kNextTierProbability = ParallelGetPartitionResult<N>(kArchitecture, cache, evidence_list,  kTier+1);
            const Partition::Node &kParentNode = kCircuit[kParentIndex];
            probabilities[kParentIndex] += kParentNode.w * kNextTierProbability;
        } else if(kIndex != kFalseTerminalIndex){

            if(recompute || id < kParentId){
                id = (recompute?unique_id++:kParentId);

                const Partition::Node &kNode = kCircuit[kIndex];
                const Variable &kVariable = kNode.v;
                const TierId &kVariableTier = kConditionTierList[kVariable];
                const bool kIsConditioned = kVariableTier <= kTier;
                VariableValue &value = evidence_list[kVariable];
                const bool kIsTrue = value == kNode.i;
                recompute = !kIsConditioned && is_persistence_[kVariable];

                if(!kIsConditioned){
                    // add both cofactors
                    Push(s,i,kIndex,kNode.e);
                    Push(s,i,kIndex,kNode.t);

                    // record decision i.o.t. to the same in the following partition(s)
                    // this is only important for persistence variables
                    value = kNode.i;
                } else if(kIsTrue) {
                    // add only positive cofactor
                    Push(s,i,kIndex,kNode.t);
                } else {
                    // add only negative cofactor
                    Push(s,i,kIndex,kNode.e);
                }

                // initialize probability
                probabilities[kIndex] = 0;
                goto traverse;

            } else {
                // this subgraph has been traversed
                // a child node computes the probability of the parent node
                const Partition::Node &kParentNode = kCircuit[kParentIndex];
                const bool kIsPositiveCofactor = kParentNode.t == kIndex;
                if(kIsPositiveCofactor){
                    // this child is a positive cofactor
                    probabilities[kParentIndex] += kParentNode.w * probabilities[kIndex];
                } else {
                    // this child is a negative cofactor
                    probabilities[kParentIndex] += probabilities[kIndex];
                }

                #ifdef DEBUG
                if(kWriteDot)
                    WriteDot(filename, kCircuit, evidence_list, probabilities, identity, kTier, kIndex,kMaxDepth,i,&s);
                #endif
            }
        }
    }

    recompute = false;
    Pop(i);
    // stop when encountering root node
    if(i > kStackItemSize) // root element should remain on stack
        goto traverse;

    return probabilities[kRootIndex];
}

template <>
template <>
probability_t ModelCounter<ModelType::PWPBDD>::ParallelTraverse<N>(const Architecture &kArchitecture, Cache &cache, const EvidenceList &kEvidenceList, const ConditionTierList &kConditionTierList, const unsigned int kTier, const unsigned int kNodeId){
    const ordering_t &kPartitionOrdering = kArchitecture.GetPartitionOrdering();
    const unsigned int kPartitionId = kPartitionOrdering[kTier];
    const Partition &kPartition = partition_[kPartitionId];
    const Partition::ArithmeticCircuit &kCircuit = kPartition.ac_;

    ProbabilityList &probabilities = cache.GetProbabilityList(gThreadId);
    std::fill(probabilities.begin(), probabilities.begin()+kCircuit.size(), kNotTraversed);
    probabilities[kFalseTerminalIndex] = 0;
    probabilities[kTrueTerminalIndex]  = 1;
    probabilities[kRootIndex] = 0;

    #ifdef DEBUG
    bool kWriteDot = false;
    unsigned int kMaxDept = -1;
    std::vector<unsigned int> identity(probabilities.size(),0);
    std::string filename = stringf("debug.pwpbdd.%u.dot", kTier);
    #endif

    StackTop i;
    StackTopInit(i);
    Stack &s = cache.GetStack(gThreadId);
    Push(s,i,kRootIndex,kCircuit,kEvidenceList,kConditionTierList,kTier);

    traverse: {
        const unsigned int &kParentIndex = s[i-2];
        const unsigned int &kIndex = s[i-1];
        probability_t &probability = probabilities[kIndex];

        #ifdef DEBUG
        if(kWriteDot)
            WriteDot(filename, kCircuit, kEvidenceList, probabilities, identity, kTier, kIndex,kMaxDept,i,&s);
        #endif

        if(probability == kNotTraversed){
            #ifdef DEBUG
            if(i >= s.size()-1){
                WriteDot(filename, kCircuit, kEvidenceList, probabilities, identity, kTier, kIndex,kMaxDept,i,&s);
                throw ModelCounterException("Custom stack is going out of bounds");
            }
            #endif
            probability = 0;
            Push(s,i,kIndex,kCircuit,kEvidenceList,kConditionTierList,kTier);

            goto traverse;
        }

        const Partition::Node &kParentNode = kCircuit[kParentIndex];
        const Partition::Node &kNode = kCircuit[kIndex];
        if(kParentNode.v != kNode.v){
            // this is a t child
            probabilities[kParentIndex] += kParentNode.w * probability;
        } else {
            // this is an e child
            probabilities[kParentIndex] += probability;
        }

        #ifdef DEBUG
        if(kWriteDot)
            WriteDot(filename, kCircuit, kEvidenceList, probabilities, identity, kTier, kIndex,kMaxDept,i,&s);
        #endif

        Pop(i);
        if(!Empty(i))
            goto traverse;
    }

    return probabilities[kRootIndex];
}

namespace bnmc_6_ {


void pthread_traverse(const Architecture &kArchitecture, Cache &cache, const ConditionTierList &kConditionTierList, const TierId kTier, const NodeIndex kNodeId){
    EvidenceList &evidence_list = cache.GetEvidenceList(kNodeId, kTier);
        Probability &probability = cache.GetCacheEntry(kNodeId, kTier);
        if(kTier < kArchitecture.Size())
            probability = gMc->ParallelTraversePartition<N>(kArchitecture, cache, evidence_list, kConditionTierList, kTier, kNodeId);
        else probability = gMc->ParallelTraverse<N>(kArchitecture, cache, evidence_list, kConditionTierList, kTier, kNodeId);
}

void* pthread_worker(void *in){
    const unsigned int kThreadId = (size_t)in;
    gThreadId = kThreadId;

    const Architecture &kArchitecture = *gArchitecture;
    const size_t kLastTier = kArchitecture.Size()-1;

    Cache &cache = *gCache;
    Cache &cache2 = *gCache2;
    const ConditionTierList &kConditionTierList  = *gConditionTierList;
    const ConditionTierList &kConditionTierList2 = *gConditionTierList2;

    auto &queue = task_queue[kThreadId];
    auto &scheduler_queue = tasks_queue;

    pthread_barrier_wait(&start_barrier);

    while(true){
        TaskItem task;

        while(!queue.pop(task));
        //printf("[%u] - %u\n", kThreadId, task.node);
        if(task.node == kTasksFinished){
            printf("[%u] done!\n", kThreadId);
            return NULL;
        }

        const TierId &kTier = task.tier;
        const unsigned int &kNodeId = task.node;

        if(task.with_query_variable){
            // traverse partition
            pthread_traverse(kArchitecture, cache, kConditionTierList,task.tier, task.node);

            if(task.tier == 0 /* && task.node == 0 */){
                printf("[%u] (%u,%u,%u)\n", kThreadId, kTier, kNodeId, task.with_query_variable);
                scheduler_queue.enqueue({kTasksFinished,task.with_query_variable});
            } else {
                const unsigned int kTaskGroupId = cache.GetTaskGroupId(task.tier,task.node);
                TaskCounter& counter = cache.GetTaskCounter(kTaskGroupId);

                std::string dependents = stringf("%u: ",cache.task_reverse_cache_[kTaskGroupId].nodes.size());;
                for(auto it = cache.task_reverse_cache_[kTaskGroupId].nodes.begin(); it != cache.task_reverse_cache_[kTaskGroupId].nodes.end(); it++)
                    dependents += stringf("(%u,%u) ", cache.task_reverse_cache_[kTaskGroupId].tier, *it);

                printf("[%u] (%u,%u,%u) -> %u [ %s]\n", kThreadId, kTier, kNodeId, task.with_query_variable, kTaskGroupId,dependents.c_str());
                const unsigned int kDependencies = --counter;
                if(kDependencies == 0)
                    scheduler_queue.enqueue({kTaskGroupId,task.with_query_variable});
            }
        } else {
            // traverse partition
            pthread_traverse(kArchitecture, cache2, kConditionTierList2,task.tier, task.node);

            if(task.tier == 0 /* && task.node == 0 */){
                printf("[%u] (%u,%u,%u)\n", kThreadId, kTier, kNodeId, task.with_query_variable);
                scheduler_queue.enqueue({kTasksFinished,task.with_query_variable});
            } else {
                const unsigned int kTaskGroupId = cache2.GetTaskGroupId(task.tier,task.node);
                TaskCounter& counter = cache2.GetTaskCounter(kTaskGroupId);

                std::string dependents = stringf("%u: ",cache.task_reverse_cache_[kTaskGroupId].nodes.size());;
                for(auto it = cache2.task_reverse_cache_[kTaskGroupId].nodes.begin(); it != cache2.task_reverse_cache_[kTaskGroupId].nodes.end(); it++)
                    dependents += stringf("(%u,%u) ", cache2.task_reverse_cache_[kTaskGroupId].tier, *it);

                printf("[%u] (%u,%u,%u) -> %u [ %s]\n", kThreadId, kTier, kNodeId, task.with_query_variable, kTaskGroupId,dependents.c_str());

                const unsigned int kDependencies = --counter;
                if(kDependencies == 0)
                    scheduler_queue.enqueue({kTaskGroupId,task.with_query_variable});
            }
        }
    }
}

void pthread_scheduler(Timer *t){
    const unsigned int kMaxThreadId = gNrThreads-1;
    const Architecture &kArchitecture = *gArchitecture;
    const size_t kLastTier = kArchitecture.Size()-1;
    const Cache &kCache = *gCache;
    const Cache &kCache2 = *gCache2;

    unsigned int thread_id = 0;
    bool tasks_incomplete = true, tasks2_incomplete = gHasQueryVariable;


    while (tasks_incomplete || tasks2_incomplete){
        TasksItem item;
        //tasks_queue.wait_dequeue(item);
        while(!tasks_queue.try_dequeue(item));
        printf("[R:%u] %u\n",item.with_query_variable,item.task_group_id);

        // round robin tasks assignment
        if(item.with_query_variable){
            if(item.task_group_id == kTasksFinished)
                tasks_incomplete = false;
            else {
                const TaskGroup &kTaskGroup = kCache.GetTaskGroup(item.task_group_id);
                for(auto it = kTaskGroup.nodes.begin(); it != kTaskGroup.nodes.end(); it++){
                    printf("[R] (%u,%u,%u) -> [%u]\n", *it, kTaskGroup.tier, item.with_query_variable, thread_id);
                    while(!task_queue[thread_id].push({*it,kTaskGroup.tier,item.with_query_variable}));
                    thread_id = (thread_id==kMaxThreadId?0:thread_id+1);
                }
            }
        } else {
            if(item.task_group_id == kTasksFinished)
                tasks2_incomplete = false;
            else {
                const TaskGroup &kTaskGroup = kCache2.GetTaskGroup(item.task_group_id);
                for(auto it = kTaskGroup.nodes.begin(); it != kTaskGroup.nodes.end(); it++){
                    printf("[R] (%u,%u,%u) -> [%u]\n", *it, kTaskGroup.tier, item.with_query_variable, thread_id);
                    while(!task_queue[thread_id].push({*it,kTaskGroup.tier,item.with_query_variable}));
                    thread_id = (thread_id==kMaxThreadId?0:thread_id+1);
                }
            }
        }
    }

    //printf("cleanup!\n");

    for(unsigned int i = 0; i < gNrThreads; i++)
        task_queue[i].push({kTasksFinished,0,true});

    //printf("cleanup done!\n");
    // cleanup



}


} // namespace bnmc_6_

template <>
template <>
probability_t ModelCounter<ModelType::PWPBDD>::ParallelPosterior<N>(const unsigned int kWorkers, Timer* t){
    // globals
    gArchitecture = &architecture_;
    gMc = this;
    gEvidenceList = &evidence_list_;
    gEvidenceList = &evidence_list2_;
    gCache = &cache_;
    gCache2 = &cache2_;
    gConditionTierList = &condition_tier_;
    gConditionTierList2 = &condition_tier2_;
    gHasQueryVariable = has_query_variable_;

    static unsigned int query_number = 1;

    // set task list
    cache_.SetTasks(architecture_, condition_tier_, evidence_list_);
    assert(cache_.GetTaskCount() < UINT16_MAX);
    if(gHasQueryVariable){
        cache2_.SetTasks(architecture_, condition_tier2_, evidence_list2_);
        assert(cache2_.GetTaskCount() < UINT16_MAX);
    }

    // determine number of worker threads
    gNrThreads = kWorkers;
    const unsigned int kMaxThreads = std::thread::hardware_concurrency()-1;
    if(gNrThreads == 0 || gNrThreads > kMaxThreads)
        gNrThreads = kMaxThreads;

    if(gNrThreads > MAX_THREADS)
        gNrThreads = MAX_THREADS-1;


    pthread_barrier_init(&start_barrier, NULL, gNrThreads+1);
    pthread_barrier_init(&barrier, NULL, gNrThreads);

    // start threads
    std::array<pthread_t,MAX_THREADS> threads;
    for(unsigned int i = 0; i < gNrThreads; i++){
        pthread_t &thread = threads[i];
        if(pthread_create(&thread, NULL, pthread_worker, (void*)i)){
            fprintf(stderr, "Error creating thread %u of %u\n", i+1, gNrThreads);
            exit(1);
        }
    }

    pthread_barrier_wait(&start_barrier);

    // set initial tasks
    tasks_queue.enqueue({0,true});
    if(gHasQueryVariable)
        tasks_queue.enqueue({0,false});


    if(t) t->Start();
    pthread_scheduler(t);

    for(unsigned int i = 0; i < gNrThreads; i++){
        if(pthread_join(threads[i],NULL)){
            fprintf(stderr, "Error canceling thread %u of %u\n", i+1, gNrThreads);
            exit(1);
        }
    }
    if(t) {
        t->Stop();
        t->Add();
    }

    pthread_barrier_destroy(&start_barrier);
    pthread_barrier_destroy(&barrier);

    probability_t probability_with_query_variable = cache_.GetCacheEntry(0,0);
    #ifdef DEBUG
    QueryProbabilities &probs = manager.probabilities["PPWPBDD_6_"];
    probs.pq = probability_with_query_variable;
    probs.p = 1;
    #endif

    if(gHasQueryVariable) {
        probability_t probability_without_query_variable = cache2_.GetCacheEntry(0,0);
        #ifdef DEBUG
        probs.p = probability_without_query_variable;
        #endif

        if(probability_without_query_variable == 0)
            return -1;
        else return probability_with_query_variable/probability_without_query_variable;
    } else return cache_.GetCacheEntry(0,0);
}

} // namespace bnmc

