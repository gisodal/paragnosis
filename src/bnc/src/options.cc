#include "options.h"

namespace bnc {

filename_t files;
float OPT_COMPUTED_TABLE_LOAD_FACTOR;
size_t OPT_COMPUTED_TABLE_BUCKETS;
int OPT_LOOKAHEAD;
int OPT_TIME_LIMIT;
int OPT_RESERVE;
int OPT_ORDER;
bdd_t OPT_BDD_TYPE;
compilation_t OPT_COMPILATION_TYPE;
bool
    OPT_USE_PROBABILITY,
    OPT_DETERMINISM,
    OPT_ENCODE_STRUCTURE,
    OPT_PARTITION,
    OPT_READ_PARTITION,
    OPT_READ_PSEUDO_ORDERING,
    OPT_READ_ORDERING,
    OPT_READ_VARIABLE_ORDERING,
    OPT_READ_ELIM_ORDERING,
    OPT_SA_READ_ELIM_ORDERING,
    OPT_WRITE_BINARY,
    OPT_WRITE_PARTITION,
    OPT_WRITE_STATS,
    OPT_WRITE_ORDERING,
    OPT_WRITE_PSEUDO_ORDERING,
    OPT_WRITE_ELIM_ORDERING,
    OPT_WRITE_COMPOSITION_ORDERING,
    OPT_WRITE_VARIABLE_ORDERING,
    OPT_WRITE_DOT,
    OPT_WRITE_SPANNING,
    OPT_WRITE_PS,
    OPT_WRITE_CNF,
    OPT_SA_PARALLELISM,
    OPT_WRITE_AC,
    OPT_WRITE_UAI,
    OPT_ORDER_POST_WEIGHT,
    OPT_WRITE_MAPPING,
    OPT_NODE_LEVEL_COMPILATION,
    OPT_COLLAPSE,
    OPT_PARALLELISM,
    OPT_PARALLEL_CONJOIN,
    OPT_PARALLEL_CUBE,
    OPT_PARALLEL_PARTITION,
    OPT_INFO,
    OPT_NO_COMPILE,
    OPT_NO_ORDERING,
    OPT_SA_PRINT_ORDERING,
    OPT_TOPDOWN_COMPILATION,
    OPT_BEST_COMPOSITION_ORDERING,
    OPT_SHOW_SCORE;

unsigned int OPT_PARALLEL_LEVEL;
int OPT_PARALLEL_CPT;
int OPT_NR_PARTITIONS;
int OPT_SA_ITERATIONS;
int OPT_SA_TRIES;
unsigned int OPT_SA_RUNS;
double OPT_SA_TEMPERATURE_INITIAL;
double OPT_SA_TEMPERATURE_MIN;
double OPT_SA_TEMPERATURE_DAMP_FACTOR;
double OPT_SA_SCORE_RATIO;
unsigned int OPT_WORKERS;

void init_options(){
    OPT_PARTITION =
    OPT_READ_PARTITION =
    OPT_READ_PSEUDO_ORDERING =
    OPT_READ_ORDERING =
    OPT_READ_VARIABLE_ORDERING =
    OPT_READ_ELIM_ORDERING =
    OPT_SA_READ_ELIM_ORDERING =
    OPT_WRITE_PARTITION =
    OPT_WRITE_STATS =
    OPT_WRITE_PSEUDO_ORDERING =
    OPT_WRITE_ORDERING =
    OPT_WRITE_ELIM_ORDERING =
    OPT_WRITE_COMPOSITION_ORDERING =
    OPT_WRITE_VARIABLE_ORDERING =
    OPT_WRITE_DOT =
    OPT_WRITE_SPANNING =
    OPT_WRITE_PS =
    OPT_WRITE_UAI =
    OPT_WRITE_CNF =
    OPT_WRITE_AC =
    OPT_WRITE_MAPPING =
    OPT_ORDER_POST_WEIGHT =
    OPT_SHOW_SCORE =
    OPT_PARALLELISM =
    OPT_SA_PARALLELISM =
    OPT_INFO =
    OPT_NO_COMPILE =
    OPT_NO_ORDERING =
    OPT_SA_PRINT_ORDERING =
    OPT_TOPDOWN_COMPILATION = false;

    OPT_USE_PROBABILITY =
    OPT_WRITE_BINARY =
    OPT_COLLAPSE =
    OPT_NODE_LEVEL_COMPILATION =
    OPT_PARALLEL_CONJOIN =
    OPT_PARALLEL_CUBE =
    OPT_PARALLEL_PARTITION =
    OPT_DETERMINISM =
    OPT_ENCODE_STRUCTURE =
    OPT_BEST_COMPOSITION_ORDERING = true;
    OPT_PARALLEL_CPT = 1;
    OPT_TIME_LIMIT = -1;
    OPT_NR_PARTITIONS = -1;
    OPT_RESERVE = 0;
    OPT_ORDER = -1;
    OPT_LOOKAHEAD = 3;
    OPT_BDD_TYPE = bdd_t::tdmultigraph;
    OPT_COMPILATION_TYPE = compilation_t::topdown_bottomup;
    OPT_SA_ITERATIONS = 100;
    OPT_SA_TRIES = 100;
    OPT_SA_RUNS = 1;
    OPT_SA_SCORE_RATIO = 0.6;
    OPT_SA_TEMPERATURE_INITIAL = 5000;
    OPT_SA_TEMPERATURE_MIN = 5.0e-1;
    OPT_SA_TEMPERATURE_DAMP_FACTOR = 1.01;
    OPT_COMPUTED_TABLE_BUCKETS = 1024;
    OPT_COMPUTED_TABLE_LOAD_FACTOR = 1.0;
    OPT_WORKERS = 0; // 0 = auto determine, 1 = disable parallelism
    OPT_PARALLEL_LEVEL = 3;
}

}

