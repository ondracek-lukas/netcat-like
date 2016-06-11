// All symbols declared here are defined in the main module

// Prints an error and immediately exits the app.
// It can be called from every module.
extern void exitErr(int exitcode, char *msg);

// Verbosity level.
extern int verbosity;

// Prints to stderr if verbosity >= level
extern void printLog(int level, const char *fmt, ...);

// Prints to stdout if verbosity >= level
extern void printOut(int level, const char *fmt, ...);
