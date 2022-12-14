#ifndef MISC_H
#define MISC_H

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

#include <string.h>
#include <string>
#include <bn-to-cnf/cnf.h>

std::string exec(const char*);
const char* get_bayesnet_filename(bayesnet *bn);
const char* get_bayesnet_filename(cnf *f);

#ifdef DEBUG
    #define pthread_check(...) {                                                            \
        int ret = __VA_ARGS__;                                                              \
        if(ret)                                                                            \
            fprintf(stderr,"PTHREAD error '%s' at %s:%d\n", strerror(ret), __FILE__, __LINE__);   \
    }
#else
    #define pthread_check(...) __VA_ARGS__
#endif

// int main()
// {
//   printf(KRED "red\n" RESET);
//   printf(KGRN "green\n" RESET);
//   printf(KYEL "yellow\n" RESET);
//   printf(KBLU "blue\n" RESET);
//   printf(KMAG "magenta\n" RESET);
//   printf(KCYN "cyan\n" RESET);
//   printf(KWHT "white\n" RESET);
//   printf(KNRM "normal\n" RESET);
//
//   return 0;
// }

#endif
