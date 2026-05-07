
#include <stdlib.h>
#include <stdio.h>
#include <omp.h>

typedef struct{
    int* data;
    int start;
    int true_wid;
    int wid;
    int hei;
} Mat;

#define mat_at(m,x,y) ((m).data[(m).start + (x) + (y)*(m).true_wid])

int sum_mat(Mat mat){
    int acc = 0;
    #pragma omp 
    for(int y = 0 ; y < mat.hei ; ++y){
        for(int x = 0 ; x < mat.wid ; ++x){
            acc += mat_at(mat,x,y);
        }
    }
    return acc;
}

Mat sub_mat(Mat mat,int x0,int y0,int x1,int y1){
    Mat subm = mat;
    subm.hei=y1-y0 +1;
    subm.wid=x1-x0 +1;
    subm.start = mat.start + x0 + y0*mat.true_wid;
    return subm;
}

void initMat(Mat *mat, int wid, int hei){
    mat->data = (int*)calloc(wid*hei,sizeof(int));
    mat->start=0;
    mat->true_wid=mat->wid=wid;
    mat->hei=hei;
}


int main(int argc, char const *argv[]){
    int key,hei,wid,count=0;
    fscanf(stdin,"%d %d %d",&key,&hei,&wid);
    Mat m;
    initMat(&m,wid,hei);
    for(int i = 0 ; i < hei*wid ; ++i){
        fscanf(stdin, "%d",m.data+i);
    }

    // canto superior esquerdo
    for(int y0 = 0 ; y0 < hei ; ++y0){
        for(int x0 = 0 ; x0 < wid ; ++x0){

            // canto inferior direito
            for(int y1 = y0 ; y1 < hei ; ++y1){
                for(int x1 = x0 ; x1 < wid ; ++x1){
                    Mat subm = sub_mat(m,x0,y0,x1,y1);
                    
                    // preciso de lock
                    count += (sum_mat(subm) == key);

                }
            }
        }
    }

    fprintf(stdout,"%d\n",count);

    return 0;
}
