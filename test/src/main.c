#include "obj.h"
#include "stdio.h"

int main() {
    unsigned int f, fv, v, vt, vn;
    fprintf(stderr, "-------- Testing libobj -------\n");
    obj_t *o = obj_load("cornellbox.obj");

    fprintf(stderr, "-------- Done loading --------\n");

    printf("Vertexes:\n");
    for(v = 0; v < o->v_size; v++) {
        unsigned int i;
        printf("  ");
        for(i = 0; i < 4; i++)
            printf("%f ", o->v[v][i]);
        printf("\n");
    }

    printf("Texture-Coordinates:\n");
    for(vt = 0; vt < o->vt_size; vt++) {
        unsigned int i;
        printf("  ");
        for(i = 0; i < 3; i++)
            printf("%f ", o->vt[vt][i]);
        printf("\n");
    }

    printf("Vertex-Normals:\n");
    for(vn = 0; vn < o->vn_size; vn++) {
        unsigned int i;
        printf("  ");
        for(i = 0; i < 3; i++)
            printf("%f ", o->vn[vn][i]);
        printf("\n");
    }

    printf("Faces:\n");
    for(f = 0; f < o->f_size; f++) {
        unsigned int i;
        printf("  ");
        for(fv = 0; fv < o->f[f].fv_size; fv++) {
            for(i = 0; i < 3; i++) {
                printf("%i", o->f[f].fv[fv][i]);
                if(i<2)
                    printf("/");
            }
            printf(" ");
        }
        printf("\n");
    }
    return 0;
}
