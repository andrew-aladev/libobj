#include "obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stderr.h>

#define alen(a) (sizeof(a)/sizeof(*a))

typedef struct {
    char *str;
    int size;
} buf_t;


static void push(void *ptr, void **a, void *size) {
    *a = realloc(*a, ++size);
    (*a)[size-1] = ptr;
}

static void read_word(cur_t *c) {

}

static int to_float(char *str, float *v) {

}

static int to_int(char *str, int *v) {

}

static int read_floats(buf_t *b, float **buf, unsigned int size) {
    unsigned int w;

    *buf = malloc(sizeof(**buf) * size);
    for(w = 0; w < size; w++) {
        char word[32];
        read_word(word, b);

        if(strlen(word) == 0)
            break;
        if(to_float(word, &(*buf)[w]))
            return -1;
    }

    return w;
}

static v_t *read_v(buf_t *b) {
    v_t *v;

    unsigned int size = read_floats(b, &v, alen(v));
    assert(size >= 3);
    v[3] = size >= 4 ? v[3] : 1.0;

    return v;
}

static vn_t *read_vn(buf_t *b) {
    vn_t *vn;

    unsigned int size = read_floats(b, &vn, alen(vn));
    assert(size >= 3);

    return vn;
}

static vt_t *read_vt(buf_t *b) {
    vt_t *vt;

    unsigned int size = read_floats(b, &vt, alen(vt));
    assert(size >= 2);
    vt[2] = size >= 3 ? vt[2] : 0.0;

    return vt;
}

static fv_t *read_fv(const char *str) {
    int seg_size;
    int slashes;
    fv_t _fv;
    fv_t *fv;
    char **seg;

    fv = malloc(sizeof(*fv));
    seg = strsplit(str, '/', &seg_size, &slashes);

    assert(seg_size >= 1);
    fv->v = to_int(seg[0]);
    fv->vt = -1;
    fv->vn = -1;

    switch(slashes) {
        case 0: break; // v
        case 1: // v/vt
            assert(seg_size >= 2);
            assert(!to_int(seg[1], &fv->vt));
        break;
        case 2:
            switch(seg_size) {
                case 2: // v//vn
                    assert(seg_size >= 2);
                    assert(!to_int(seg[1], &fv->vn));
                break;
                case 3:  // v/vt/vn
                    assert(seg_size >= 3);
                    assert(!to_int(seg[1], &fv->vt));
                    assert(!to_int(seg[2], &fv->vn));
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

static f_t *read_f(buf_t *b) {
    f_t *f;
    unsigned int i;

    f = malloc(sizeof(*f));
    f->fv_size = 0;
    f->fv = NULL;

    for(i = 0;; i++) {
        char word[32];
        read_word(word, b);
        if(strlen(word == 0))
            break;

        fv_t *fv = read_fv(word);
        if(fv == NULL)
            continue;

        push(fv, &f->fv, &f->fv_size);
    }

    return f;
}


static void read_line(FILE *f, obj_t *obj) {
    char line[512];
    char s[16];
    cur_t c = {line, alen(line)};

    c = line;
    if(fgets(line, alen(line), f) == NULL) {
        return NULL;
    }
    assert(strlen(line) < alen(line) || feof(f));
    read_word(s, alen(s), &c);

    if(strcmp(s, "#") == 0) {
        return;
    }
    else if(strcmp(s, "v") == 0) {
        v_t *v = read_v(&c);
        push(v, &obj->v, &obj->v_size);
    }
    else if(strcmp(s, "vn") == 0) {
        vn_t *vn = read_vn(&c);
        push(v, &obj->vn, &obj->vn_size);
    }
    else if(strcmp(s, "vn") == 0) {
        vn_t *vn = read_vn(&c);
        push(v, &obj->vt, &obj->vt_size);
    }
    else if(strcmp(s, "f") == 0) {
        f_t *f = read_f(&c);
        push(v, &obj->f, &obj->f_size);
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
