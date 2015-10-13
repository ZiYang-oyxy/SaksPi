#include "common.h"

void tokensfree(char **tokens, size_t numtokens)
{
    size_t i;
    if (tokens) {
        for (i = 0; i < numtokens; i++) {
            free(tokens[i]);
            tokens[i] = NULL;
        }

        free(tokens);
    }
}

char **strsplit(const char* str, const char* delim, size_t* numtokens)
{
    char *s = strdup(str);
    size_t tokens_alloc = 1;
    size_t tokens_used = 0;
    char **tokens = calloc(tokens_alloc, sizeof(char*));
    char *token, *rest = s;

    while ((token = strsep(&rest, delim)) != NULL) {
        if (tokens_used == tokens_alloc) {
            tokens_alloc *= 2;
            /* FIXME: (error) Common realloc mistake: 'tokens' nulled but not freed upon failure */
            tokens = realloc(tokens, tokens_alloc * sizeof(char*));
        }
        tokens[tokens_used++] = strdup(token);
    }

    if (tokens_used == 0) {
        free(tokens);
        tokens = NULL;
    } else {
        /* FIXME: (error) Common realloc mistake: 'tokens' nulled but not freed upon failure */
        tokens = realloc(tokens, tokens_used * sizeof(char*));
    }

    *numtokens = tokens_used;
    free(s);
    return tokens;
}

