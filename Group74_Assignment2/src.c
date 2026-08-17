#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

/* flat 3D index, row-major, z outermost */
static inline long IDX(int iz,int iy,int ix,int NX,int NY){
    return (long)iz*NY*NX + (long)iy*NX + ix;
}

/* get MPI rank of process at (cx,cy,cz), -1 if outside grid */
static int get_rank(int cx,int cy,int cz,int px,int py,int pz){
    if(cx<0||cx>=px) return -1;
    if(cy<0||cy>=py) return -1;
    if(cz<0||cz>=pz) return -1;
    return cz*(px*py)+cy*px+cx;
}

/* halo exchange: share boundary data with neighbours in X->Y->Z order
   Y packs full NX width to carry x-corners, Z packs full NX*NY to carry x+y edges
   buffers passed in from main, not allocated here */
static void halo_exchange(double *A,
    int nx,int ny,int nz,int k,
    int NX,int NY,int NZ,
    int cx,int cy,int cz,
    int px,int py,int pz,
    MPI_Comm comm,
    double *sxm,double *sxp,double *rxm,double *rxp,
    double *sym,double *syp,double *rym,double *ryp,
    double *szm,double *szp,double *rzm,double *rzp
){
    MPI_Status  stats[4];
    MPI_Request req[4];
    int r;

    /* find all 6 neighbours, -1 means domain boundary in that direction */
    int nb_xm = get_rank(cx-1,cy,cz,px,py,pz);
    int nb_xp = get_rank(cx+1,cy,cz,px,py,pz);
    int nb_ym = get_rank(cx,cy-1,cz,px,py,pz);
    int nb_yp = get_rank(cx,cy+1,cz,px,py,pz);
    int nb_zm = get_rank(cx,cy,cz-1,px,py,pz);
    int nb_zp = get_rank(cx,cy,cz+1,px,py,pz);

    long p;

    /* --- X --- */
    long xfs=(long)k*ny*nz;

    /* pack left and right interior faces */
    p=0;
    for(int iz=k;iz<nz+k;iz++)
    for(int iy=k;iy<ny+k;iy++)
    for(int ix=k;ix<2*k;ix++)
        sxm[p++]=A[IDX(iz,iy,ix,NX,NY)];

    p=0;
    for(int iz=k;iz<nz+k;iz++)
    for(int iy=k;iy<ny+k;iy++)
    for(int ix=nx;ix<nx+k;ix++)
        sxp[p++]=A[IDX(iz,iy,ix,NX,NY)];

    /* non-blocking send+recv then wait */
    r=0;
    if(nb_xm>=0) MPI_Irecv(rxm,xfs,MPI_DOUBLE,nb_xm,11,comm,&req[r++]);
    if(nb_xp>=0) MPI_Irecv(rxp,xfs,MPI_DOUBLE,nb_xp,10,comm,&req[r++]);
    if(nb_xm>=0) MPI_Isend(sxm,xfs,MPI_DOUBLE,nb_xm,10,comm,&req[r++]);
    if(nb_xp>=0) MPI_Isend(sxp,xfs,MPI_DOUBLE,nb_xp,11,comm,&req[r++]);
    MPI_Waitall(r,req,stats);

    /* unpack into x ghost layers */
    if(nb_xm>=0){
        p=0;
        for(int iz=k;iz<nz+k;iz++)
        for(int iy=k;iy<ny+k;iy++)
        for(int ix=0;ix<k;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=rxm[p++];
    }
    if(nb_xp>=0){
        p=0;
        for(int iz=k;iz<nz+k;iz++)
        for(int iy=k;iy<ny+k;iy++)
        for(int ix=nx+k;ix<NX;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=rxp[p++];
    }

    /* --- Y (full NX width so x-ghost corners are included) --- */
    long yfs=(long)NX*k*nz;

    p=0;
    for(int iz=k;iz<nz+k;iz++)
    for(int iy=k;iy<2*k;iy++)
    for(int ix=0;ix<NX;ix++)
        sym[p++]=A[IDX(iz,iy,ix,NX,NY)];

    p=0;
    for(int iz=k;iz<nz+k;iz++)
    for(int iy=ny;iy<ny+k;iy++)
    for(int ix=0;ix<NX;ix++)
        syp[p++]=A[IDX(iz,iy,ix,NX,NY)];

    r=0;
    if(nb_ym>=0) MPI_Irecv(rym,yfs,MPI_DOUBLE,nb_ym,21,comm,&req[r++]);
    if(nb_yp>=0) MPI_Irecv(ryp,yfs,MPI_DOUBLE,nb_yp,20,comm,&req[r++]);
    if(nb_ym>=0) MPI_Isend(sym,yfs,MPI_DOUBLE,nb_ym,20,comm,&req[r++]);
    if(nb_yp>=0) MPI_Isend(syp,yfs,MPI_DOUBLE,nb_yp,21,comm,&req[r++]);
    MPI_Waitall(r,req,stats);

    if(nb_ym>=0){
        p=0;
        for(int iz=k;iz<nz+k;iz++)
        for(int iy=0;iy<k;iy++)
        for(int ix=0;ix<NX;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=rym[p++];
    }
    if(nb_yp>=0){
        p=0;
        for(int iz=k;iz<nz+k;iz++)
        for(int iy=ny+k;iy<NY;iy++)
        for(int ix=0;ix<NX;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=ryp[p++];
    }

    /* --- Z (full NX*NY slab so x+y ghost edges are included) --- */
    long zfs=(long)NX*NY*k;

    p=0;
    for(int iz=k;iz<2*k;iz++)
    for(int iy=0;iy<NY;iy++)
    for(int ix=0;ix<NX;ix++)
        szm[p++]=A[IDX(iz,iy,ix,NX,NY)];

    p=0;
    for(int iz=nz;iz<nz+k;iz++)
    for(int iy=0;iy<NY;iy++)
    for(int ix=0;ix<NX;ix++)
        szp[p++]=A[IDX(iz,iy,ix,NX,NY)];

    r=0;
    if(nb_zm>=0) MPI_Irecv(rzm,zfs,MPI_DOUBLE,nb_zm,31,comm,&req[r++]);
    if(nb_zp>=0) MPI_Irecv(rzp,zfs,MPI_DOUBLE,nb_zp,30,comm,&req[r++]);
    if(nb_zm>=0) MPI_Isend(szm,zfs,MPI_DOUBLE,nb_zm,30,comm,&req[r++]);
    if(nb_zp>=0) MPI_Isend(szp,zfs,MPI_DOUBLE,nb_zp,31,comm,&req[r++]);
    MPI_Waitall(r,req,stats);

    if(nb_zm>=0){
        p=0;
        for(int iz=0;iz<k;iz++)
        for(int iy=0;iy<NY;iy++)
        for(int ix=0;ix<NX;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=rzm[p++];
    }
    if(nb_zp>=0){
        p=0;
        for(int iz=nz+k;iz<NZ;iz++)
        for(int iy=0;iy<NY;iy++)
        for(int ix=0;ix<NX;ix++)
            A[IDX(iz,iy,ix,NX,NY)]=rzp[p++];
    }
}

/* d-point stencil: average over up to k neighbours each direction + self
   boundary points use fewer neighbours, divide by actual count m not fixed d */
static void stencil_update(double *src,double *dst,
    int nx,int ny,int nz,int k,
    int gx0,int gy0,int gz0,
    int GX,int GY,int GZ,
    int NX,int NY){

    for(int iz=k;iz<nz+k;iz++){
        int gz=gz0+(iz-k);
        for(int iy=k;iy<ny+k;iy++){
            int gy=gy0+(iy-k);
            for(int ix=k;ix<nx+k;ix++){
                int gx=gx0+(ix-k);

                double sum=src[IDX(iz,iy,ix,NX,NY)];
                int cnt=1;

                /* skip neighbours outside global domain, count only valid ones */
                for(int s=1;s<=k;s++){
                    if(gx+s<GX){sum+=src[IDX(iz,iy,ix+s,NX,NY)];cnt++;}
                    if(gx-s>=0) {sum+=src[IDX(iz,iy,ix-s,NX,NY)];cnt++;}
                    if(gy+s<GY){sum+=src[IDX(iz,iy+s,ix,NX,NY)];cnt++;}
                    if(gy-s>=0) {sum+=src[IDX(iz,iy-s,ix,NX,NY)];cnt++;}
                    if(gz+s<GZ){sum+=src[IDX(iz+s,iy,ix,NX,NY)];cnt++;}
                    if(gz-s>=0) {sum+=src[IDX(iz-s,iy,ix,NX,NY)];cnt++;}
                }

                dst[IDX(iz,iy,ix,NX,NY)]=sum/cnt;
            }
        }
    }
}

/* count unit cubes where isovalue crosses through (at least one corner below, one above)
   boundary cubes between processes are double counted */
static long count_cells(double *A,int nx,int ny,int nz,int k,
    double iso,int NX,int NY,
    int gx0,int gy0,int gz0,
    int GX,int GY,int GZ){

    long cnt=0;

    /* stop one short at global boundary, include boundary cube if neighbour exists */
    int ixlim = (gx0+nx < GX) ? nx+k : nx+k-1;
    int iylim = (gy0+ny < GY) ? ny+k : ny+k-1;
    int izlim = (gz0+nz < GZ) ? nz+k : nz+k-1;

    for(int iz=k;iz<izlim;iz++)
    for(int iy=k;iy<iylim;iy++)
    for(int ix=k;ix<ixlim;ix++){

        /* 8 corners of this unit cube */
        double c0=A[IDX(iz,  iy,  ix,  NX,NY)];
        double c1=A[IDX(iz,  iy,  ix+1,NX,NY)];
        double c2=A[IDX(iz,  iy+1,ix,  NX,NY)];
        double c3=A[IDX(iz,  iy+1,ix+1,NX,NY)];
        double c4=A[IDX(iz+1,iy,  ix,  NX,NY)];
        double c5=A[IDX(iz+1,iy,  ix+1,NX,NY)];
        double c6=A[IDX(iz+1,iy+1,ix,  NX,NY)];
        double c7=A[IDX(iz+1,iy+1,ix+1,NX,NY)];

        /* count if corners exist on both sides of isovalue */
        int below=(c0<iso)||(c1<iso)||(c2<iso)||(c3<iso)||
                  (c4<iso)||(c5<iso)||(c6<iso)||(c7<iso);
        int above=(c0>=iso)||(c1>=iso)||(c2>=iso)||(c3>=iso)||
                  (c4>=iso)||(c5>=iso)||(c6>=iso)||(c7>=iso);

        if(below&&above) cnt++;
    }

    return cnt;
}

/* main function */
int main(int argc,char* argv[]){
    MPI_Init(&argc,&argv);

    int rank,size;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&size);

    if(argc<13){
        if(rank==0)
            fprintf(stderr,
                "Usage: %s d ppn px py pz nx ny nz T seed F isovalue\n",
                argv[0]);
        MPI_Finalize();
        return 1;
    }

    int d   =atoi(argv[1]),  ppn=atoi(argv[2]);
    int px  =atoi(argv[3]),  py =atoi(argv[4]),  pz=atoi(argv[5]);
    int nx  =atoi(argv[6]),  ny =atoi(argv[7]),  nz=atoi(argv[8]);
    int T   =atoi(argv[9]),  seed=atoi(argv[10]), F=atoi(argv[11]);
    double iso=atof(argv[12]);

    /* k = halo depth: d=7->k=1, d=13->k=2, d=19->k=3 */
    int k=(d-1)/6;

    /* this process's position in the 3D process grid */
    int cx=rank%px;
    int cy=(rank/px)%py;
    int cz=rank/(px*py);

    /* global start index of this process's chunk */
    int gx0=cx*nx, gy0=cy*ny, gz0=cz*nz;

    /* total global domain size */
    int GX=px*nx,  GY=py*ny,  GZ=pz*nz;

    /* local array size with halo on both sides of each axis */
    int NX=nx+2*k, NY=ny+2*k, NZ=nz+2*k;
    long haloSize=(long)NX*NY*NZ;

    /* allocate cur and nxt for F fields, calloc keeps ghost cells zero */
    double **cur=malloc(F*sizeof(double*));
    double **nxt=malloc(F*sizeof(double*));
    for(int f=0;f<F;f++){
        cur[f]=calloc(haloSize,sizeof(double));
        nxt[f]=calloc(haloSize,sizeof(double));
    }

    /* initialise interior data as per assignment spec */
    srand(seed);
    for(int i=0;i<F;i++){
        for(long j=0;j<(long)nx*ny*nz;j++){
            double val=(double)rand()*(rank+1)/(110426.0+i+j);
            int x=j%nx;
            int y=(j/nx)%ny;
            int z=j/(nx*ny);
            cur[i][IDX(z+k,y+k,x+k,NX,NY)]=val;
        }
    }

    long *lcnt=malloc(F*sizeof(long));  /* local  isovalue counts */
    long *gcnt=malloc(F*sizeof(long));  /* global isovalue counts */

    /* halo buffer sizes */
    long xfs=(long)k*ny*nz;
    long yfs=(long)NX*k*nz;
    long zfs=(long)NX*NY*k;

    /* allocate halo buffers once, reused every timestep */
    double *sxm=malloc(xfs*sizeof(double));
    double *sxp=malloc(xfs*sizeof(double));
    double *rxm=malloc(xfs*sizeof(double));
    double *rxp=malloc(xfs*sizeof(double));

    double *sym=malloc(yfs*sizeof(double));
    double *syp=malloc(yfs*sizeof(double));
    double *rym=malloc(yfs*sizeof(double));
    double *ryp=malloc(yfs*sizeof(double));

    double *szm=malloc(zfs*sizeof(double));
    double *szp=malloc(zfs*sizeof(double));
    double *rzm=malloc(zfs*sizeof(double));
    double *rzp=malloc(zfs*sizeof(double));

    double start=MPI_Wtime();

    for(int t=0;t<T;t++){

        /* halo -> stencil -> count */

        for(int f=0;f<F;f++){
            halo_exchange(cur[f],nx,ny,nz,k,NX,NY,NZ,
                cx,cy,cz,px,py,pz,MPI_COMM_WORLD,
                sxm,sxp,rxm,rxp,
                sym,syp,rym,ryp,
                szm,szp,rzm,rzp);
        }

        for(int f=0;f<F;f++)
            stencil_update(cur[f],nxt[f],nx,ny,nz,k,
                gx0,gy0,gz0,GX,GY,GZ,NX,NY);

        /* swap pointers instead of copying arrays */
        for(int f=0;f<F;f++){
            double *tmp=cur[f];
            cur[f]=nxt[f];
            nxt[f]=tmp;
        }

        /* count locally then reduce sum to rank 0 */
        for(int f=0;f<F;f++)
            lcnt[f]=count_cells(cur[f],nx,ny,nz,k,iso,NX,NY,
                                gx0,gy0,gz0,GX,GY,GZ);

        MPI_Reduce(lcnt,gcnt,F,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);

        /* rank 0 prints F counts for this time step */
        if(rank==0){
            for(int f=0;f<F;f++){
                printf("%ld",gcnt[f]);
                if(f<F-1) printf(" ");
            }
            printf("\n");
        }
    }

    /* rank 0 prints elapsed time */
    if(rank==0)
        printf("%f\n",MPI_Wtime()-start);

    /* free all memory */
    for(int f=0;f<F;f++){
        free(cur[f]);
        free(nxt[f]);
    }
    free(cur); free(nxt);
    free(lcnt); free(gcnt);

    free(sxm); free(sxp); free(rxm); free(rxp);
    free(sym); free(syp); free(rym); free(ryp);
    free(szm); free(szp); free(rzm); free(rzp);

    MPI_Finalize();
    return 0;
}
