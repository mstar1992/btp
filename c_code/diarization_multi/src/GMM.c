/****************************************************************************
 *   Function             : A collection of procedures for Gaussian Mixture
 *                        : Modeling
 *   Uses                 : DspLibrary.c, InitAsdf.c
 *   Author               : Hema A Murthy
 *   Last Updated         : May 23 2002
 *   Source               : Rabiner and Juang, Fundamentals of Speech Recogn.
 *   Bugs                 : none known to date
 *****************************************************************************/

#include "../../include/FrontEndDefs.h"
#include "../../include/FrontEndTypes.h"
#include "../../include/InitAsdf.h"
#include "../../include/DspLibrary.h"
#include "../../include/GMM.h"
#include <stdlib.h>
#include <math.h>


/****************************************************************************
 *   Function             : InitGMM - initialises the GMMs with a set
 *                        : of mean vectors and variance vectors
 *   Input args           : seed - seed for randomising
 *                        : vfv - vector of featureVectors
 *                        : numMixtures : number of Vectors
 *   Output args          : mixtureMeans - vector of GMM mean vectors
 *                        : mixtureVars  - vector of GMM variance vectors 
 *****************************************************************************/

void InitGMM (VECTOR_OF_F_VECTORS *vfv,int numVectors, 
              VECTOR_OF_F_VECTORS *mixtureMeans,               
	      VECTOR_OF_F_VECTORS *mixtureVars, 
	     int numMixtures, int seed) {

  int                          index;
  int                          i, j;
  int                          random;
  float                        rmax;


  srand(seed);
  printf("seed = %d\n",seed);
  fflush(stdout);
  for (i = 0; i < numMixtures; i++) {
      random = rand();
      rmax = RAND_MAX;
      index = (int) ((float) (random/rmax*numVectors));
     for (j = 0; j <vfv[0]->numElements; j++)
      mixtureMeans[i]->array[j] = vfv[index]->array[j];
    for (j = 0; j <vfv[0]->numElements; j++)
      mixtureVars[i]->array[j] = 1.0;
  }
}


/****************************************************************************
 *   Function             : ComputeProbability - computes euclidean distance
 *                        : distance between two vectors
 *   Input args           : mixtureMean, mixtureVar, priorProb, 
 *                        : probScaleFactor, fvect : input vector 
 *   Outputs              : ComputeDiscrimnant - distance      	  
 *****************************************************************************/

float ComputeProbability(F_VECTOR *mixtureMean, 
			  F_VECTOR *mixtureVar, float priorProb, 
			 F_VECTOR *fvect, float probScaleFactor) 
{
  int                     i=0;  
  volatile float                   sumProb = 0;
  float                   scale = 0.0;
  float                   floorValue = 0.0;
  float                   temp = 0.0;
  //priorProb = 1.0;
  //  printf("i_max : %d\n", fvect->numElements);
  // if (prevMean != mixtureMean) {
  //printf("scale f:%f\n", probScaleFactor);
  probScaleFactor = 1.0;
  for (i = 0; i < fvect->numElements; i++)
    {
      if (mixtureVar->array[i] > 0.0001)
	scale = scale + log(mixtureVar->array[i]);
    } /*  for (..i < fvect->numElements..)  */
  scale = 0.5*(fvect->numElements*log(2.0*PI) + scale);
  scale =/* log(priorProb)*/ - scale;  
  //    prevMean = mixtureMean;
  /*    printf("scale = %f \n", scale);
	scanf("%*c"); */
  //}
  
  sumProb = scale;
  for (i = 0; i < fvect->numElements; i++) {
    floorValue =  floorValue +
      (mixtureMean->array[i] - fvect->array[i])*
      (mixtureMean->array[i] - fvect->array[i])
      /(2*mixtureVar->array[i]);
  }
  //  for( i = 0; i < fvect->numElements; i++){
  // temp = temp + 2 * log(mixtureMean->array[i] - fvect->array[i]) - log(mixtureVar->array[i]);
  // }
      
  sumProb = sumProb - floorValue; //fvect->numElements;
  //  printf("f1: %f   log(f1): %f  sumProb: %f\n", floorValue, log(floorValue), sumProb);
  /*  printf("sum = %f\n", sum); */
  //printf("sumprob: %f\n", sumProb);
  //printf("sumprob scaled: %f\n", sumProb + log(probScaleFactor));
  return (sumProb);
}

/****************************************************************************
 *   Function             : DecideWhichMixture - determines index of Mixture
 *                        : to which a given vector belongs
 *   Input args           : fvect : input vector to be classified
 *                                  mixtureMeans,  mixtureVars,numMixtures,
 *                                  mixtureElemCnt, numVectors, probScaleFactor
 *   Outputs              : DecideWhichMixture - Mixture index      	  
 *****************************************************************************/

int DecideWhichMixture(F_VECTOR *fvect, 
                      VECTOR_OF_F_VECTORS *mixtureMeans, 
                      VECTOR_OF_F_VECTORS *mixtureVars, 
                      int numMixtures, float *mixtureElemCnt, 
                      int numVectors, float probScaleFactor) {
  int                     i;
  float                   tempDesc;
  int                     index;
  float                   Discriminant, priorProb;

  priorProb = mixtureElemCnt[0]/numVectors;
  Discriminant  = ComputeProbability(mixtureMeans[0], 
                                   mixtureVars[0], priorProb, 
				     fvect, probScaleFactor);
  //  printf("tempDesc: %f\n", Discriminant);
  index = 0;
  probScaleFactor = 1.0;//i put it
  for (i = 1; i < numMixtures; i++) {
    if (mixtureElemCnt[i] == 0)
      priorProb = 1.0E-100;
    else
      priorProb = mixtureElemCnt[i]/numVectors;
    tempDesc = ComputeProbability(mixtureMeans[i], 
				  mixtureVars[i], priorProb, 
				  fvect, probScaleFactor);
    //printf("tempDesc: %f\n", tempDesc);
    if (tempDesc > Discriminant) {
      index = i;
      Discriminant = tempDesc;
    }
  }
  return(index);
}

/****************************************************************************
 *   Function             : ComputeGMM - compute GMMs
 *                        : for the given set of vectors
 *   Input args           : vfv - input vectors,
 *                        : numVectors - number of input vectors
 *                        : numMixtures - codebook size 
 *                        : VQIter - number of VQ iterations
 *                        : GMMIter - number of GMM iterations
 *   Outputs		  : mixtureMeans - array of mixture means
 *			  : mixtureVars - array of mixture vars
 *			  : mixtureElemCnt - number of elements in
 *			    each mixture
 *****************************************************************************/



void ComputeGMM(VECTOR_OF_F_VECTORS *vfv, int numVectors, 
		VECTOR_OF_F_VECTORS *mixtureMeans, 
		VECTOR_OF_F_VECTORS *mixtureVars, 
		float *mixtureElemCnt, int numMixtures, 
		int VQIter, int GMMIter, float probScaleFactor,
                int ditherMean, int varianceNormalize, int seed) {
  int                            i,j,k;
  static VECTOR_OF_F_VECTORS     *tempMeans, *tempVars;
  static float                   *tempMixtureElemCnt;
  int                            mixtureNumber;
  int                            featLength;
  int                            flag;
  //int                            total;
  //int                            minIndex, maxIndex;
  //int                            minMixtureSize, maxMixtureSize;
  //  printf("computing GMM......\n");
  probScaleFactor = 1.0;
  featLength = vfv[0]->numElements;
  tempMeans = (VECTOR_OF_F_VECTORS *) calloc (numMixtures, 
					      sizeof(VECTOR_OF_F_VECTORS));
  tempVars = (VECTOR_OF_F_VECTORS *) calloc (numMixtures, 
					      sizeof(VECTOR_OF_F_VECTORS));
  for (i = 0; i < numMixtures; i++) { 
    tempMeans[i] = (F_VECTOR *) AllocFVector(featLength);
    tempVars[i] = (F_VECTOR *) AllocFVector(featLength);
  }
  tempMixtureElemCnt = (float *) AllocFloatArray(tempMixtureElemCnt, 
						 numMixtures);
  // printf("temp mixtures allocated\n");
  fflush(stdout);
  //InitGMM (vfv, numVectors, mixtureMeans, mixtureVars, numMixtures, seed);
  ComputeVQ(vfv, numVectors, mixtureMeans, mixtureVars,
      mixtureElemCnt, numMixtures, varianceNormalize, 
      ditherMean, VQIter, seed);
  for ( k = 0; k < GMMIter; k++) {
    //    printf(" GMM iteration number = %d\n", k);
    fflush(stdout);
    for (i = 0; i < numMixtures; i++) {
      for (j = 0; j < vfv[0]->numElements; j++) {
	tempMeans[i]->array[j] = 0;
	tempVars[i]->array[j] = 0;
      }
      tempMixtureElemCnt[i] = 0;
    }
    for (i = 0; i < numVectors; i++) {
      mixtureNumber = DecideWhichMixture(vfv[i], mixtureMeans, 
					 mixtureVars, numMixtures, 
					 mixtureElemCnt, numVectors,
					 probScaleFactor);
      tempMixtureElemCnt[mixtureNumber] = tempMixtureElemCnt[mixtureNumber]+1;
      for (j = 0; j < featLength; j++){
	tempMeans[mixtureNumber]->array[j] = 
	  tempMeans[mixtureNumber]->array[j] + vfv[i]->array[j];
      }
    }  
    /*    for (i = 0; i < numMixtures; i++)
	  printf("mixElemCnt %d = %e\n", i, tempMixtureElemCnt[i]); */
      /*	scanf("%*c"); */
    for (i = 0; i < numMixtures; i++) {
      for (j = 0; j < featLength; j++){
	if (tempMixtureElemCnt[i] != 0)
	  tempMeans[i]->array[j] = 
	    tempMeans[i]->array[j]/tempMixtureElemCnt[i];
	else
	  tempMeans[i]->array[j] = mixtureMeans[i]->array[j];
      }
    }
      
    for (i = 0; i < numVectors; i++) {
      mixtureNumber = DecideWhichMixture(vfv[i], mixtureMeans, 
					 mixtureVars, numMixtures, 
					 mixtureElemCnt, numVectors, 
					 probScaleFactor);
      for (j = 0; j < featLength; j++){
	tempVars[mixtureNumber]->array[j] = 
	  tempVars[mixtureNumber]->array[j] + 
	  (vfv[i]->array[j] - tempMeans[mixtureNumber]->array[j]) * 
	  (vfv[i]->array[j] - tempMeans[mixtureNumber]->array[j]);
      }
    }
    /*for (i = 0; i < numMixtures; i++)
      printf("mixElemCnt %d = %f\n", i, tempMixtureElemCnt[i]);
      scanf("%*c");*/
    
    for (i = 0; i < numMixtures; i++){
      flag = ZeroFVector(tempMeans[i]);
      flag = (flag || ZeroFVector(tempVars[i]));
      for (j = 0; j < featLength; j++) {
	if ((tempMixtureElemCnt[i] > 1) && !flag){
	  mixtureMeans[i]->array[j] = tempMeans[i]->array[j];
	  mixtureVars[i]->array[j] = tempVars[i]->array[j]
	    /tempMixtureElemCnt[i];	
	  if (mixtureVars[i]->array[j] < 1.0E-10) 
	    mixtureVars[i]->array[j] = 0.0001;
	  mixtureElemCnt[i] = tempMixtureElemCnt[i];
	} else {
	  //	     mixtureMeans[i]->array[j] = tempMeans[i]->array[j];
	  mixtureVars[i]->array[j] = 0.0001;
	  mixtureElemCnt[i] = 0;
	  /* printf("Mixture %d %d mean = %e var = %e mixElemCnt = %e\n",i,j, 
		 mixtureMeans[i]->array[j],
		 mixtureVars[i]->array[j], mixtureElemCnt[i]);*/
	  fflush(stdout);

	  }
	}
      }
    }
    /*  for (i = 0; i < numMixtures; i++)
    for (j = 0; j < featLength; j++) {
	printf("mean %d %d = %f var %d %d = %f\n",i,j, 
	       mixtureMeans[i]->array[j], i, j, mixtureVars[i]->array[j]);
	fflush(stdout);
	}*/
  // printf("computing GMM finished.\n");
}

/*-------------------------------------------------------------------------
 *  AllocFloatArray -- Allocates an array of floats
 *    Args:	Array, size of array
 *    Returns:	allocated array
 *    Bugs:	
 * -------------------------------------------------------------------------*/

float * AllocFloatArray(float *array, int npts)
{
  array = (float *) calloc (npts, sizeof(float));
  if (array == NULL) {
    printf("unable to allocate Float array \n");
    exit(-1);
  }
  return(array);
}	/*  End of AllocFloatArray */	

/*-------------------------------------------------------------------------
 *  AllocIntArray -- Allocates an array of Ints
 *    Args:	Array, size of array
 *    Returns:	allocated array
 *    Bugs:	
 * -------------------------------------------------------------------------*/

int *AllocIntArray(int *array, int npts)
{
  array = (int *) calloc(npts,sizeof(int));
  if (array == NULL) {
    printf("unable to allocate Int array \n");
    exit(-1);
  }
  return(array);
}	/*  End of AllocIntArray  */


/*--------------------------------------------------------------------------
    Determine if most elements in an F_VECTOR are zero 
    ---------------------------------------------------------------------------*/
int ZeroFVector (F_VECTOR *fvect) {
  int i = 0;
  int flag = 0;
  int cnt = 0;
  int numElem = ceilf((float)fvect->numElements/3.0);
  while ((i < fvect->numElements) && (cnt < numElem)) {
    flag = (fvect->array[i] == 0);
    i++;
    if (flag)
      cnt++;
  }
  flag = cnt >= numElem;
  return(flag);
}

/******************************************************************************
   AllocFVector : Allocate space for a variable of type F_VECTOR with an 
   array size of numPts.
   inputs : npts
   outputs : an F_VECTOR of size npts
******************************************************************************/

F_VECTOR *AllocFVector(int npts) {
  F_VECTOR *fVect;
  fVect = (F_VECTOR *) malloc(1*sizeof(F_VECTOR));
  if (fVect == NULL) {
    printf("unable to allocate FVector \n");
    exit(-1);
  }
  fVect->numElements = npts;
  fVect->array = (float *) calloc(npts, sizeof(float));
  if (fVect->array == NULL) {
    printf("unable to allocate FVector array \n");
    exit(-1);
  } else
    return(fVect);
}

/*--------------------------------------------------------------------------
    FindIndex checks whether a given element is found in a 
    an array of ints of size npts
 -------------------------------------------------------------------------*/
 int FindIndex(int *array,int npts, int index)
 {
   int flag = 0;
   int i = 0;
   while ((i <= npts) && (!flag)) 
     if (array[i] == index)
       flag = 1;
     else 
       i++;
   return(flag);
 }
/*--------------------------------------------------------------------------
    FindMatch checks whether a given element already exists 
    an array of ints of size npts
 -------------------------------------------------------------------------*/
 int FindMatch(VECTOR_OF_F_VECTORS *vfv, int numVectors, int *array,int npts, int index)
 {
   F_VECTOR *fvect1, *fvect2;
   fvect1 = vfv[index];
   int flag = 0;
   int i = 0, k;
   float sum;
   while ((i <= npts) && (!flag)){
     fvect2 = vfv[i];
     k = 0;
     sum = 0;
     while (k < fvect1->numElements) {
       sum = sum + fvect1->array[k] - fvect2->array[k];
       k++;
     }
     if (sum == 0) flag = 1;
   i++;
   }
   return(flag);
 }

