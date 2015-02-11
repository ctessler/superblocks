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
#define LC (INT_MAX)
/* #define DEBUG_CODE */
/* #define DISPLAY_MATRICES */
#define DISPLAY_PREEMPT_POINTS
/* #define DISPLAY_PREEMPT_COST */
/* #define CHECK_PREEMPT_COST */

typedef unsigned int cost_t;
typedef unsigned int index_t;

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

cost_t optimalPPPlacement(index_t blockCount, cost_t maxTaskQI, cost_t * blockCycles, cost_t **pCostMatrix)
{
    cost_t  **bCostMatrix;
    index_t   j;
    index_t   k;
    index_t   m;
    cost_t  **npCostMatrix;
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
            qCostMatrix[j][j] = blockCycles[j] + pCostMatrix[j][j];
	    pPrevArray[j] = 0;
        }
        else
        {
            printf("No feasible solution exists for this task since blockCycles[%d] = %d > maxTaskQI of %d\n",j,blockCycles[j], maxTaskQI);
            return LC;
        }
    }

    for (s=1; s<blockCount; s++)
    {
        for (j=0; j<blockCount-s; j++)
	{
	    k = j + s;
            npCostMatrix[j][k] = npCostMatrix[k][j] = npCostMatrix[j][k-1] + blockCycles[k];
#if defined(DEBUG_CODE)
	    printf("npCostMatrix[%d][%d]=%d\n",j,k,npCostMatrix[j][k]);
#endif
	    qCostMatrix[j][k] = qCostMatrix[k][j] = npCostMatrix[j][k] + pCostMatrix[j][k];
#if defined(DEBUG_CODE)
            printf("qCostMatrix[%d][%d]=%d\n",j,k,qCostMatrix[j][k]);
#endif
	    if (qCostMatrix[j][k] <= maxTaskQI)
	    {
	        bCostMatrix[j][k] = qCostMatrix[j][k];
		pPrevArray[k] = j;
#if defined(DEBUG_CODE)
                printf("QI:bCostMatrix[%d][%d]=%d\n",j,k,bCostMatrix[j][k]);
                printf("QI:pPrevArray[%d]=%d\n",k,pPrevArray[k]);
#endif
	    }
	    else
	    {
                if (j-k>1)
                {
	            bCostMatrix[j][k] = LC;
                    pPrevArray[k] = 0;
                }
                else
                {
                    printf("No feasible solution exists for this task since qCostMatriX[%d][%d] = %d > maxTaskQI of %d for j=%d and k=%d\n", j, k, qCostMatrix[j][k], maxTaskQI, j, k);
                    return LC;
                }
	    }
	}
    }

    for (k=2; k<blockCount; k++)
    {
        for (j=k-1; ((j>=1)&&(bCostMatrix[j][k]<=maxTaskQI)); j--)
	{
#if defined(DEBUG_CODE)
            printf("Considering bCostMatrix[%d][%d]=%d and bCostMatrix[%d][%d]=%d\n",0,j,bCostMatrix[0][j],j,k,bCostMatrix[j][k]);
            printf("versus bCostMatrix[%d][%d]=%d\n",0,k,bCostMatrix[0][k]);
#endif
            pcost = bCostMatrix[0][j] + bCostMatrix[j][k];
            if (pcost <= bCostMatrix[0][k])
	    {
	        bCostMatrix[0][k] = pcost;
   		pPrevArray[k] = j;
#if defined(DEBUG_CODE)
                printf("j=%d\n",j);
                printf("P:bCostMatrix[%d][%d]=%d\n",0,k,bCostMatrix[0][k]);
                printf("P:pPrevArray[%d][%d]=%d\n",k,pPrevArray[k]);
#endif
	    }
	}
    }

#if defined(DISPLAY_PREEMPT_COST)
    printf("The minimum preemption cost is: %d\n", bCostMatrix[0][blockCount-1]);
#endif

#if defined(DISPLAY_PREEMPT_POINTS)
    totalPCost = 0;
    printf("\n");
    printf("Selected Preemption Points:\n");
    j = blockCount-1;
    while (j != 0)
    {
        printf("%d, ", j);
        totalPCost += bCostMatrix[pPrevArray[j]][j];
	j = pPrevArray[j];
    }
    printf("%d, ", j);
    printf("\n");
#endif

#if defined(CHECK_PREEMPT_COST)
    totalPCost = 0;
    j = blockCount-1;
    while (j != 0)
    {
        printf("bCostMatrix[%d][%d] = %d\n", pPrevArray[j], j, bCostMatrix[pPrevArray[j]][j]);
        totalPCost += bCostMatrix[pPrevArray[j]][j];
	j = pPrevArray[j];
    }
    printf("The minimum preemption cost is: %d\n", totalPCost);
#endif

#if defined(DISPLAY_MATRICES)
    printf("npCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_MATRICES)
        for (k=0; k<blockCount; k++)
	{
	     printf("%d, ", npCostMatrix[j][k]);
	}
#endif
        free(npCostMatrix[j]);
#if defined(DISPLAY_MATRICES)
	printf("\n");
#endif
    }
    free(npCostMatrix);
#if defined(DISPLAY_MATRICES)
    printf("\n");
#endif

#if defined(DISPLAY_MATRICES)
    printf("pCostMatrix:\n");
    for (j=0; j<blockCount; j++)
    {
        for (k=0; k<blockCount; k++)
	{
	    printf("%d, ", pCostMatrix[j][k]);
	}
	printf("\n");
    }
    printf("\n");
#endif

#if defined(DISPLAY_MATRICES)
    printf("qCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_MATRICES)
        for (k=0; k<blockCount; k++)
	{
	    printf("%d, ", qCostMatrix[j][k]);
	}
#endif
        free(qCostMatrix[j]);
#if defined(DISPLAY_MATRICES)
	printf("\n");
#endif
    }
    free(qCostMatrix);
#if defined(DISPLAY_MATRICES)
    printf("\n");
#endif

#if defined(DISPLAY_MATRICES)
    printf("bCostMatrix:\n");
#endif
    for (j=0; j<blockCount; j++)
    {
#if defined(DISPLAY_MATRICES)
        for (k=0; k<blockCount; k++)
	{
	    printf("%d, ", bCostMatrix[j][k]);
	}
#endif
        free(bCostMatrix[j]);
#if defined(DISPLAY_MATRICES)
	printf("\n");
#endif
    }
    free(bCostMatrix);
#if defined(DISPLAY_MATRICES)
    printf("\n");
#endif

#if defined(DISPLAY_MATRICES)
    printf("pPrevArray:\n");
    for (j=0; j<blockCount; j++)
    {
	printf("%d, ", pPrevArray[j]);
    }
    printf("\n");
#endif
    free(pPrevArray);

    return totalPCost;
}

int main(int argc, char *argv[])
{
    index_t    blockCount;
    index_t    blockIndex;
    char     **blockNames;
    cost_t    *blockCycles;
    index_t    columnIndex;
    cost_t     currentCycles;
    char      *currentToken;
    cost_t   **dMatrix;
    char       fileName[80];
    char       fileNamePrefix[80];
    FILE      *fptr;
    cost_t   **iMatrix;
    char       inputString[80];
    char      *line;
    index_t    lineIndex;
    char      *linePtr;
    cost_t     maxBlockCycles = 0;
    cost_t     minBlockCycles = LC;
    cost_t     maxPCostCycles = 0;
    cost_t     minPCostCycles = LC;
    cost_t     maxTaskQI;
    cost_t   **pCostMatrix;
    int        rc;
    cost_t     totalPCost;
	
#if 0
    blockCount = NI;
    pCostMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(pCostMatrix, 0, (blockCount * sizeof(cost_t *)));
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        pCostMatrix[blockIndex] = (cost_t *) malloc(blockCount * sizeof(cost_t));
        memset(pCostMatrix[blockIndex], 0, (blockCount * sizeof(cost_t)));
        for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
        {
            pCostMatrix[blockIndex][columnIndex] = e[blockIndex][columnIndex];
        }
    }
    totalPCost = optimalPPPlacement(blockCount, QI, bb, pCostMatrix);
    printf("The minimum preemption cost is: %d\n", totalPCost);
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        free(pCostMatrix[blockIndex]);
    }
    free(pCostMatrix);
    exit(1);
#endif

    rc = getLine ("Enter the file name prefix to process:  ", fileNamePrefix, sizeof(fileNamePrefix));
    if (rc == NO_INPUT) 
    {
        /* Extra NL since my system doesn't output that on EOF. */
        printf ("\nNo input\n");
        return 1;
    }

    if (rc == TOO_LONG) 
    {
        printf ("Input too long [%s]\n", fileNamePrefix);
        return 1;
    }

    printf ("OK [%s]\n", fileNamePrefix);
	
    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-observed.txt");
	
    blockCount = 0;
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
                blockCount++;
            }
            else
            {
                /* printf("Error 1 reading line %d empty string\n", (blockCount+1)); */
            }
        }
        blockCount++; /* Dummy basic block 0 */
	fclose(fptr);
        printf("The number of basic blocks is %d\n", blockCount);
    }
    else
    {
        printf("Unable to open input file %s, exiting...\n", fileName);
        exit(1);
    }
	
    /* Allocate array pointers for the basic block names and cycles. */
    blockNames = malloc(blockCount * sizeof(char *));
    blockNames[0] = malloc((strlen("dummy basic block")+1)*sizeof(char));
    strcpy(blockNames[0], "dummy basic block");
    blockCycles = (cost_t *) malloc(blockCount * sizeof(cost_t));
    memset(blockCycles, 0, (blockCount * sizeof(cost_t)));

    blockIndex = 1;
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if (strlen(line) > 0)
            {
                blockNames[blockIndex] = malloc((strlen(line)+1)*sizeof(char));
                if (blockNames[blockIndex] != NULL)
                {
                    strcpy(blockNames[blockIndex], line);
                    /* printf("%s\n", blockNames[blockIndex]); */
                }
                blockIndex++;
                free(line);
            }
            else
            {
                /* printf("Error 2 reading line %d empty string\n", (blockIndex+1)); */
            }
        }
	fclose(fptr);
    }
    else
    {
        printf("Unable to open input file %s, exiting...\n", fileName);
        exit(1);
    }

    strcpy(fileName, fileNamePrefix);
    strcat(fileName, "-cycles.txt");
	
    fptr = fopen(fileName, "r");
    if (fptr != NULL)
    {
        while (!feof(fptr)) 
	{
            line = readFileLine(fptr);
            if (strlen(line) > 0)
            {
                currentToken = strtok(line, " ");
                /* printf("Block name: %s\n", currentToken); */
                for (blockIndex = 1; blockIndex < blockCount; blockIndex++)
                {
                    if ((strcmp(blockNames[blockIndex], currentToken) == 0) &&
                        (blockCycles[blockIndex] == 0))
                    {
                        currentToken = strtok(NULL, " ");
                        currentCycles = atoi(currentToken);
                        /* printf("Cycles: %s\n", currentToken); */
                        blockCycles[blockIndex] = currentCycles;
                        if (blockCycles[blockIndex] < minBlockCycles)
                        {
                             minBlockCycles = blockCycles[blockIndex];
                        }
                        if (blockCycles[blockIndex] > maxBlockCycles)
                        {
                             maxBlockCycles = blockCycles[blockIndex];
                        }
                        break;
                    }
                }
                free(line);
            }
        }
	fclose(fptr);

#if 0
        for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
        {
            printf("Block: %s Cycles: %d\n", blockNames[blockIndex], blockCycles[blockIndex]);
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, exiting...\n", fileName);
        exit(1);
    }

    /* Allocate array pointers for the data cache cost matrix. */
    dMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(dMatrix, 0, (blockCount * sizeof(cost_t *)));
    dMatrix[0] = (cost_t *) malloc(blockCount * sizeof(int));
    memset(dMatrix[0], 0, (blockCount * sizeof(cost_t)));

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
                dMatrix[blockIndex] = (cost_t *) malloc(blockCount * sizeof(int));
                memset(dMatrix[blockIndex], 0, (blockCount * sizeof(cost_t)));
                columnIndex = blockIndex + 1;
                /* printf("Block %d: ", blockIndex); */
                for (currentToken = strtok(linePtr," "); currentToken != NULL; currentToken = strtok(NULL, " "))
                {
                    /* printf("%s ", currentToken); */
                    currentCycles = atoi(currentToken);
                    dMatrix[blockIndex][columnIndex] = currentCycles;
                    columnIndex++;
                }
                /* printf("Block %d %s columnIndex = %d\n",blockIndex,blockNames[blockIndex],columnIndex); */
                /* printf("\n"); */
                blockIndex++;
                free(line);
            }
            lineIndex++;
        }
	fclose(fptr);
#if 0
        printf("dMatrix:\b");
        for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
        {
            printf("Block %s: ", blockNames[blockIndex]);
            for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
            {
                printf("%d ", dMatrix[blockIndex][columnIndex]);
            }
            printf("\n");
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, exiting...\n", fileName);
        exit(1);
    }

    /* Allocate array pointers for the instruction cache cost matrix. */
    iMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(iMatrix, 0, (blockCount * sizeof(cost_t *)));
    iMatrix[0] = (cost_t *) malloc(blockCount * sizeof(int));
    memset(iMatrix[0], 0, (blockCount * sizeof(cost_t)));

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
                iMatrix[blockIndex] = (cost_t *) malloc(blockCount * sizeof(cost_t));
                memset(iMatrix[blockIndex], 0, (blockCount * sizeof(cost_t)));
                columnIndex = blockIndex + 1;
                /* printf("Block %d: ", blockIndex); */
                for (currentToken = strtok(linePtr," "); currentToken != NULL; currentToken = strtok(NULL, " "))
                {
                    /* printf("%s ", currentToken); */
                    currentCycles = atoi(currentToken);
                    iMatrix[blockIndex][columnIndex] = currentCycles;
                    columnIndex++;
                }
                /* printf("Block %d %s columnIndex = %d\n",blockIndex,blockNames[blockIndex],columnIndex); */
                /* printf("\n"); */
                blockIndex++;
                free(line);
            }
            lineIndex++;
        }
	fclose(fptr);
#if 0
        printf("iMatrix:\b");
        for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
        {
            printf("Block %s: ", blockNames[blockIndex]);
            for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
            {
                printf("%d ", iMatrix[blockIndex][columnIndex]);
            }
            printf("\n");
        }
#endif
    }
    else
    {
        printf("Unable to open input file %s, exiting...\n", fileName);
        exit(1);
    }

    /* Allocate array pointers for the combined cache cost matrix. */
    pCostMatrix = (cost_t **) malloc(blockCount * sizeof(cost_t *));
    memset(pCostMatrix, 0, (blockCount * sizeof(cost_t *)));
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        pCostMatrix[blockIndex] = (cost_t *) malloc(blockCount * sizeof(cost_t));
        memset(pCostMatrix[blockIndex], 0, (blockCount * sizeof(cost_t)));
    }

    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
        {
            pCostMatrix[blockIndex][columnIndex] = dMatrix[blockIndex][columnIndex] + iMatrix[blockIndex][columnIndex];
            if (pCostMatrix[blockIndex][columnIndex] < minPCostCycles)
            {
                minPCostCycles = pCostMatrix[blockIndex][columnIndex];
            }
            if (pCostMatrix[blockIndex][columnIndex] > maxPCostCycles)
            {
                maxPCostCycles = pCostMatrix[blockIndex][columnIndex];
            }
            pCostMatrix[columnIndex][blockIndex] = pCostMatrix[blockIndex][columnIndex];
        }
        free(dMatrix[blockIndex]);
        free(iMatrix[blockIndex]);
    }
    free(dMatrix);
    free(iMatrix);

#if 0
    printf("pCostMatrix:\b");
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        printf("Block %s: ", blockNames[blockIndex]);
        for (columnIndex = 0; columnIndex < blockCount; columnIndex++)
        {
            printf("%d ", pCostMatrix[blockIndex][columnIndex]);
        }
        printf("\n");
    }
#endif

    printf("Non-preemptive cycles range is [%d,%d]\n",minBlockCycles,maxBlockCycles);
    printf("Preemptive cycles range is [%d,%d]\n",minPCostCycles,maxPCostCycles);

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
	
    maxTaskQI = 0;
    maxTaskQI = atoi(inputString);
    maxTaskQI = (maxTaskQI > 0) ? maxTaskQI : 100;

    /* Perform optimal preemption point placement on the loaded data. */
    totalPCost = optimalPPPlacement(blockCount, maxTaskQI, blockCycles, pCostMatrix);
    printf("The minimum preemption cost is: %d\n", totalPCost);

    /* CLean up allocated memory. */
    for (blockIndex = 0; blockIndex < blockCount; blockIndex++)
    {
        free(pCostMatrix[blockIndex]);
        free(blockNames[blockIndex]);
    }
    free(pCostMatrix);
    free(blockCycles);
    free(blockNames);
    exit(1);
}
