extern char *optarg;
extern int optind, opterr;

#if defined(__STDC__)
int getopt (int argc, char **argv, const char *options);
#else
int getopt ();
#endif
