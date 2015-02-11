#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#define MAXIMUM_LINE_LENGTH (2048)

#define NI (7)
#define QI (12)
#define LC (LONG_MAX)
/* #define DEBUG_CODE */
/* #define EPP_ALGORITHM */
#define BERTOGNA_ALGORITHM
/* #define DISPLAY_PREEMPT_POINT_DATA */
/* #define DISPLAY_PREEMPT_POINTS */ 
/* #define DISPLAY_PREEMPT_COST */
/* #define CHECK_PREEMPT_COST */

typedef signed long cost_t;
typedef signed long index_t;
typedef double      pcnt_t;

static cost_t bb[NI] = {0,3,2,2,2,2,3};
static cost_t e[NI][NI] = {
    {0, 1, 2, 4, 4, 3, 2},
    {1, 1, 3, 5, 5, 4, 3},
    {2, 3, 2, 6, 6, 5, 4},
    {4, 5, 6, 4, 6, 7, 6},
    {4, 5, 6, 6, 4, 5, 6},
    {3, 4, 5, 7, 5, 3, 5},
    {2, 3, 4, 6, 6, 5, 2}
};

#define NIB (7)
#define QIB (8)
static cost_t bbB[NIB] = {0,2,2,2,1,2,3};
static cost_t eB[NIB] = {0,1,2,3,3,1,0};

static int getLine (char *prompt, char *buff, size_t sz) 
{
    int ch, extra;

    // Get line with buffer overrun protection.
    if (prompt != NULL) 
    {
        printf ("%s", prompt);
        fflush (stdout);
    }

    if (fgets (buff, sz, stdin) == NULL)
    {
        return NO_INPUT;
    }
	
    // If it was too long, there'll be no newline. In that case, we flush
    // to end of line so that excess doesn't affect the next call.
    if (buff[strlen(buff)-1] != '\n') {
        extra = 0;
        while (((ch = getchar()) != '\n') && (ch != EOF))
            extra = 1;
        return (extra == 1) ? TOO_LONG : OK;
    }

    // Otherwise remove newline and give string back to caller.
    buff[strlen(buff)-1] = '\0';
    return OK;
}

char *readFileLine(FILE *file) {

    int maximumLineLength = MAXIMUM_LINE_LENGTH;

    if (file == NULL) {
        printf("Error: file pointer is null.");
        exit(1);
    }

    char *lineBuffer = (char *)malloc(sizeof(char) * maximumLineLength);

    if (lineBuffer == NULL) {
        printf("Error allocating memory for line buffer.");
        exit(1);
    }

    char ch = getc(file);
    int count = 0;

    while ((ch != '\n') && (ch != EOF)) {
        if (count == maximumLineLength) {
            maximumLineLength += MAXIMUM_LINE_LENGTH;
            lineBuffer = realloc(lineBuffer, maximumLineLength);
            if (lineBuffer == NULL) {
                printf("Error reallocating space for line buffer.");
                exit(1);
            }
        }
        lineBuffer[count] = ch;
        count++;

        ch = getc(file);
    }

    lineBuffer[count] = '\0';
    char *constLine = lineBuffer;
    return constLine;
}

void costMatrixBertognaSimulated(index_t blockCount, cost_t *pCostMatrix)
{
    index_t   accessIdx;
    index_t   j;
    index_t   k;
    cost_t    pcost;
    index_t   s;
    cost_t    maxPCost;

    for (s=1; s<blockCount; s++)
    {
        maxPCost = 0;
        for (j=0; j<blockCount-s; j++)
	{
	    k = j + s;
            accessIdx = (j * blockCount) + k;
            if (pCostMatrix[accessIdx] > maxPCost)
            {
                maxPCost = pCostMatrix[accessIdx];
            }
        }
        for (j=0; j<blockCount-s; j++)
	{
	    k = j + s;
            accessIdx = (j * blockCount) + k;
            pCostMatrix[accessIdx] = maxPCost;
        }
    }
}

void costMatrixBertognaReal(index_t blockCount, cost_t *pCostMatrix, cost_t **pCostArray)
{
    index_t   accessIdx;
    index_t   j;
    index_t   k;
    cost_t    pcost;
    index_t   s;
    cost_t    maxPCost;

    /* Allocate array pointers for the preemptive cost array. */
    *pCostArray = (cost_t *) malloc(blockCount * sizeof(cost_t));
    memset(*pCostArray, 0, (blockCount * sizeof(cost_t)));

    for (s=1; s<blockCount; s++)
    {
        maxPCost = 0;
        for (j=0; j<blockCount-s; j++)
	{
	    k = j + s;
            accessIdx = (j * blockCount) + k;
            if (pCostMatrix[accessIdx] > maxPCost)
            {
                maxPCost = pCostMatrix[accessIdx];
            }
        }
        (*pCostArray)[s] = maxPCost;
    }
}

cost_t optimalPPPlacementBertognaReal(char *taskName, index_t blockCount, cost_t maxTaskQI, cost_t *blockCycles, cost_t *pCostArray)
{
    cost_t    acost;
    cost_t    bcost;
    cost_t   *bCostArray;
    index_t   j;
    index_t   k;
    index_t   n;
    cost_t   *npCostArray;
    index_t   numPreemptions;
    cost_t    pcost;
    index_t  *preemptionsArray;
    index_t  *pPrevArray;
    cost_t    totalPCost;

    /* Allocate array for the non-preemptive cost. */
    npCostArray = (cost_t *) malloc(blockCount * sizeof(cost_t));
    memset(npCostArray, 0, (blockCount * sizeof(cost_t)));

    /* Allocate array for the preemptive cost. */
    bCostArray = (cost_t *) malloc(blockCount * sizeof(cost_t));
    memset(bCostArray, 0, (blockCount * sizeof(cost_t)));

    preemptionsArray = (index_t *) malloc(blockCount * sizeof(index_t));
    memset(preemptionsArray, 0, (blockCount * sizeof(index_t)));

    pPrevArray = (index_t *) malloc(blockCount * sizeof(index_t));
    memset(pPrevArray, 0, (blockCount * sizeof(index_t)));

    for (j=0; j<blockCount; j++)
    {
        npCostArray[j] = (j > 0) ? (npCostArray[j-1] + blockCycles[j]) : blockCycles[j];
        if (blockCycles[j] <= maxTaskQI)
        {
	    pPrevArray[j] = 0;
            preemptionsArray[j] = 1;
        }
        else
        {
            if (pCostArray[j] < maxTaskQI)
            {
	        pPrevArray[j] = 0;
                preemptionsArray[j] = 1;
            }
            else
            {
#if 0
                printf("No feasible solution exists for task %s since blockCycles[%ld] = %ld > maxTaskQI of %ld\n",taskName,j,blockCycles[j],maxTaskQI);
                printf("No feasible solution exists for task %s since pCostArray[%ld] = %ld > maxTaskQI of %ld\n",taskName,j,pCostArray[j],maxTaskQI);
#endif
#if 1
                return LC;
#endif
            }
        }
    }

    for (j=1; j<blockCount; j++)
    {
        bCostArray[j] = (pCostArray[pPrevArray[j]] + npCostArray[j] - npCostArray[pPrevArray[j]]);
        if (bCostArray[j] <= maxTaskQI)
	{
            pPrevArray[j] = pPrevArray[j-1];
            preemptionsArray[j] = 1;
#if defined(DEBUG_CODE)
            printf("QI:bCostArray[%ld]=%ld\n",s,bCostArray[j]);
            printf("QI:pPrevArray[%ld]=%ld\n",s,pPrevArray[j]);
#endif
        }
        else
        {
#if 0
            printf("Checking best preemption point for range [%ld,%ld]\n",(index_t)(pPrevArray[j-1]+1), (index_t)(j-1));
#endif
            bCostArray[j] = LC;
            for (k=pPrevArray[j-1]+1; k<j; k++)
	    {
                bcost = (k > (pPrevArray[j-1]+1)) ? (npCostArray[k]-npCostArray[pPrevArray[j-1]]) : (npCostArray[k]-npCostArray[pPrevArray[j-1]]); 
                acost = (k < (j-1)) ? (npCostArray[j-1]-npCostArray[k]) : 0; 
                pcost = bcost + pCostArray[k] + acost; 
#if 0
                printf("Preemption cost at index %ld is %ld,%ld,%ld.\n", k, bcost, acost, pcost);
#endif
                if (pcost < bCostArray[j])
                {
                    if (pcost > maxTaskQI)
	            {
                        if ((j-k) == 1)
                        {
                            if (pCostArray[k] < maxTaskQI)
                            {
                                numPreemptions = ((pcost-pCostArray[k])/(maxTaskQI-pCostArray[k])); 
                                numPreemptions = (((pcost-pCostArray[k]) % (maxTaskQI-pCostArray[k])) != 0) ? numPreemptions+1 : numPreemptions;
                                bCostArray[j] =  (pcost-pCostArray[k])/numPreemptions + pCostArray[k]; 
                                preemptionsArray[j] = numPreemptions;
                                pPrevArray[j] = k;
#if 0
                                printf("Task %s splitting large basic block with bCostArray[%ld] = %ld > maxTaskQI of %ld into %ld preemptions\n", taskName, j, pcost, maxTaskQI, numPreemptions);
#endif
                            } 
                            else
                            {
	                        bCostArray[j] = LC;
                                pPrevArray[j] = 0;
#if 0
                                printf("No feasible solution exists for task %s since pCostArray[%ld] = %ld > maxTaskQI of %ld for k=%ld\n", taskName, 
                                       k, pCostArray[k], maxTaskQI, k);
#endif
#if 0
	                        printf("npCostArray[%ld]=%ld\n",j,k,npCostArray[j]);
                                printf("pCostArray[%ld]=%ld\n",k,pCostArray[k]);
                                for (n = k; n <= j; n++)
                                {
                                    printf("blockCycles[%ld] = %ld\n", n, blockCycles[n]);
                                }
#endif
                                return LC;
                            }
                        }
                    }
                    else
                    {
                        bCostArray[j] = pcost;
                        pPrevArray[j] = k;
                    }
                }
            }
            bCostArray[j] = (pCostArray[pPrevArray[j]] + npCostArray[j] - npCostArray[pPrevArray[j]]);
        } 
    }

    totalPCost = 0;
#if defined(DISPLAY_PREEMPT_POINTS)
    printf("\n");
    printf("Selected Preemption Points:\n");
#endif
    j = blockCount-1;
    while (j != 0)
    {
#if defined(DISPLAY_PREEMPT_POINTS)
        printf("%ld, ", j);
#endif
        if ((bCostArray[j] < 0) || (bCostArray[j] >= LC))
        {
            printf("Invalid bCostArray[%ld]=%ld.\n", j, bCostArray[j]);
        }
        if (preemptionsArray[j] <= 0)
        {
            printf("Invalid preemptions count %ld at index %ld\n", preemptionsArray[j], j);
        }
        totalPCost += (bCostArray[j] * preemptionsArray[j]);
	j = pPrevArray[j];
    }
#if defined(DISPLAY_PREEMPT_POINTS)
    printf("%ld, ", j);
    printf("\n");
#endif

#if defined(CHECK_PREEMPT_COST)
    totalPCost = 0;
    j = blockCount-1;
    while (j != 0)
    {
        printf("bCostArray[%ld] = %ld\n", j, bCostArray[j]);
        totalPCost += bCostArray[j];
	j = pPrevArray[j];
    }
    printf("The minimum preemption cost is: %ld\n", totalPCost);
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("npCostArray:\n");
    for (j=0; j<blockCount; j++)
    {
        printf("%ld, ", npCostArray[j]);
    }
    printf("\n");
#endif
    free(npCostArray);

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("pCostArray:\n");
    for (j=0; j<blockCount; j++)
    {
        printf("%ld, ", pCostArray[j]);
    }
    printf("\n");
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("bCostArray:\n");
    for (j=0; j<blockCount; j++)
    {
        printf("%ld, ", bCostArray[j]);
    }
    printf("\n");
#endif
    free(bCostArray);

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("pPrevArray:\n");
    for (j=0; j<blockCount; j++)
    {
	printf("%ld, ", pPrevArray[j]);
    }
    printf("\n");
#endif
    free(pPrevArray);

    return totalPCost;
}

cost_t optimalPPPlacement(char *taskName, index_t blockCount, cost_t maxTaskQI, cost_t *blockCycles, cost_t *pCostMatrix)
{
    index_t   accessIdx;
    cost_t  **bCostMatrix;
    index_t   j;
    index_t   k;
    index_t   m;
    index_t   n;
    cost_t    maxPreemptionCost;
    cost_t  **npCostMatrix;
    index_t   numPreemptions;
    index_t  *preemptionsArray;
    cost_t    pcost;
    index_t  *pPrevArray;
    cost_t  **qCostMatrix;
    index_t   s;
    cost_t    totalPCost;
	
    /* Allocate array pointers for the non-preemptive cost matrix. */
    npCostMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(npCostMatrix, 0, (blockCount * sizeof(cost_t *)));

    /* Allocate array pointers for the preemptive cost matrix. */
    qCostMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(qCostMatrix, 0, (blockCount * sizeof(cost_t *)));

    /* Allocate array pointers for the preemptive cost matrix. */
    bCostMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(bCostMatrix, 0, (blockCount * sizeof(cost_t *)));

    preemptionsArray = (index_t *) malloc(blockCount * sizeof(index_t));
    memset(preemptionsArray, 0, (blockCount * sizeof(index_t)));

    pPrevArray = (index_t *) malloc(blockCount * sizeof(index_t));
    memset(pPrevArray, 0, (blockCount * sizeof(index_t)));

    for (j=0; j<blockCount; j++)
    {
        npCostMatrix[j] = (cost_t *) malloc(blockCount * sizeof(cost_t));
        memset(npCostMatrix[j], 0, (blockCount * sizeof(cost_t)));
        qCostMatrix[j] = (cost_t *) malloc(blockCount * sizeof(cost_t));
        memset(qCostMatrix[j], 0, (blockCount * sizeof(cost_t)));
        bCostMatrix[j] = (cost_t *) malloc(blockCount * sizeof(cost_t));
        memset(bCostMatrix[j], 0, (blockCount * sizeof(cost_t)));
        if (blockCycles[j] <= maxTaskQI)
        {
            npCostMatrix[j][j] = blockCycles[j];
            accessIdx = (j * blockCount) + j;
            qCostMatrix[j][j] = blockCycles[j] + pCostMatrix[accessIdx];
	    pPrevArray[j] = 0;
            preemptionsArray[j] = 1;
        }
        else
        {
            accessIdx = (j * blockCount) + j;
            if (pCostMatrix[accessIdx] < maxTaskQI)
            {
                npCostMatrix[j][j] = blockCycles[j];
                qCostMatrix[j][j] = blockCycles[j] + pCostMatrix[accessIdx];
	        pPrevArray[j] = 0;
                preemptionsArray[j] = 1;
            }
            else
            {
#if 1
                printf("No feasible solution exists for task %s since blockCycles[%ld] = %ld > maxTaskQI of %ld\n",taskName,j,blockCycles[j],maxTaskQI);
                printf("No feasible solution exists for task %s since pCostMatrix[%ld][%ld] = %ld > maxTaskQI of %ld\n",taskName,j,j,pCostMatrix[accessIdx],maxTaskQI);
                return LC;
#endif
            }
        }
    }

    for (s=1; s<blockCount; s++)
    {
        for (j=0; j<blockCount-s; j++)
	{
	    k = j + s;
            npCostMatrix[j][k] = npCostMatrix[k][j] = npCostMatrix[j][k-1] + blockCycles[k];
#if defined(DEBUG_CODE)
	    printf("npCostMatrix[%ld][%ld]=%ld\n",j,k,npCostMatrix[j][k]);
#endif
            accessIdx = (j * blockCount) + k;
	    qCostMatrix[j][k] = qCostMatrix[k][j] = npCostMatrix[j][k] + pCostMatrix[accessIdx];
#if defined(DEBUG_CODE)
            printf("qCostMatrix[%ld][%ld]=%ld\n",j,k,qCostMatrix[j][k]);
#endif
	    if (qCostMatrix[j][k] <= maxTaskQI)
	    {
	        bCostMatrix[j][k] = qCostMatrix[j][k];
		pPrevArray[k] = j;
                preemptionsArray[j] = 1;
#if defined(DEBUG_CODE)
                printf("QI:bCostMatrix[%ld][%ld]=%ld\n",j,k,bCostMatrix[j][k]);
                printf("QI:pPrevArray[%ld]=%ld\n",k,pPrevArray[k]);
#endif
	    }
	    else
	    {
                if (k-j>1)
                {
	            bCostMatrix[j][k] = LC;
                    /* pPrevArray[k] = 0; */
                }
                else
                {
                    accessIdx = (j * blockCount) + k;
                    if (pCostMatrix[accessIdx] < maxTaskQI)
                    {
                        numPreemptions = ((qCostMatrix[j][k]-pCostMatrix[accessIdx])/(maxTaskQI-pCostMatrix[accessIdx])); 
                        numPreemptions = (((qCostMatrix[j][k]-pCostMatrix[accessIdx]) % (maxTaskQI-pCostMatrix[accessIdx])) != 0) ? numPreemptions+1 : numPreemptions;
                        qCostMatrix[j][k] = bCostMatrix[j][k] = (qCostMatrix[j][k]-pCostMatrix[accessIdx])/numPreemptions + pCostMatrix[accessIdx]; 
		        pPrevArray[k] = j;
                        preemptionsArray[j] = numPreemptions;
#if 0
                        printf("Task %s splitting large basic block with qCostMatrix[%ld][%ld] = %ld > maxTaskQI of %ld into %ld preemptions\n", taskName, j, k, qCostMatrix[j][k], maxTaskQI, numPreemptions);
#endif
                    }
                    else
                    {
	                bCostMatrix[j][k] = LC;
                        pPrevArray[k] = 0;
#if 0
                        printf("No feasible solution exists for task %s since pCostMatriX[%ld][%ld] = %ld > maxTaskQI of %ld for j=%ld and k=%ld\n", taskName, j, k, pCostMatrix[accessIdx], maxTaskQI, j, k);
#endif
#if 0
	                printf("npCostMatrix[%ld][%ld]=%ld\n",j,k,npCostMatrix[j][k]);
                        accessIdx = (j * blockCount) + k;
                        printf("pCostMatrix[%ld][%ld]=%ld\n",j,k,pCostMatrix[accessIdx]);
                        for (n = j; n <= k; n++)
                        {
                            printf("blockCycles[%ld] = %ld\n", n, blockCycles[n]);
                        }
#endif
                        return LC;
                    }
                }
	    }
	}
    }

    for (k=2; k<blockCount; k++)
    {
        for (j=k-1; ((j>=1)&&(bCostMatrix[j][k]<=maxTaskQI)); j--)
	{
#if defined(DEBUG_CODE)
            printf("Considering bCostMatrix[%ld][%ld]=%ld and bCostMatrix[%ld][%ld]=%ld\n",0,j,bCostMatrix[0][j],j,k,bCostMatrix[j][k]);
            printf("versus bCostMatrix[%ld][%ld]=%ld\n",0,k,bCostMatrix[0][k]);
#endif
            pcost = bCostMatrix[0][j] + bCostMatrix[j][k];
            if (pcost <= bCostMatrix[0][k])
	    {
	        bCostMatrix[0][k] = pcost;
   		pPrevArray[k] = j;
#if defined(DEBUG_CODE)
                printf("j=%ld\n",j);
                printf("P:bCostMatrix[%ld][%ld]=%ld\n",0,k,bCostMatrix[0][k]);
                printf("P:pPrevArray[%ld][%ld]=%ld\n",k,pPrevArray[k]);
#endif
	    }
	}
    }

#if defined(DISPLAY_PREEMPT_COST)
    printf("The minimum preemption cost is: %ld\n", bCostMatrix[0][blockCount-1]);
#endif

    totalPCost = 0;
#if defined(DISPLAY_PREEMPT_POINTS)
    printf("\n");
    printf("Selected Preemption Points:\n");
#endif
    j = blockCount-1;
    while (j != 0)
    {
#if defined(DISPLAY_PREEMPT_POINTS)
        printf("%ld, ", j);
#endif
        if ((bCostMatrix[pPrevArray[j]][j] < 0) || (bCostMatrix[pPrevArray[j]][j] >= LC))
        {
            printf("Invalid bCostMatrix[%ld][%ld]=%ld.\n", pPrevArray[j], j, bCostMatrix[pPrevArray[j]][j]);
        }
        if (preemptionsArray[pPrevArray[j]] <= 0)
        {
            printf("Invalid preemptions count %ld at index %ld\n", preemptionsArray[pPrevArray[j]], pPrevArray[j]);
        }
        totalPCost += (bCostMatrix[pPrevArray[j]][j] * preemptionsArray[pPrevArray[j]]);
	j = pPrevArray[j];
    }
#if defined(DISPLAY_PREEMPT_POINTS)
    printf("%ld, ", j);
    printf("\n");
#endif

#if defined(CHECK_PREEMPT_COST)
    totalPCost = 0;
    j = blockCount-1;
    while (j != 0)
    {
        printf("bCostMatrix[%ld][%ld] = %ld\n", pPrevArray[j], j, bCostMatrix[pPrevArray[j]][j]);
        totalPCost += bCostMatrix[pPrevArray[j]][j];
	j = pPrevArray[j];
    }
    printf("The minimum preemption cost is: %ld\n", totalPCost);
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("npCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_PREEMPT_POINT_DATA)
        for (k=0; k<blockCount; k++)
	{
	     printf("%ld, ", npCostMatrix[j][k]);
	}
#endif
        free(npCostMatrix[j]);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
	printf("\n");
#endif
    }
    free(npCostMatrix);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("\n");
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("pCostMatrix:\n");
    for (j=0; j<blockCount; j++)
    {
        for (k=0; k<blockCount; k++)
	{
            accessIdx = (j * blockCount) + k;
	    printf("%ld, ", pCostMatrix[accessIdx]);
	}
	printf("\n");
    }
    printf("\n");
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("qCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_PREEMPT_POINT_DATA)
        for (k=0; k<blockCount; k++)
	{
	    printf("%ld, ", qCostMatrix[j][k]);
	}
#endif
        free(qCostMatrix[j]);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
	printf("\n");
#endif
    }
    free(qCostMatrix);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("\n");
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("bCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_PREEMPT_POINT_DATA)
        for (k=0; k<blockCount; k++)
	{
	    printf("%ld, ", bCostMatrix[j][k]);
	}
#endif
        free(bCostMatrix[j]);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
	printf("\n");
#endif
    }
    free(bCostMatrix);
#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("\n");
#endif

#if defined(DISPLAY_PREEMPT_POINT_DATA)
    printf("pPrevArray:\n");
    for (j=0; j<blockCount; j++)
    {
	printf("%ld, ", pPrevArray[j]);
    }
    printf("\n");
#endif
    free(pPrevArray);

    return totalPCost;
}

/* Simulation of Bertogna's algorithm using our algorithm. */
cost_t optimalPPPlacementBertognaSimulated(char *taskName, index_t blockCount, cost_t maxTaskQI, cost_t *blockCycles, cost_t *pCostMatrix)
{
    cost_t    pcost;

    costMatrixBertognaSimulated(blockCount, pCostMatrix);

    pcost = optimalPPPlacement(taskName, blockCount, maxTaskQI, blockCycles, pCostMatrix);

    return pcost;
}

void testOptimalPPAlgorithm(void)
{
    index_t    accessIndex;
    index_t    blockCount;
    index_t    blockIndex;
    index_t    columnIndex;
    cost_t    *pCostMatrix;
    cost_t     totalPCost;

    blockCount = NI;
    pCostMatrix = (cost_t *) malloc(blockCount * blockCount * sizeof(cost_t));
    memset(pCostMatrix, 0, (blockCount * blockCount * sizeof(cost_t)));
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
        {
            accessIndex = (blockIndex * blockCount) + columnIndex;
            pCostMatrix[accessIndex] = e[blockIndex][columnIndex];
        }
    }
    totalPCost = optimalPPPlacement("test", blockCount, QI, bb, pCostMatrix);
    printf("The minimum preemption cost is: %ld\n", totalPCost);
    free(pCostMatrix);
    exit(1);
}

void testOptimalPPAlgorithmBertognaReal(void)
{
    index_t    blockCount;
    index_t    blockIndex;
    cost_t    *pCostArray;
    cost_t     totalPCost;

    blockCount = NIB;
    pCostArray = (cost_t *) malloc(blockCount * sizeof(cost_t));
    memset(pCostArray, 0, (blockCount * sizeof(cost_t)));
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        pCostArray[blockIndex] = eB[blockIndex];
    }
    totalPCost = optimalPPPlacementBertognaReal("test", blockCount, QIB, bbB, pCostArray);
    printf("The minimum preemption cost is: %ld\n", totalPCost);
    free(pCostArray);
    exit(1);
}

int loadTaskData(char *fileNamePrefix, index_t *blockCount, char ***blockNames, cost_t **blockCycles, cost_t **pCostMatrixPtr,
                 cost_t *maxUCBCount, cost_t blockReloadCycles)
{
    index_t      accessIndex;
    index_t      blockIndex;
    index_t      columnIndex;
    cost_t       currentBlocks;
    cost_t       currentCycles;
    char        *currentToken;
    cost_t      *dMatrix;
    char         fileName[80];
    FILE        *fptr;
    cost_t      *iMatrix;
    char         inputString[80];
    char        *line;
    index_t      lineIndex;
    char        *linePtr;
    cost_t       maxBlockCycles = 0;
    cost_t       minBlockCycles = LC;
    cost_t       maxPCostCycles = 0;
    cost_t       minPCostCycles = LC;
    cost_t       maxTaskQI;
    cost_t       totalNPRCycles;
    cost_t      *pCostMatrix;
    int          rc;
    char        *token;
    cost_t       totalPCost;
	
    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-ucb.txt");
	
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            /* printf("line: %s\n", line); */
            if (strlen(line) > 0)
            {
                /* printf("%s\n", line); */
                token = strtok(line, ":");
                token = strtok(NULL, ":");
                /* printf("%s\n", token); */ 
                *maxUCBCount = atol(token) * blockReloadCycles;
                free(line);
            }
            else
            {
                /* printf("Error 1 reading line 1 empty string\n"); */
            }
        }
	fclose(fptr);
#ifdef DEBUG_CODE
        printf("The maximum number of UCB is %ld\n", *maxUCBCount);
#endif
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }
	
    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-observed.txt");
	
    *blockCount = 0;
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            /* printf("line: %s\n", line); */
            if (strlen(line) > 0)
            {
                /* printf("%s\n", line); */
                free(line);
                (*blockCount)++;
            }
            else
            {
                /* printf("Error 1 reading line %ld empty string\n", ((*blockCount)+1)); */
            }
        }
        (*blockCount)++; /* Dummy basic block 0 */
	fclose(fptr);
#ifdef DEBUG_CODE
        printf("The number of basic blocks is %ld\n", *blockCount);
#endif
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }
	
    /* Allocate array pointers for the basic block names and cycles. */
    *blockNames = malloc(*blockCount * sizeof(char *));
    memset(*blockNames, 0, (*blockCount * sizeof(char *)));
    (*blockNames)[0] = malloc((strlen("dummy basic block")+1)*sizeof(char));
    strcpy((*blockNames)[0], "dummy basic block");
    *blockCycles = (cost_t *) malloc(*blockCount * sizeof(cost_t));
    memset(*blockCycles, 0, (*blockCount * sizeof(cost_t)));

    blockIndex = 1;
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if (strlen(line) > 0)
            {
                (*blockNames)[blockIndex] = malloc((strlen(line)+1)*sizeof(char));
                if ((*blockNames)[blockIndex] != NULL)
                {
                    strcpy((*blockNames)[blockIndex], line);
                    /* printf("%s\n", (*blockNames)[blockIndex]); */
                }
                blockIndex++;
                free(line);
            }
            else
            {
                /* printf("Error 2 reading line %ld empty string\n", (blockIndex+1)); */
            }
        }
	fclose(fptr);
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }

    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-cycles.txt");
	
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        totalNPRCycles = 0;
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if (strlen(line) > 0)
            {
                currentToken = strtok(line, " ");
                /* printf("Block name: %s\n", currentToken); */
                for (blockIndex = 1; blockIndex < *blockCount; blockIndex++)
                {
                    if ((strcmp((*blockNames)[blockIndex], currentToken) == 0) &&
                        ((*blockCycles)[blockIndex] == 0))
                    {
                        currentToken = strtok(NULL, " ");
                        currentCycles = atoi(currentToken);
                        totalNPRCycles += currentCycles;
                        /* printf("Cycles: %s\n", currentToken); */
                        (*blockCycles)[blockIndex] = currentCycles;
                        if ((*blockCycles)[blockIndex] < minBlockCycles)
                        {
                             minBlockCycles = (*blockCycles)[blockIndex];
                        }
                        if ((*blockCycles)[blockIndex] > maxBlockCycles)
                        {
                             maxBlockCycles = (*blockCycles)[blockIndex];
                        }
                        break;
                    }
                }
                free(line);
            }
        }
#ifdef DEBUG_CODE
        printf("The total non-preemptive execution cycles is %ld\n", totalNPRCycles);
#endif
	fclose(fptr);

#if 0
        for (blockIndex = 0; blockIndex < *blockCount; blockIndex++)
        {
            printf("Block: %s Cycles: %ld\n", (*blockNames)[blockIndex], (*blockCycles)[blockIndex]);
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }

    /* Allocate array pointers for the data cache cost matrix. */
    dMatrix = (cost_t *) malloc(*blockCount * *blockCount * sizeof(cost_t));
    memset(dMatrix, 0, (*blockCount * *blockCount * sizeof(cost_t)));

    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-dmatrix.txt");
	
    blockIndex = 1;
    lineIndex = 0;
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if ((strlen(line) > 0) && (lineIndex > 0))
            {
                linePtr = line;
                currentToken = strtok(linePtr, " ");
                linePtr += (strlen(currentToken) + 1);
                while (*linePtr == ' ')
                {
                    linePtr++;
                }
                columnIndex = blockIndex + 1;
                /* printf("Block %ld: ", blockIndex); */
                for (currentToken = strtok(linePtr," "); currentToken != NULL; currentToken = strtok(NULL, " "))
                {
                    /* printf("%s ", currentToken); */
                    currentBlocks = atoi(currentToken);
                    accessIndex = (blockIndex * *blockCount) + columnIndex;
                    dMatrix[accessIndex] = currentBlocks * blockReloadCycles;
                    accessIndex = (columnIndex * *blockCount) + blockIndex;
                    dMatrix[accessIndex] = currentBlocks * blockReloadCycles;
                    columnIndex++;
                }
                /* printf("Block %ld %s columnIndex = %ld\n",blockIndex,*blockNames[blockIndex],columnIndex); */
                /* printf("\n"); */
                blockIndex++;
                free(line);
            }
            lineIndex++;
        }
	fclose(fptr);
#if 0
        printf("dMatrix:\b");
        for (blockIndex = 0; blockIndex < *blockCount; blockIndex++)
        {
            printf("Block %s: ", (*blockNames)[blockIndex]);
            for (columnIndex = 0; columnIndex < *blockCount; columnIndex++)
            {
                accessIndex = (blockIndex * *blockCount) + columnIndex;
                printf("%ld ", dMatrix[accessIndex]);
            }
            printf("\n");
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }

    /* Allocate array pointers for the instruction cache cost matrix. */
    iMatrix = (cost_t *) malloc(*blockCount * *blockCount * sizeof(cost_t));
    memset(iMatrix, 0, (*blockCount * *blockCount * sizeof(cost_t)));

    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-imatrix.txt");
	
    blockIndex = 1;
    lineIndex = 0;
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if ((strlen(line) > 0) && (lineIndex > 0))
            {
                linePtr = line;
                currentToken = strtok(linePtr, " ");
                linePtr += (strlen(currentToken) + 1);
                while (*linePtr == ' ')
                {
                    linePtr++;
                }
                columnIndex = blockIndex + 1;
                /* printf("Block %ld: ", blockIndex); */
                for (currentToken = strtok(linePtr," "); currentToken != NULL; currentToken = strtok(NULL, " "))
                {
                    /* printf("%s ", currentToken); */
                    currentBlocks = atoi(currentToken);
                    accessIndex = (blockIndex * *blockCount) + columnIndex;
                    iMatrix[accessIndex] = currentBlocks * blockReloadCycles;
                    accessIndex = (columnIndex * *blockCount) + blockIndex;
                    iMatrix[accessIndex] = currentBlocks * blockReloadCycles;
                    columnIndex++;
                }
                /* printf("Block %ld %s columnIndex = %ld\n",blockIndex,*blockNames[blockIndex],columnIndex); */
                /* printf("\n"); */
                blockIndex++;
                free(line);
            }
            lineIndex++;
        }
	fclose(fptr);
#if 0
        printf("iMatrix:\n");
        for (blockIndex = 0; blockIndex < *blockCount; blockIndex++)
        {
            printf("Block %s: ", (*blockNames)[blockIndex]);
            for (columnIndex = 0; columnIndex < *blockCount; columnIndex++)
            {
                accessIndex = (blockIndex * *blockCount) + columnIndex;
                printf("%ld ", iMatrix[accessIndex]);
            }
            printf("\n");
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, returning...\n", fileName);
        return 0;
    }

    /* Allocate array pointers for the combined cache cost matrix. */
    pCostMatrix = (cost_t *) malloc((*blockCount) * (*blockCount) * sizeof(cost_t));
    *pCostMatrixPtr = pCostMatrix;
    memset(pCostMatrix, 0, ((*blockCount) * (*blockCount) * sizeof(cost_t)));

    for (blockIndex = 0; blockIndex < *blockCount; blockIndex++)
    {
        for (columnIndex = 0; columnIndex < *blockCount; columnIndex++)
        {
            accessIndex = (blockIndex * (*blockCount)) + columnIndex;
            pCostMatrix[accessIndex] = dMatrix[accessIndex] + iMatrix[accessIndex];
            /* printf("pCostMatrix[%ld[%ld] = %ld\n", blockIndex, columnIndex, pCostMatrix[accessIndex]); */
            if ((columnIndex > 0) && (blockIndex > 0) && (columnIndex > blockIndex))
            {
                if (pCostMatrix[accessIndex] < minPCostCycles)
                {
                    minPCostCycles = pCostMatrix[accessIndex];
                }
                if (pCostMatrix[accessIndex] > maxPCostCycles)
                {
                    maxPCostCycles = pCostMatrix[accessIndex];
                }
            }
        }
    }

#if 0
    printf("pCostMatrix:\n");
    for (blockIndex = 0; blockIndex < *blockCount; blockIndex++)
    {
        printf("Block %s: ", (*blockNames)[blockIndex]);
        for (columnIndex = 0; columnIndex < *blockCount; columnIndex++)
        {
            accessIndex = (blockIndex * (*blockCount)) + columnIndex;
            printf("%ld ", pCostMatrix[accessIndex]);
        }
        printf("\n");
    }
#endif

#ifdef DEBUG_CODE
    printf("Task %s\n", fileNamePrefix);
    printf("UCB cache block counts range is [%ld,%ld]\n",minPCostCycles/blockReloadCycles,maxPCostCycles/blockReloadCycles);
    printf("Non-preemptive cycles range is [%ld,%ld]\n",minBlockCycles,maxBlockCycles);
    printf("Preemptive cycles range is [%ld,%ld]\n",minPCostCycles,maxPCostCycles);
#endif

    free(dMatrix);
    free(iMatrix);

    return 1;
}

cost_t gcd(cost_t a, cost_t b)
{
    cost_t c;
    cost_t as = a;
    cost_t bs = b;

    while ( a != 0 ) 
    {
        c = a; 
        a = b%a;  
        b = c;
    }
    /* printf("GCD of %ld and %ld is %ld\n",as,bs,b); */

    return b;
}

cost_t lcm(cost_t x, cost_t y) 
{
    cost_t a;
    cost_t b;
    cost_t t;
    cost_t gcd;
    cost_t lcmm;
 
    a = x;
    b = y;
 
    while (b != 0) 
    {
        t = b;
        b = a % b;
        a = t;
    }
 
    gcd = a;
    lcmm = (x*y)/gcd;
 
    /* printf("Greatest common divisor of %ld and %ld = %ld\n", x, y, gcd); */
    /* printf("Least common multiple of %ld and %ld = %ld\n", x, y, lcmm); */
 
    return lcmm;
}

cost_t computeHyperPeriod(index_t noTasks, const cost_t *taskPeriods)
{
    cost_t  hyperPeriod = 1;
    index_t idx;

    for (idx=0; idx<noTasks; idx++)
    {
        hyperPeriod = lcm(hyperPeriod, taskPeriods[idx]);
    }
    return hyperPeriod;
}

cost_t getMaxDeadline(index_t noTasks, const cost_t *taskDeadlines)
{
    index_t idx;
    cost_t maxDeadline = INT_MIN;

    for (idx=0; idx<noTasks; idx++)
    {
        if (maxDeadline < taskDeadlines[idx])
        {
             maxDeadline = taskDeadlines[idx];
        }
    }
    return maxDeadline;
}

cost_t evaluateTaskDBF(cost_t t, cost_t p, cost_t d, cost_t e)
{
    cost_t value;

    /* printf("t:%ld p:%ld d:%ld e:%ld\n",t,p,d,e); */
    if (t >= d)
    {
        value = (((t - d)/p) + 1) * e;
    }
    else
    {
        value = 0;
    }

    return value;
}

cost_t getNextDeadline(cost_t dcurr, index_t noTasks, const cost_t *taskDeadlines, const cost_t *taskPeriods)
{
    cost_t  currDeadline;
    cost_t  nextDeadline = LC;
    index_t taskIdx;

    for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
    {
        currDeadline = ((dcurr / taskPeriods[taskIdx]) * taskPeriods[taskIdx]) + taskDeadlines[taskIdx];
        if ((nextDeadline > currDeadline) && (currDeadline > dcurr))
        {
            nextDeadline = currDeadline;
        }
    }
    return nextDeadline;
}

cost_t evaluateSlack(cost_t dk, index_t noTasks, const cost_t *taskPeriods, const cost_t *taskDeadlines, const cost_t *taskWCETs) 
{
    cost_t  dj = getNextDeadline(0, noTasks, taskDeadlines, taskPeriods);
    cost_t  currSlack;
    cost_t  minSlack = LC;
    cost_t  sum;
    cost_t  taskDBF;
    index_t taskIdx;

    while (dj <= dk)
    {
        sum = 0;
        for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
        {
            taskDBF = evaluateTaskDBF(dj, taskPeriods[taskIdx], taskDeadlines[taskIdx], taskWCETs[taskIdx]); 
            /* printf("t:%ld p:%ld d:%ld e:%ld dbf:%ld\n",dj,taskPeriods[taskIdx],taskDeadlines[taskIdx],taskWCETs[taskIdx],taskDBF); */
            sum += taskDBF; 
        }
        currSlack = dj - sum;  
        if (minSlack > currSlack)
        {
            minSlack = currSlack;
        }
        /* printf("t:%ld sum:%ld slack:%ld minSlack:%ld\n",dj, sum, currSlack, minSlack); */
        dj = getNextDeadline(dj, noTasks, taskDeadlines, taskPeriods);
    }

    return minSlack;
}

cost_t computeTaskNPRWCET(index_t noBlocks, cost_t *blockCycles)
{
    index_t blockIdx;
    cost_t  taskWCET = 0;

    for (blockIdx = 0; blockIdx < noBlocks; blockIdx++)
    {
        taskWCET += blockCycles[blockIdx];
    }
    return taskWCET;
}

pcnt_t computeTaskSetUtilization(index_t noTasks, cost_t *taskPeriods, cost_t *taskWCETs)
{
    index_t taskIdx;
    pcnt_t  taskUtilization = 0;

    for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
    {
        taskUtilization += (pcnt_t)taskWCETs[taskIdx]/(pcnt_t)taskPeriods[taskIdx];
    }
    return taskUtilization;
}

void testDeadlineGeneration(void)
{
    const index_t noTasks = 2;
    cost_t        currDeadline = 0;
    cost_t        slack;
    const cost_t  taskDeadlines[2] = {4, 25};
    const cost_t  taskPeriods[2]   = {4, 25};
    const cost_t  taskWCETs[2]   =   {2, 10};
    cost_t        taskDBF;
    index_t       taskIdx;

    currDeadline = getNextDeadline(currDeadline, noTasks, taskDeadlines, taskPeriods);
    while (currDeadline <= 40)
    {
        printf("Deadline: %ld\n", currDeadline);
        for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
        {
            taskDBF = evaluateTaskDBF(currDeadline, taskPeriods[taskIdx], taskDeadlines[taskIdx], taskWCETs[taskIdx]);
            printf("Task %ld: DBF: %ld\n", (taskIdx+1), taskDBF);
        }
        slack = evaluateSlack(currDeadline, noTasks, taskPeriods, taskDeadlines, taskWCETs);
        printf("Deadline %ld: slack: %ld\n", currDeadline, slack);
        currDeadline = getNextDeadline(currDeadline, noTasks, taskDeadlines, taskPeriods);
    }
}

void testNPRComputation(void)
{
    const index_t noTasks = 10;
    cost_t        currDeadline = 0;
    cost_t        hyperPeriod;
    cost_t        maxDeadline;
    cost_t        maxTaskQI[10];
    cost_t        slack;
    const cost_t  taskDeadlines[10] = {8, 10, 15, 30, 50, 50, 60, 60, 60, 100};
    const cost_t  taskPeriods[10]   = {8, 20, 25, 35, 50, 90, 110, 105, 100, 110};
    const cost_t  taskWCETs[10]   =   {2, 4, 2, 4, 3, 4, 8, 5, 3, 4};
    index_t       taskIdx;

    hyperPeriod = computeHyperPeriod(noTasks, taskPeriods);
    printf("Hyper Period: %ld\n", hyperPeriod);
    maxDeadline = 725;
    printf("Maximum Deadline: %ld\n", maxDeadline);
    currDeadline = getNextDeadline(currDeadline, noTasks, taskDeadlines, taskPeriods);
    while (currDeadline <= maxDeadline)
    {
        /* printf("Deadline: %ld\n", currDeadline); */
        slack = evaluateSlack(currDeadline, noTasks, taskPeriods, taskDeadlines, taskWCETs);
        printf("Deadline %ld: slack: %ld\n", currDeadline, slack);
        for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
        {
            if (taskDeadlines[taskIdx] == currDeadline)
            {
                maxTaskQI[taskIdx] = (taskWCETs[taskIdx] < slack) ? taskWCETs[taskIdx] : slack;
            }
        }
        currDeadline = getNextDeadline(currDeadline, noTasks, taskDeadlines, taskPeriods);
    }
    for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
    {
        printf("Task %ld: QI: %ld\n", (taskIdx+1), maxTaskQI[taskIdx]);
    }
}

#if 0
void checkTaskBasicBlockExecution(index_t noTasks, char **fileNamePrefixes, index_t *blockCount, cost_t **blockCycles, char ***blockNames, cost_t *maxTaskQI, cost_t *pCostMatrix[])
{
    const index_t SPLIT_SCALE_FACTOR = 2;

    index_t       accessIdx;
    index_t       accessIdx2;
    index_t       blockIdx;
    index_t       colIdx;
    index_t       currNameLen;
    index_t       lBlockIdx;
    cost_t        maxBBPCost;
    cost_t        minBBPCost;
    index_t       nBlockIdx;
    index_t       newBlocks;
    cost_t       *oldPCostMatrixPtr;
    index_t       pBlockIdx;
    index_t       pColIdx;
    cost_t       *pCostMatrixPtr;
    index_t       rcBlockIdx;
    index_t       taskIdx;
    cost_t        totalBBNPRCost;

    for (taskIdx = 0; taskIdx < noTasks; taskIdx++)
    {
#if 0
        printf("Checking task basic block cycles for task %ld: (%s) maxTaskQI = %ld\n", (taskIdx+1), 
                                                                      fileNamePrefixes[taskIdx], maxTaskQI[taskIdx]);
#endif

#if 0
        pCostMatrixPtr = pCostMatrix[taskIdx];
        printf("pCostMatrix[%ld]: %lx\n", taskIdx, pCostMatrixPtr);
        for (blockIdx = 0; blockIdx < blockCount[taskIdx]; blockIdx++)
        {
            printf("Block %s: ", blockNames[taskIdx][blockIdx]);
            for (colIdx = 0; colIdx < blockCount[taskIdx]; colIdx++)
            {
                accessIdx = (blockIdx * blockCount[taskIdx]) + colIdx;
                printf("%ld ", pCostMatrixPtr[accessIdx]);
            }
            printf("\n");
        }
#endif
        for (blockIdx = 0; blockIdx < blockCount[taskIdx]; blockIdx++)
        {
            maxBBPCost = 0;
            minBBPCost = LC;
            pCostMatrixPtr = pCostMatrix[taskIdx];
            for (lBlockIdx = blockIdx+1; lBlockIdx < blockCount[taskIdx]; lBlockIdx++)
            {
                accessIdx = (blockIdx * blockCount[taskIdx]) + lBlockIdx;
                if (pCostMatrixPtr[accessIdx] < LC)
                {
                    if (pCostMatrixPtr[accessIdx] > maxBBPCost) 
                    {
                        maxBBPCost = pCostMatrixPtr[accessIdx];
                    }
                    if (pCostMatrixPtr[accessIdx] < minBBPCost)
                    {
                        minBBPCost = pCostMatrixPtr[accessIdx];
                    }
                }
                else
                {
                    printf("Invalid pCostMatrix[%ld][%ld]=%ld, ", blockIdx, lBlockIdx, pCostMatrixPtr[accessIdx]);
                }
            }

            totalBBNPRCost = blockCycles[taskIdx][blockIdx];

            if ((totalBBNPRCost+maxBBPCost > (maxTaskQI[taskIdx]/SPLIT_SCALE_FACTOR)) && (totalBBNPRCost > SPLIT_SCALE_FACTOR) && (maxBBPCost < maxTaskQI[taskIdx]))
            {
#if 0
                printf("    Task %s basic block %s (%ld) with WCET of %ld PCOST of %ld exceeds the maximum task QI value of %ld\n", fileNamePrefixes[taskIdx], blockNames[taskIdx][blockIdx], blockIdx, totalBBNPRCost, maxBBPCost, maxTaskQI[taskIdx]);
#endif
#if 0
                printf("    Minimum preemption cost = %ld\n", minBBPCost);
                printf("    Maximum preemption cost = %ld\n", maxBBPCost);
#endif
                newBlocks = ((totalBBNPRCost+maxBBPCost)/((maxTaskQI[taskIdx]-maxBBPCost)/SPLIT_SCALE_FACTOR))*SPLIT_SCALE_FACTOR;
                newBlocks = (((totalBBNPRCost+maxBBPCost) % ((maxTaskQI[taskIdx]-maxBBPCost)/SPLIT_SCALE_FACTOR)) != 0) ? (newBlocks + 1) : newBlocks;
                newBlocks = (newBlocks > 0) ? (newBlocks - 1) : 0;
                if (newBlocks <= 0)
                {
                    printf("Invalid number of new blocks = %ld for blockIdx=%ld, NPR cost = %ld, P cost = %ld, QI = %ld\n", newBlocks, blockIdx, totalBBNPRCost, maxBBPCost, maxTaskQI[taskIdx]);
                    exit(1);
                }
#if 1
                blockCount[taskIdx] += newBlocks;

#if 0
                printf("Block Names\n");
#endif
                blockNames[taskIdx] = realloc(blockNames[taskIdx], ((blockCount[taskIdx]) * sizeof(char *)));
                if (blockNames[taskIdx] == NULL)
                {
                    printf("Error allocating blockNames array for task %lf\n", taskIdx);
                    exit(1);
                }
                /* Move the items to make room. */
                for (nBlockIdx = blockCount[taskIdx]-1; nBlockIdx > blockIdx; nBlockIdx--)
                {
                    blockNames[taskIdx][nBlockIdx] = blockNames[taskIdx][nBlockIdx-newBlocks];
                }
                /* Copy the names for the new blocks. */
                currNameLen = strlen(blockNames[taskIdx][blockIdx]);
                blockNames[taskIdx][blockIdx] = realloc(blockNames[taskIdx][blockIdx], ((currNameLen+1)*sizeof(char)));
                for (nBlockIdx = blockIdx+1; nBlockIdx <= blockIdx+newBlocks; nBlockIdx++)
                {
                    blockNames[taskIdx][nBlockIdx] = malloc((currNameLen+1)*sizeof(char));
                    if (blockNames[taskIdx][nBlockIdx] != NULL)
                    {
                        strcpy(blockNames[taskIdx][nBlockIdx], blockNames[taskIdx][blockIdx]);
                        /* printf("Block Names %s\n", blockNames[taskIdx][nBlockIdx]); */
                    }
                    else
                    {
                        printf("Error allocating blockNames array for task %lf at block %lf\n", taskIdx, blockIdx);
                        exit(1);
                    }
                }

#if 0
                printf("Block Cycles\n");
#endif
                blockCycles[taskIdx] = (cost_t *) realloc(blockCycles[taskIdx], (blockCount[taskIdx] * sizeof(cost_t)));
                if (blockCycles[taskIdx] == NULL)
                {
                    printf("Error allocating blockCycles array for task %lf\n", taskIdx);
                    exit(1);
                }
                /* Move the items to make room. */
                for (nBlockIdx = blockCount[taskIdx]-1; nBlockIdx > blockIdx; nBlockIdx--)
                {
                    blockCycles[taskIdx][nBlockIdx] = blockCycles[taskIdx][nBlockIdx-newBlocks];
                }
                /* Reduce the block cycles by the new amount. */
                for (nBlockIdx = blockIdx+1; nBlockIdx <= blockIdx+newBlocks; nBlockIdx++)
                {
                    blockCycles[taskIdx][nBlockIdx] = blockCycles[taskIdx][blockIdx]/(newBlocks+1);
#if 0
                    printf("Block Cycles %ld\n", blockCycles[taskIdx][nBlockIdx]);
#endif
                }
                blockCycles[taskIdx][blockIdx] = (blockCycles[taskIdx][blockIdx]/(newBlocks+1)) + 
                                                 (blockCycles[taskIdx][blockIdx] % (newBlocks+1));
#if 0
                printf("Block Cycles %ld\n", blockCycles[taskIdx][blockIdx]);
#endif
#if 0
                for (pBlockIdx = 0; pBlockIdx < blockCount[taskIdx]; pBlockIdx++)
                {
                    printf("Block: %s Cycles: %ld\n", blockNames[taskIdx][pBlockIdx], blockCycles[taskIdx][pBlockIdx]);
                }
#endif

#if 0
                printf("Preemption Cost Matrix\n");
                printf("blockIdx = %ld\n", blockIdx);
                printf("newBlocks = %ld\n", newBlocks);
                printf("blockCount = %ld\n", blockCount[taskIdx]);
#endif
                oldPCostMatrixPtr = pCostMatrix[taskIdx];
                pCostMatrix[taskIdx] = (cost_t *) malloc((blockCount[taskIdx] * blockCount[taskIdx] * sizeof(cost_t)));
                if (pCostMatrix[taskIdx] == NULL)
                {
                    printf("Error allocating pCostMatrix array for task %lf\n", taskIdx);
                    exit(1);
                }
                pCostMatrixPtr = pCostMatrix[taskIdx];
                /* Copy the old matrix contents into the new matrix contents. */
                for (rcBlockIdx = 0; rcBlockIdx < (blockCount[taskIdx]-newBlocks); rcBlockIdx++)
                {
                    for (nBlockIdx = 0; nBlockIdx < (blockCount[taskIdx]-newBlocks); nBlockIdx++)
                    {
                        accessIdx = (rcBlockIdx * blockCount[taskIdx]) + nBlockIdx;
                        accessIdx2 = (rcBlockIdx * (blockCount[taskIdx]-newBlocks)) + nBlockIdx;
                        pCostMatrixPtr[accessIdx] = oldPCostMatrixPtr[accessIdx2];
                    }
                }
                /* Move the row and column items to make room. */
                for (rcBlockIdx = 0; rcBlockIdx < (blockCount[taskIdx]-newBlocks); rcBlockIdx++)
                {
                    for (nBlockIdx = blockCount[taskIdx]-1; nBlockIdx >= (blockIdx+1+newBlocks); nBlockIdx--)
                    {
                        accessIdx = (rcBlockIdx * blockCount[taskIdx]) + nBlockIdx;
                        accessIdx2 = (rcBlockIdx * blockCount[taskIdx]) + nBlockIdx - newBlocks;
#if 0
                        printf("Moving %ld,%ld to %ld,%ld\n", rcBlockIdx, (nBlockIdx-newBlocks), rcBlockIdx, nBlockIdx);
#endif
                        pCostMatrixPtr[accessIdx] = pCostMatrixPtr[accessIdx2];
                    }
                }
                for (rcBlockIdx = 0; rcBlockIdx < blockCount[taskIdx]; rcBlockIdx++)
                {
                    for (nBlockIdx = blockCount[taskIdx]-1; nBlockIdx >= (blockIdx+1+newBlocks); nBlockIdx--)
                    {
                        accessIdx = (nBlockIdx * blockCount[taskIdx]) + rcBlockIdx;
                        accessIdx2 = ((nBlockIdx - newBlocks) * blockCount[taskIdx]) + rcBlockIdx;
#if 0
                        printf("Moving %ld,%ld to %ld,%ld\n", (nBlockIdx-newBlocks), rcBlockIdx, nBlockIdx, rcBlockIdx);
#endif
                        pCostMatrixPtr[accessIdx] = pCostMatrixPtr[accessIdx2];
                    }
                }
                /* Copy the preemption cost into the new entries. */
                for (rcBlockIdx = 0; rcBlockIdx < blockCount[taskIdx]; rcBlockIdx++)
                {
                    for (nBlockIdx = blockIdx+1; nBlockIdx <= blockIdx+newBlocks; nBlockIdx++)
                    {
                        accessIdx = (rcBlockIdx * blockCount[taskIdx]) + nBlockIdx;
                        accessIdx2 = (rcBlockIdx * blockCount[taskIdx]) + blockIdx;
#if 0
                        printf("Copying %ld,%ld to %ld,%ld\n", rcBlockIdx, blockIdx, rcBlockIdx, nBlockIdx);
#endif
                        pCostMatrixPtr[accessIdx] = pCostMatrixPtr[accessIdx2];
                    }
                }
                for (rcBlockIdx = 0; rcBlockIdx < blockCount[taskIdx]; rcBlockIdx++)
                {
                    for (nBlockIdx = blockIdx+1; nBlockIdx <= blockIdx+newBlocks; nBlockIdx++)
                    {
                        accessIdx = (nBlockIdx * blockCount[taskIdx]) + rcBlockIdx;
                        accessIdx2 = (blockIdx * blockCount[taskIdx]) + rcBlockIdx;
#if 0
                        printf("Copying %ld,%ld to %ld,%ld\n", blockIdx, rcBlockIdx, nBlockIdx, rcBlockIdx);
#endif
                        pCostMatrixPtr[accessIdx] = pCostMatrixPtr[accessIdx2];
                    }
                }
                /* Free the old preemption cost matrix. */
                free(oldPCostMatrixPtr);
#if 0
                printf("pCostMatrix:\n");
                for (pBlockIdx = 0; pBlockIdx < blockCount[taskIdx]; pBlockIdx++)
                {
                    for (pColIdx = 0; pColIdx < blockCount[taskIdx]; pColIdx++)
                    {
                        accessIdx = (pBlockIdx * blockCount[taskIdx]) + pColIdx;
                        printf("%ld ", pCostMatrixPtr[accessIdx]);
                    }
                    printf("\n");
                }
#endif
#if 0
                printf("Done Reallocating Memory\n");
#endif

#endif

#if 0
                printf("    Number of new basic block chunks needed = %ld\n", newBlocks);
#endif
                /* Decrement the current block index in order to check it again. */
                /* blockIdx--; */
            }
        }
    }
}
#endif

#define NUMBER_OF_TASKS      (10)
#define SCHEDULE_OK          (1)
#define SCHEDULE_NOTOK       (0)
#define INC_UTIL_PCNT        (1)
#define MIN_UTIL_PCNT        (1)
#define MAX_UTIL_PCNT        (100)
#define INC_BRT_CYCLES       (10)
#define MIN_BRT_CYCLES       (0)
#define MAX_BRT_CYCLES       (390)

int main(int argc, char *argv[])
{
    index_t     accessIndex;
    index_t     blockCount[NUMBER_OF_TASKS];
    index_t     blockIndex;
    char      **blockNames[NUMBER_OF_TASKS];
    cost_t     *blockCycles[NUMBER_OF_TASKS];
    cost_t      breakdownUtilization;
    cost_t      cacheBRTCycles;
    index_t     columnIndex;
    cost_t      currDeadline;
    char       *fileNamePrefixes[NUMBER_OF_TASKS] = {"fibcall", "recursion", "cover", "bsort100", "cnt", "crc", "ndes", "fft1",
                                                     "adpcm", "lms"};
    cost_t      hyperPeriod;
    char        inputString[80];
    int         loadStatus;
    cost_t      maxDeadline;
    cost_t      maxTaskQI[NUMBER_OF_TASKS];
    cost_t      maxUCBCount[NUMBER_OF_TASKS];
    cost_t     *pCostArray[NUMBER_OF_TASKS];
    cost_t     *pCostMatrix[NUMBER_OF_TASKS];
    cost_t     *pCostMatrixPtr;
    pcnt_t      preemptTaskUtilization;
    int         rc;
    cost_t      scaleFactor;
    int         schedulable;
    cost_t      slack;
    cost_t      targetTaskUtilization;
    index_t     taskIdx;
    cost_t      taskDeadlines[NUMBER_OF_TASKS];
    cost_t      taskPeriods[NUMBER_OF_TASKS];
    pcnt_t      taskSetUtilization;
    cost_t      taskWCETs[NUMBER_OF_TASKS];
    cost_t      totalPCost[NUMBER_OF_TASKS];
    cost_t      totalPOCost[NUMBER_OF_TASKS];
	
#if 0
    testOptimalPPAlgorithm();
    exit(1);
#endif

#if 0
    testOptimalPPAlgorithmBertognaReal();
    exit(1);
#endif

#if 0
    testDeadlineGeneration();
    exit(1);
#endif

#if 0
    testNPRComputation();
    exit(1);
#endif

    cacheBRTCycles = MAX_BRT_CYCLES;
    while (cacheBRTCycles > MIN_BRT_CYCLES)
    {
#if 0
        printf ("Processing with cache reload cycles = %ld\n", cacheBRTCycles);
#endif
        /* Start with a minumum working target task set utilization and work higher. */
        targetTaskUtilization = MIN_UTIL_PCNT;
        breakdownUtilization = 0;
        schedulable = SCHEDULE_OK;
        while ((schedulable == SCHEDULE_OK) && (targetTaskUtilization <= MAX_UTIL_PCNT))
        {
#if 0
            printf ("Processing with target utilization = %ld\n", targetTaskUtilization);
#endif

            scaleFactor = (100*NUMBER_OF_TASKS)/targetTaskUtilization;
            for (taskIdx=0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
            {
#if 0
                printf ("Loading task %s\n", fileNamePrefixes[taskIdx]);
#endif
                loadStatus = loadTaskData(fileNamePrefixes[taskIdx], &blockCount[taskIdx], &blockNames[taskIdx], 
                                          &blockCycles[taskIdx], &pCostMatrix[taskIdx], &maxUCBCount[taskIdx], cacheBRTCycles);
                if (loadStatus == 1)
                {	
#if 0
                    for (blockIndex = 0; blockIndex < blockCount[taskIdx]; blockIndex++)
                    {
                        printf("Block: %s Cycles: %ld\n", blockNames[taskIdx][blockIndex], blockCycles[blockIndex]);
                    }
#endif
                }
                else
                {
                    printf("Error loading task data for file name prefix %s, exiting...\n", fileNamePrefixes[taskIdx]);
                    exit(1);
                }

#if 0
                printf("pCostMatrix for task %s:\n", fileNamePrefixes[taskIdx]);
                printf("pCostMatrix[%ld]: %lx\n", taskIdx, pCostMatrix[taskIdx]);
                pCostMatrixPtr = pCostMatrix[taskIdx];
                for (blockIndex = 0; blockIndex < blockCount[taskIdx]; blockIndex++)
                {
                    printf("Block %s: ", blockNames[blockIndex]);
                    for (columnIndex = 0; columnIndex < blockCount[taskIdx]; columnIndex++)
                    {
                        accessIndex = (blockIndex * blockCount[taskIdx]) + columnIndex;
                        printf("%ld ", pCostMatrixPtr[accessIndex]);
                    }
                    printf("\n");
                }
#endif
            }

            /* Compute each tasks WCET. */ 
            for (taskIdx=0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
            {
                taskWCETs[taskIdx] = computeTaskNPRWCET(blockCount[taskIdx], blockCycles[taskIdx]);
                taskPeriods[taskIdx] = taskDeadlines[taskIdx] = scaleFactor*taskWCETs[taskIdx];
#if 0
                printf("Task %ld (%s): e,p,d (%ld,%ld,%ld)\n", (taskIdx+1), fileNamePrefixes[taskIdx], taskWCETs[taskIdx], 
                                                               taskPeriods[taskIdx], taskDeadlines[taskIdx]);
#endif
            }

            taskSetUtilization = computeTaskSetUtilization(NUMBER_OF_TASKS, taskPeriods, taskWCETs);
#if 0
            printf("Task Set utilization is %10.3lf \n", taskSetUtilization);
#endif

            /* Compute the maximum non-preemptive region parameter for each task. */
            hyperPeriod = computeHyperPeriod(NUMBER_OF_TASKS, taskPeriods);
#if 0
            printf("Hyper Period: %ld\n", hyperPeriod);
#endif
            maxDeadline = getMaxDeadline(NUMBER_OF_TASKS, taskDeadlines);
#if 0
            printf("Maximum Deadline: %ld\n", maxDeadline);
#endif
            currDeadline = 0;
            currDeadline = getNextDeadline(currDeadline, NUMBER_OF_TASKS, taskDeadlines, taskPeriods);
#if 0
            while (currDeadline <= maxDeadline)
            {
#endif
#if 0
                printf("Deadline: %ld\n", currDeadline);
#endif
                slack = evaluateSlack(currDeadline, NUMBER_OF_TASKS, taskPeriods, taskDeadlines, taskWCETs);
#if 0
                printf("Deadline %ld: slack: %ld\n", currDeadline, slack);
#endif
                for (taskIdx = 0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
                {
#if 0
                    if (taskDeadlines[taskIdx] == currDeadline)
                    {
                        maxTaskQI[taskIdx] = (taskWCETs[taskIdx] < slack) ? taskWCETs[taskIdx] : slack;
#endif
                        maxTaskQI[taskIdx] = slack;
#if 0
                    }
#endif
                }
#if 0
                currDeadline = getNextDeadline(currDeadline, NUMBER_OF_TASKS, taskDeadlines, taskPeriods);
            }
#endif

#if 0
            for (taskIdx = 0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
            {
                printf("Task %ld (%s): QI: %ld\n", (taskIdx+1), fileNamePrefixes[taskIdx], maxTaskQI[taskIdx]);
            }
            printf("QI: %ld\n", maxTaskQI[0]);
#endif

            /* Check to ensure the basic block BPR WCET execution time is within the maximum task QI parameter. */
#if 0
            printf("Checking the basic blocks for BRT of %ld and a utilization of %ld percent.\n", cacheBRTCycles, targetTaskUtilization);
            checkTaskBasicBlockExecution(NUMBER_OF_TASKS, fileNamePrefixes, blockCount, blockCycles, blockNames, 
                                         maxTaskQI, pCostMatrix);
#endif
#if 0
            printf("Done checking the basic blocks for BRT of %ld and a utilization of %ld percent.\n", cacheBRTCycles, targetTaskUtilization);
#endif

            /* Perform optimal preemption point placement on the loaded data. */
            schedulable = SCHEDULE_OK;
            for (taskIdx=0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
            {
                /* Perform optimal preemption point placement on the loaded data. */
#if 0
                printf("\nPlacing preemption points for task %ld (%s) with blockCount %ld QI %ld\n", (taskIdx+1), 
                       fileNamePrefixes[taskIdx], blockCount[taskIdx], maxTaskQI[taskIdx]);
#endif
#if defined(EPP_ALGORITHM)
                totalPCost[taskIdx] = optimalPPPlacement(fileNamePrefixes[taskIdx], blockCount[taskIdx], maxTaskQI[taskIdx], 
                                                         blockCycles[taskIdx], pCostMatrix[taskIdx]);
#endif
#if defined(BERTOGNA_ALGORITHM)
                costMatrixBertognaReal(blockCount[taskIdx], pCostMatrix[taskIdx], &pCostArray[taskIdx]);
                totalPCost[taskIdx] = optimalPPPlacementBertognaReal(fileNamePrefixes[taskIdx], blockCount[taskIdx], maxTaskQI[taskIdx], 
                                                                     blockCycles[taskIdx], pCostArray[taskIdx]);
#endif
                /* Bound the preemption cost by the task UCB count times the cache block reload cycles. */
                if ((totalPCost[taskIdx] < LC) && (totalPCost[taskIdx] > 0))
                {
                    totalPOCost[taskIdx] = totalPCost[taskIdx] - taskWCETs[taskIdx];
                    if (totalPOCost[taskIdx] > maxUCBCount[taskIdx])
                    {
#if 0
                        printf("The maximum UCB cost (%ld) for task %ld (%s) is exceeded: %ld\n", maxUCBCount[taskIdx], (taskIdx+1), 
                               fileNamePrefixes[taskIdx], totalPCost[taskIdx]);
#endif
                        totalPCost[taskIdx] = totalPCost[taskIdx] - totalPOCost[taskIdx] + maxUCBCount[taskIdx];
#if 0
                        printf("The adjusted WCET cost for task %ld (%s) is %ld\n", (taskIdx+1), fileNamePrefixes[taskIdx], 
                                                                                totalPCost[taskIdx]);
#endif
                    }
                }

                if ((totalPCost[taskIdx] >= LC) || (totalPCost[taskIdx] < 0) || (totalPCost[taskIdx] > taskDeadlines[taskIdx]))
                {
                    schedulable = SCHEDULE_NOTOK;
#if 0
                    printf("The minimum WCET cost for task %ld (%s) is: %ld\n", (taskIdx+1), fileNamePrefixes[taskIdx], 
                                                                                totalPCost[taskIdx]);
#endif
                }
                else
                {
                    if (totalPCost[taskIdx] > taskWCETs[taskIdx])
                    {
#if 0
                        printf("The minimum WCET cost for task %ld (%s) is: %ld\n", (taskIdx+1), fileNamePrefixes[taskIdx], 
                                                                                    totalPCost[taskIdx]);
                        printf("    Task NPR WCET is %ld, task preemption cost is %ld\n", taskWCETs[taskIdx], 
                                                                                (totalPCost[taskIdx]-taskWCETs[taskIdx]));
#endif
                    }
                }
            }

            if (schedulable == SCHEDULE_OK)
            {
                preemptTaskUtilization = computeTaskSetUtilization(NUMBER_OF_TASKS, taskPeriods, totalPCost);
#if 0
                printf("Task Set utilization with preemption is %10.3lf \n", preemptTaskUtilization);
#endif

                if (preemptTaskUtilization > 1.0)
                {
                    schedulable = SCHEDULE_NOTOK;
                }
            }

            if (schedulable == SCHEDULE_OK)
            {
                breakdownUtilization = targetTaskUtilization;
#if 0
                printf("The taskset is SCHEDULABLE for a utilization of %ld percent.\n", breakdownUtilization);
#endif
            }
            else
            {
#if 0
                printf("The taskset is NOT SCHEDULABLE for a utilization of %ld percent.\n", targetTaskUtilization);
#endif
            }

            /* Clean up allocated memory. */
#if 0
            printf("Clean up allocated memory.\n");
#endif
            for(taskIdx=0; taskIdx < NUMBER_OF_TASKS; taskIdx++)
            {
#if 0
                printf("Clean up allocated memory for task %ld.\n", taskIdx);
#endif
                for (blockIndex = 0; blockIndex < blockCount[taskIdx]; blockIndex++)
                {
#if 0
                    printf("Block name pointer %ld %lx\n", blockIndex, blockNames[taskIdx][blockIndex]);
#endif
                    free(blockNames[taskIdx][blockIndex]);
                }
#if 0
                printf("Preemption cost pointer %lx\n", pCostMatrix[taskIdx]);
#endif
#if defined(BERTOGNA_ALGORITHM)
                free(pCostArray[taskIdx]);
#endif
                free(pCostMatrix[taskIdx]);
#if 0
                printf("Block cycles pointer %lx\n", blockCycles[taskIdx]);
#endif
                free(blockCycles[taskIdx]);
#if 0
                printf("Block names pointer %lx\n", blockNames[taskIdx]);
#endif
                free(blockNames[taskIdx]);
            }
#if 0
            printf("Done cleaning up allocated memory.\n");
#endif

            targetTaskUtilization += INC_UTIL_PCNT;
        }

        printf("BRT: %ld U: %4.2lf\n", cacheBRTCycles, (double)((double)(breakdownUtilization+1)/(double)100.0)); 
#if 0
        printf("The breakdown utilization for cache block reload cycles of %ld is: %ld percent\n", cacheBRTCycles, 
               breakdownUtilization+1);
#endif

        cacheBRTCycles -= INC_BRT_CYCLES;
    }
    exit(1);
}

#if 0
    rc = getLine ("Enter the task maximum non-preemptive region parameter: ", inputString, sizeof(inputString));
    if (rc == NO_INPUT) 
    {
        /* Extra NL since my system doesn't output that on EOF. */
        printf ("\nNo input\n");
        return 1;
    }

    if (rc == TOO_LONG) 
    {
        printf ("Input too long [%s]\n", inputString);
        return 1;
    }

    printf ("OK [%s]\n", inputString);
	
    maxTaskQI[taskIdx] = 0;
    maxTaskQI[taskIdx] = atoi(inputString);
    maxTaskQI[taskIdx] = (maxTaskQI[taskIdx] > 0) ? maxTaskQI[taskIdx] : 100;

    /* Perform optimal preemption point placement on the loaded data. */
    totalPCost[taskIdx] = optimalPPPlacement("test", blockCount[taskIdx], maxTaskQI[taskIdx], blockCycles[taskIdx], pCostMatrix[taskIdx]);
    printf("The minimum WCET cost is: %ld\n", totalPCost[taskIdx]);
#endif
