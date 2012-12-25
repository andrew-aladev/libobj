#include "obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>

#define alen(a) (sizeof(a)/sizeof(*a))

#define push(ptr, a, size) { \
    a = realloc(a, (++size) * sizeof(*a)); \
    memcpy(&a[size-1], ptr, sizeof(*a)); \
    free(ptr); \
}

static int strsplit(const char *str, char d, char **segs, unsigned int *segs_size, unsigned int *slashes) {
    unsigned int i;
    unsigned int str_size = strlen(str);    
    
    *segs_size = 0;
    *slashes = 0;
    segs[*segs_size++] = &str[0];    
    for(i = 0; i < str_size; i++) {
        if(str[i] == d) {
            *slashes++;
            segs[*segs_size++] = &str[i+1]; 
        }            
    }
    
    return 0;
}

static void read_word(char *buf, unsigned int bufsize, const char *str) {    
    unsigned int i;
    unsigned int str_size = strlen(str);    
    
    for(;; str++) {
        if(*str == '\0') {
            buf[0] = '\0';    
            return;
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
}

static int to_float(char *str, float *v) {
    char *endptr;
    *v = strtof(str, &endptr);
    return endptr - str != strlen(str) ? -1 : 0;
}

static int to_int(char *str, int *v) {
    long _v;
    char *endptr;
    _v = strtol(str, &endptr, 10);
    if(endptr - str != strlen(str))
        return -1;
    if(_v < INT_MIN || _v > INT_MAX)
        return -1;
        
    *v = (int)_v;
    return 0;
}

static int read_floats(const char *str, float **buf, unsigned int size) {
    unsigned int w;

    *buf = malloc(sizeof(**buf) * size);
    for(w = 0; w < size; w++) {
        char word[32];
        read_word(word, alen(word), str);

        if(strlen(word) == 0)
            break;
        if(to_float(word, &(*buf)[w]))
            return -1;
    }

    return w;
}

static int read_v(const char *str, float **v) {
    *v = malloc(4 * sizeof(**v));
    unsigned int size = read_floats(str, v, 4);
    assert(size >= 3);
    (*v)[3] = size >= 4 ? (*v)[3] : 1.0;

    return 0;
}

static int read_vn(const char *str, float **vn) {
    *vn = malloc(3 * sizeof(**vn));

    unsigned int size = read_floats(str, vn, 3);
    assert(size >= 3);

    return 0;
}

static int read_vt(const char *str, float **vt) {
    *vt = malloc(3 * sizeof(**vt));
    
    unsigned int size = read_floats(str, vt, 3);
    assert(size >= 2);
    (*vt)[2] = size >= 3 ? (*vt)[2] : 0.0;

    return 0;
}

static fv_t *read_fv(const char *str) {
    int seg_size;
    int slashes;
    fv_t _fv;
    fv_t *fv;
    char *segs[3];

    fv = malloc(sizeof(*fv));
    strsplit(str, '/', segs, &seg_size, &slashes);

    assert(seg_size >= 1);
    assert(!to_int(segs[0], &fv->v));
    fv->vt = -1;
    fv->vn = -1;

    switch(slashes) {
        case 0: break; // v
        case 1: // v/vt
            assert(seg_size >= 2);
            assert(!to_int(segs[1], &fv->vt));
        break;
        case 2:
            switch(seg_size) {
                case 2: // v//vn
                    assert(seg_size >= 2);
                    assert(!to_int(segs[1], &fv->vn));
                break;
                case 3:  // v/vt/vn
                    assert(seg_size >= 3);
                    assert(!to_int(segs[1], &fv->vt));
                    assert(!to_int(segs[2], &fv->vn));
                break;
                default:
                    assert(0);
            }
        break;
        default:
            assert(0);
    }

    return fv;
}

static f_t *read_f(const char *str) {
    f_t *f;
    unsigned int i;

    f = malloc(sizeof(*f));
    f->fv_size = 0;
    f->fv = NULL;

    for(i = 0;; i++) {
        char word[32];
        read_word(word, alen(word), str);
        if(strlen(word) == 0)
            break;

        fv_t *fv = read_fv(word);
        if(fv == NULL)
            continue;

        push(fv, f->fv, f->fv_size);
    }

    return f;
}


static void read_line(FILE *f, obj_t *obj) {
    char line[512];
    char s[16];

    if(fgets(line, alen(line), f) == NULL) {
        return;
    }
    assert(strlen(line) < alen(line) || feof(f));
    read_word(s, alen(s), line);

    if(strcmp(s, "#") == 0) {
        return;
    }
    else if(strcmp(s, "v") == 0) {
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
    else {
        assert(0);
    }
}


obj_t *obj_load(const char *path) {
    obj_t *obj;
    FILE *f;

    f = fopen(path, "r");
    if(f == NULL) {
        fprintf(stderr, "Couldn't open .obj: %s\n", strerror(errno));
        return NULL;
    }

    obj = malloc(sizeof(*obj));
    while(!feof(f)) {
        read_line(f, obj);
    }

    fclose(f);
    return obj;
}

void obj_free(obj_t *obj) {

}
