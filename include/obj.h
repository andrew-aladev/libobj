#ifndef OBJ_H
#define OBJ_H

    typedef float[4] v_t;
    typedef float[3] vn_t;
    typedef float[3] vt_t;

    typedef struct {
        int v, vt, vn;
    } fv_t;

    typedef struct {
        unsigned int fv_size;
        fv_t *fv;
    } f_t;

    typedef struct {
        unsigned int f_size;
        unsigned int v_size;
        unsigned int vt_size;
        unsigned int vn_size;
        f_t *f;
        v_t *v;
        vt_t *vt;
        vn_t *vn;
    } obj_t;


    obj_t *obj_load(const char *path);
    void obj_free(obj_t *obj);


#endif // OBJ_H
