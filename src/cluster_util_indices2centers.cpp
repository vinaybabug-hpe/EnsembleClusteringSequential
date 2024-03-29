/*
 ============================================================================
 Name        : cluster_util_indices2centers.c
 Author      : Vinay B Gavirangaswamy
 Created on	 : Aug 21, 2015
 Version     : 1.0
 Copyright   :  This program is free software: you can redistribute it and/or modify
    			it under the terms of the GNU General Public License as published by
    			the Free Software Foundation, either version 3 of the License, or
    			(at your option) any later version.

    			This program is distributed in the hope that it will be useful,
    			but WITHOUT ANY WARRANTY; without even the implied warranty of
    			MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    			GNU General Public License for more details.


    			You should have received a copy of the GNU General Public License
    			along with this program.  If not, see <http://www.gnu.org/licenses/>.
 Description : 
 ============================================================================
 */
/* Include files */
/* Include files */
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string>
#include <cstring>

/* Some sample C code for the quickselect algorithm,
   taken from Numerical Recipes in C. */

#define SWAP(a,b) temp=(a);(a)=(b);(b)=temp;

static float quickselect(float *arr, int n, int k) {
  unsigned long i,ir,j,l,mid;
  float a,temp;

  l=0;
  ir=n-1;
  for(;;) {
    if (ir <= l+1) {
      if (ir == l+1 && arr[ir] < arr[l]) {
	SWAP(arr[l],arr[ir]);
      }
      return arr[k];
    }
    else {
      mid=(l+ir) >> 1;
      SWAP(arr[mid],arr[l+1]);
      if (arr[l] > arr[ir]) {
	SWAP(arr[l],arr[ir]);
      }
      if (arr[l+1] > arr[ir]) {
	SWAP(arr[l+1],arr[ir]);
      }
      if (arr[l] > arr[l+1]) {
	SWAP(arr[l],arr[l+1]);
      }
      i=l+1;
      j=ir;
      a=arr[l+1];
      for (;;) {
	do i++; while (arr[i] < a);
	do j--; while (arr[j] > a);
	if (j < i) break;
	SWAP(arr[i],arr[j]);
      }
      arr[l+1]=arr[j];
      arr[j]=a;
      if (j >= k) ir=j-1;
      if (j <= k) l=i;
    }
  }
}

static float median (int n, float x[])
/**
 * TODO: Running quick sort and taking median is too slow so changed
 * to use quick select as it has linear run time of O(n) or O(n^2) worst case
 *
 * Find the median of X(1), ... , X(N), using as much of the quicksort
 * algorithm as is needed to isolate it.
 * Uses cdpAdvancedQuickSort algorithm from nvidia 7.5 sample code
*/

{ int i, j;
  int nr = n / 2;
  int nl = nr - 1;
  int even = 0;


  if (n==2*nr) even = 1;
  if (n<3)
  { if (n<1) return 0.;
    if (n == 1) return x[0];
    return 0.5*(x[0]+x[1]);
  }

  if(even) return quickselect(x, n, nr);
  else return quickselect(x, n, nl);

//  run_qsort(n, x, 0);
//
//  if (even) return (0.5*(x[nl]+x[nr]));
//  return x[nr];

}

/* Function Definitions */
void cluster_util_indices2centers(float *X, int nSamples, int nFeatures, int  *I, char *metric, int nClusters, float* M_data)
{

  /* CLUSTER_UTIL_INDICES2CENTERS - Computes cluster centers based on indices */
  /*  */
  /*    Syntax:  M =  cluster_util_indices2centers(X,I,metric) */
  /*             [M,STD,SEM] = cluster_util_indices2centers(X,I,metric) */
  /*  */
  /*    X      :[n,p] matrix of data */
  /*  */
  /*    I      : [n,1] set of cluster assignment indices for N data points */
  /*             if includes zeros or nans, these will be ignored. */
  /*  */
  /*    metric : 'mean' (default) or 'median'.  */
  /*  */
  /*    NOTE: appropriate metric depends on distance */
  /*    metric used in the clustering algorithm. squared euclidean, cosine, */
  /*    correlation distances need to have centers/centroids be means. */
  /*    cityblock and hamming distances need to have centers/centroids that are  */
  /*    medians.  I  am not sure what chebychev, mahal, and others use.  Note, */
  /*    this only matters if clustering algorithm explicity computes centers, */
  /*    as in kmeans and spectral.  kmedoids doesn't compute centers. */
  /*  */
  /*    M      [nClusters, nFeatures] rows contain K prototypes , columns are attributes */

  /*  check for NaN in indices (if I generated by clustering a boostrap data */
  /*  set, then I will have NaN or Zeros for samples not included in the boostrap set. */
  /*  get variable sizes */

	// TODO: Currently only mean is supported median can be implemented later
	std::string metricStr(metric);

	if (metricStr.compare("mean")==0){

		/**
		 * Iterate through all unique cluster id's
		 * for each find mean of data[i] where idx[i] = c
		 * where c is current cluster id.
		 * As cluster id's are unique and range from 1 to *
		 */
		for(int c = 1; c <= nClusters; c++){

			for(int col =0; col < nFeatures; col++){
				float mean = 0;
				int cntMean = 0;

				for(int row =0; row < nSamples; row++){
					if(c == I[col]){
						mean += X[row * nFeatures + col];
						cntMean++;
					}
				}
				if(cntMean != 0)
					M_data[(c-1) * nFeatures + col] = mean/cntMean;
				else
					M_data[(c-1) * nFeatures + col] = 0;
			}
		}
	}
	else if (metricStr.compare("median")==0){

		float *cache = (float*) malloc(nSamples * sizeof(float));

		/**
		 * Iterate through all unique cluster id's
		 * for each find mean of data[i] where idx[i] = c
		 * where c is current cluster id.
		 * As cluster id's are unique and range from 1 to *
		 */
		for(int c = 1; c <= nClusters; c++){

			for(int col =0; col < nFeatures; col++){
				int cntObjs = 0;
				for(int row =0; row < nSamples; row++){
					if(c == I[col]){
						cache[cntObjs]= X[row * nFeatures + col];
						cntObjs++;
					}
				}
				if(cntObjs != 0)
					M_data[(c-1) * nFeatures + col] = median(cntObjs, cache);
				else
					M_data[(c-1) * nFeatures + col] = 0;
			}
		}
	}

}

/* End of code generation (cluster_util_indices2centers.c) */
