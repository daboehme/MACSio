/*
Copyright (c) 2015, Lawrence Livermore National Security, LLC.
Produced at the Lawrence Livermore National Laboratory.
Written by Mark C. Miller

LLNL-CODE-676051. All rights reserved.

This file is part of MACSio

Please also read the LICENSE file at the top of the source code directory or
folder hierarchy.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License (as published by the Free Software
Foundation) version 2, dated June 1991.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the terms and conditions of the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <stdio.h>
#include <time.h>
#include <math.h>

#include <macsio_work.h>


int MACSIO_WORK_DoComputeWork(double *currentDt, double targetDelta, int workIntensity) 
{
    int ret = 0;
    switch(workIntensity)
    {
	case 1:
	    MACSIO_WORK_LevelOne(currentDt, targetDelta);
	    break;
	case 2:
	    MACSIO_WORK_LevelTwo(currentDt, targetDelta);
	    break;
	case 3:
	    MACSIO_WORK_LevelThree(currentDt, targetDelta);
	    break;
	default:
	    return 0;
    }

    return ret;
}

/* Sleep */
int MACSIO_WORK_LevelOne(double *currentDt, double targetDelta)
{
    int ret = 0;
    struct timespec tim, tim2;
    tim.tv_sec = targetDelta;
    tim.tv_nsec = 0;
    nanosleep(&tim, &tim2);
    return ret;
}

typedef struct _cell {
    // Cell centered scalars
    double ccs1, ccs2, ccs3, ccs4;
    // Cell centered vectors
    double ccv1[3], ccv2[3], ccv3[3], ccv4[3];
    // Node centered scalars
    double ncs1[4], ncs2[4];
    // Node centered vectors
    double ncv1[4][3], ncv2[4][3];
} cell_t;

typedef struct _mesh {
    cell_t *mcell, *gcell;
} mesh_t;

/* Spin CPU */
int MACSIO_WORK_LevelTwo(double *currentDt, double targetDelta)
{
    int ret = 0;
    time_t start_t, end_t;

    mesh_t *mesh;
    double *xx,*yy,*zz;
    cell_t *cell;

    int nm = 8000;
    int ng = 150;
    int np = 1000000;
    int nshuffle = np/2;
    int nloop = 10;
    int i1, i2, i3;
    double rnum;

    /* Setup data */
    mesh = (mesh_t*) malloc(sizeof(mesh_t));
    mesh->mcell = (cell_t*) malloc(nm*sizeof(cell_t));
    mesh->gcell = (cell_t*) malloc(ng*sizeof(cell_t));

    int *indx = (int*) malloc(np*sizeof(int));
    int *loc = (int*) malloc(np*sizeof(int));

    xx = (double*) malloc(np*sizeof(double));
    yy = (double*) malloc(np*sizeof(double));
    zz = (double*) malloc(np*sizeof(double));

    for (int ii=0; ii<nm; ii++){
	mesh->mcell[ii].ccs1 = ii+1.0;
	mesh->mcell[ii].ccs2 = ii+2.0;
	mesh->mcell[ii].ccs3 = ii+3.0;
	mesh->mcell[ii].ccs4 = ii+4.0;
	for (int j=0; j<3; j++){
	    mesh->mcell[ii].ccv1[j] = ii+5.0;
	    mesh->mcell[ii].ccv2[j] = ii+6.0;
	    mesh->mcell[ii].ccv3[j] = ii+7.0;
	    mesh->mcell[ii].ccv4[j] = ii+8.0;
	}
	for (int j=0; j<4; j++){
	    mesh->mcell[ii].ncs1[j] = ii+9.0;
	    mesh->mcell[ii].ncs2[j] = ii+10.0;
	}
	for (int j=0; j<4; j++){
	    for (int k=0; k<3; k++){
		mesh->mcell[ii].ncv1[j][k] = ii+11.0;
		mesh->mcell[ii].ncv2[j][k] = ii+12.0;
	    }
	}
    }
    for (int ii=0; ii<ng; ii++){
	mesh->mcell[ii].ccs1 = ii+1.0-ng;
	mesh->mcell[ii].ccs2 = ii+2.0-ng;
	mesh->mcell[ii].ccs3 = ii+3.0-ng;
	mesh->mcell[ii].ccs4 = ii+4.0-ng;
	for (int j=0; j<3; j++){
	    mesh->mcell[ii].ccv1[j] = ii+5.0-ng;
	    mesh->mcell[ii].ccv2[j] = ii+6.0-ng;
	    mesh->mcell[ii].ccv3[j] = ii+7.0-ng;
	    mesh->mcell[ii].ccv4[j] = ii+8.0-ng;
	}
	for (int j=0; j<4; j++){
	    mesh->mcell[ii].ncs1[j] = ii+9.0-ng;
	    mesh->mcell[ii].ncs2[j] = ii+10.0-ng;
	}
	for (int j=0; j<4; j++){
	    for (int k=0; k<3; k++){
		mesh->mcell[ii].ncv1[j][k] = ii+11.0-ng;
		mesh->mcell[ii].ncv2[j][k] = ii+12.0-ng;
	    }
	}
    }
    for (int ii=0; ii<np; ii++){
	indx[ii] = ii;
    }
    for (int ii=0; ii<nshuffle; ii++){
	rnum = rand();
	i1 = int(rnum*(ng+nm))-ng;
	if (i1 > nm) i1 = nm;
	if (i1 < (ng*-1)) i1 = ng;
	if (i1 == 0) i1 = 1;
	loc[ii] = i1;
    }

    /* Start timer */ 
    time(&start_t);

    int ii=0;
    /* Start of work loop */
    while (true){
	i1 = indx[ii];
	if (loc[i1] > 0){
	    cell = mesh->mcell;
	} else {
	    cell = mesh->gcell;
	}

	xx[i1]=0.0;
	yy[i1]=0.0;
	zz[i1]=0.0;

	for (int jj=0; jj<nloop; jj++){
	    /* Do flops requiring cell data */
	    xx[i1] += cell[loc[i1]].ccs1;
	    yy[i1] += cell[loc[i1]].ccs2;
	    zz[i1] += cell[loc[i1]].ccs3;

	    xx[i1] += sqrt( square(cell[loc[i1]].ccv1[0]) + square(cell[loc[i1]].ccv2[0]) + square(cell[loc[i1]].ccv3[0]) );
	    yy[i1] += sqrt( square(cell[loc[i1]].ccv1[1]) + square(cell[loc[i1]].ccv2[1]) + square(cell[loc[i1]].ccv3[1]) );
	    zz[i1] += sqrt( square(cell[loc[i1]].ccv1[2]) + square(cell[loc[i1]].ccv2[2]) + square(cell[loc[i1]].ccv3[2]) );

	    xx[i1] += log(cell[loc[i1]].ccv4[0]);
	    yy[i1] += log(cell[loc[i1]].ccv4[1]);
	    zz[i1] += log(cell[loc[i1]].ccv4[2]);

	    xx[i1] += sqrt( cell[loc[i1]].ncs1[0] + cell[loc[i1]].ncs1[1] + cell[loc[i1]].ncs1[2] + cell[loc[i1]].ncs1[3]);
	    yy[i1] += sqrt( cell[loc[i1]].ncs2[0] + cell[loc[i1]].ncs2[1] + cell[loc[i1]].ncs2[2] + cell[loc[i1]].ncs2[3]);
	    zz[i1] += sqrt( cell[loc[i1]].ncs1[0] + cell[loc[i1]].ncs1[1] + cell[loc[i1]].ncs1[2] + cell[loc[i1]].ncs1[3]
			    + cell[loc[i1]].ncs2[0] + cell[loc[i1]].ncs2[1] + cell[loc[i1]].ncs2[2] + cell[loc[i1]].ncs2[3]);

	    xx[i1] += sqrt( exp(cell[loc[i1]].ncv1[0][0])+ exp(cell[loc[i1]].ncv1[1][0])+ exp(cell[loc[i1]].ncv1[2][0])+ exp(cell[loc[i1]].ncv1[4][0]) 
			    + exp(cell[loc[i1]].ncv2[0][0])+ exp(cell[loc[i1]].ncv2[1][0])+ exp(cell[loc[i1]].ncv2[2][0])+ exp(cell[loc[i1]].ncv2[4][0]));
	    yy[i1] += sqrt( exp(cell[loc[i1]].ncv1[0][1])+ exp(cell[loc[i1]].ncv1[1][1])+ exp(cell[loc[i1]].ncv1[2][1])+ exp(cell[loc[i1]].ncv1[4][1]) 
			    + exp(cell[loc[i1]].ncv2[0][1])+ exp(cell[loc[i1]].ncv2[1][1])+ exp(cell[loc[i1]].ncv2[2][1])+ exp(cell[loc[i1]].ncv2[4][1]));
	    zz[i1] += sqrt( exp(cell[loc[i1]].ncv1[0][2])+ exp(cell[loc[i1]].ncv1[1][2])+ exp(cell[loc[i1]].ncv1[2][2])+ exp(cell[loc[i1]].ncv1[4][2]) 
			    + exp(cell[loc[i1]].ncv2[0][2])+ exp(cell[loc[i1]].ncv2[1][2])+ exp(cell[loc[i1]].ncv2[2][2])+ exp(cell[loc[i1]].ncv2[4][2]));
	}

	cell = NULL;
	time(&end_t);
	/* If we've done enough work break out and return to main loop */
	if (difftime(end_t, start_t) >= targetDelta) break;
	if (ii == np){
	    ii = 0;
	} else {
	    ii++;
	}
    }
	printf("Worked for %f seconds, targetDelta %f\n", difftime(end_t,start_t), targetDelta);
	free(xx);
	free(yy); 
	free(zz);
	free(indx);
	free(loc);
	free(mesh->mcell);
	free(mesh->gcell);
	free(mesh);

    return ret;
}

double square(double num)
{
    return num*num;
}

/* Computation based proxy code */
int MACSIO_WORK_LevelThree(double *currentDt, double targetDelta)
{
    int ret = 0;

    return ret;
}