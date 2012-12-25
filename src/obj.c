#include "obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>

#define alen(a) (sizeof(a)/sizeof(*a))

#define push(ptr, a, size) { \
    a = realloc(a, (++size) * sizeof(*a)); \
    memcpy(&a[size-1], ptr, sizeof(*a)); \
    free(ptr); \
}

static int strsplit(const char *str, char d, const char **segs, unsigned int *segs_size, unsigned int *slashes) {
    unsigned int i;
    unsigned int str_size = strlen(str);

    *segs_size = 0;
    *slashes = 0;
    segs[(*segs_size)++] = &str[0];
    for(i = 0; i < str_size; i++) {
        if(str[i] == d) {
            (*slashes)++;
            if(str[i+1] != d && str[i+1] != '\0')
                segs[(*segs_size)++] = &str[i+1];
        }
    }

    return 0;
}

static char *read_word(char *buf, unsigned int bufsize, const char *str) {
    unsigned int i;
    unsigned int str_size = strlen(str);

    for(;; str++) {
        if(*str == '\0') {
            buf[0] = '\0';
            return NULL;
        }
        if(isprint(*str) && !isspace(*str))
            break;
    }

    for(i = 0; i < str_size-1 && i < bufsize-1; i++) {
        if(isprint(str[i]) && !isspace(str[i]))
            buf[i] = str[i];
        else
            break;
    }
    buf[i] = '\0';
    return &str[i];
}

static int to_float(const char *str, float *v) {
    char *endptr;
    *v = strtof(str, &endptr);
    return endptr - str == 0 ? -1 : 0;
}

static int to_int(const char *str, int *v) {
    long _v;
    char *endptr;
    _v = strtol(str, &endptr, 10);
    if(endptr - str == 0)
        return -1;
    if(_v < INT_MIN || _v > INT_MAX)
        return -1;

    *v = (int)_v;
    return 0;
}

static int read_floats(const char *str, float *buf, unsigned int size) {
    unsigned int w;

    for(w = 0; w < size; w++) {
        char word[32];
        str = read_word(word, alen(word), str);

        if(strlen(word) == 0)
            break;
        if(to_float(word, &buf[w]))
            return -1;
        if(str == NULL)
            break;
    }

    return w;
}

static int read_v(const char *str, float **v) {
    *v = malloc(4 * sizeof(**v));

    unsigned int size = read_floats(str, *v, 4);
    assert(size >= 3);
    (*v)[3] = size >= 4 ? (*v)[3] : 1.0;

    return 0;
}

static int read_vn(const char *str, float **vn) {
    *vn = malloc(3 * sizeof(**vn));

    unsigned int size = read_floats(str, *vn, 3);
    assert(size >= 3);

    return 0;
}

static int read_vt(const char *str, float **vt) {
    *vt = malloc(3 * sizeof(**vt));

    unsigned int size = read_floats(str, *vt, 3);
    assert(size >= 1);
    (*vt)[1] = size >= 2 ? (*vt)[1] : 0.0;
    (*vt)[2] = size >= 3 ? (*vt)[2] : 0.0;

    return 0;
}

static int read_fv(const char *str, int fv[3]) {
    unsigned int segs_size;
    unsigned int slashes;
    const char *segs[3];

    strsplit(str, '/', segs, &segs_size, &slashes);

    assert(segs_size >= 1);
    assert(!to_int(segs[0], &fv[0]));
    fv[1] = 0;
    fv[2] = 0;


    switch(slashes) {
        case 0: break; // v
        case 1: // v/vt
            assert(segs_size >= 2);
            assert(!to_int(segs[1], &fv[1]));
        break;
        case 2:
            switch(segs_size) {
                case 2: // v//vn
                    assert(segs_size >= 2);
                    assert(!to_int(segs[1], &fv[2]));
                break;
                case 3:  // v/vt/vn
                    assert(segs_size >= 3);
                    assert(!to_int(segs[1], &fv[1]));
                    assert(!to_int(segs[2], &fv[2]));
                break;
                default:
                    assert(0);
            }
        break;
        default:
            assert(0);
    }

    return 0;
}

static f_t *read_f(const char *str) {
    f_t *f;
    unsigned int i;

    f = malloc(sizeof(*f));
    f->fv_size = 0;
    f->fv = NULL;

    for(i = 0;; i++) {
        int fv[3];
        char word[32];
        read_word(word, alen(word), str);
        if(strlen(word) == 0)
            break;

        if(read_fv(word, fv))
            continue;

        f->fv = realloc(f->fv, (++f->fv_size) * sizeof(*f->fv));
        memcpy(&f->fv[f->fv_size-1], fv, sizeof(*f->fv));

        str += strlen(word) + 1;
        if(*str == '\0')
            break;
    }

    return f;
}

static int ignore_line(const char *line) {
    char s[16];
    read_word(s, alen(s), line);

    if(strlen(s) == 0 || strcmp(s, "#") == 0)
        return 1;

    return 0;
}

static int read_line(const char *line, obj_t *obj) {
    char s[16];

    if(ignore_line(line))
        return 0;

    read_word(s, alen(s), line);

    if(strcmp(s, "v") == 0) {
        float *v;
        read_v(&line[1], &v);
        push(v, obj->v, obj->v_size);
    }
    else if(strcmp(s, "vn") == 0) {
        float *vn;
        read_vn(&line[2], &vn);
        push(vn, obj->vn, obj->vn_size);
    }
    else if(strcmp(s, "vt") == 0) {
        float *vt;
        read_vt(&line[2], &vt);
        push(vt, obj->vt, obj->vt_size);
    }
    else if(strcmp(s, "f") == 0) {
        f_t *f = read_f(&line[1]);
        push(f, obj->f, obj->f_size);
    }
    else if(strcmp(s, "g") == 0) {
        fprintf(stderr, "Ignored group directive\n");
    }
    else {
        assert(0);
    }

    return -1;
}

static int load_lines(FILE *file, char ***bufs, unsigned int *lines_size) {
    char line[512];

    *bufs = NULL;
    *lines_size = 0;
    while(!feof(file)) {
        if(fgets(line, alen(line), file) == NULL) {
            if(ferror(file))
                return -1;
            else
                return 0;
        }
        assert(strlen(line) < alen(line) || feof(file));

        *bufs = realloc(*bufs, 2012);
        (*bufs)[*lines_size] = malloc(2012);
        strcpy((*bufs)[*lines_size], line);
        (*lines_size)++;
    }

    return 0;
}

obj_t *obj_new() {
    obj_t *obj = malloc(sizeof(*obj));
    obj->v  = NULL;
    obj->vt = NULL;
    obj->vn = NULL;
    obj->f  = NULL;
    obj->v_size  = 0;
    obj->vt_size = 0;
    obj->vn_size = 0;
    obj->f_size  = 0;
    return obj;
}

obj_t *obj_load(const char *path) {
    obj_t *obj;
    char **lines;
    unsigned int l;
    unsigned int lines_size;
    FILE *f;

    f = fopen(path, "r");
    if(f == NULL) {
        fprintf(stderr, "Couldn't open .obj: %s\n", strerror(errno));
        return NULL;
    }

    if(load_lines(f, &lines, &lines_size)) {
        fprintf(stderr, "Error loading .obj-file: %s.\n", strerror(errno));
        return NULL;
    }
    fclose(f);


    obj = obj_new();
    for(l = 0; l < lines_size; l++) {
        read_line(lines[l], obj);
    }

    return obj;
}

void obj_free(obj_t *obj) {

}
