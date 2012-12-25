#ifndef OBJ_H
#define OBJ_H



    typedef struct {
        unsigned int fv_size;
        int (*fv)[3]; // v/vt/vn
    } f_t;

    typedef struct {
        unsigned int f_size;
        unsigned int v_size;
        unsigned int vt_size;
        unsigned int vn_size;
        f_t *f;
        float (*v)[4];
        float (*vt)[3];
        float (*vn)[3];
    } obj_t;


    obj_t *obj_new();
    obj_t *obj_load(const char *path);
    void obj_free(obj_t *obj);


#endif // OBJ_H
