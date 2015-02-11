#include <stdio.h>
#include <stdlib.h>

#define NI (7)
#define QI (12)
#define LC (65535)

typedef unsigned int cost_t;
typedef unsigned int mask_t;
typedef unsigned int index_t;

static const cost_t bb[NI] = {0,3,2,2,2,2,3};
static const cost_t e[NI][NI] = {
    {0, 1, 2, 4, 4, 3, 2},
	{1, 1, 3, 5, 5, 4, 3},
	{2, 3, 2, 6, 6, 5, 4},
	{4, 5, 6, 4, 6, 7, 6},
	{4, 5, 6, 6, 4, 5, 6},
	{3, 4, 5, 7, 5, 3, 5},
	{2, 3, 4, 6, 6, 5, 2}
};
static cost_t np[NI][NI] = {0};
static cost_t q[NI][NI] = {0};
static cost_t b[NI][NI] = {0};
static mask_t pb[NI][NI] = {0};

int main(int argc, char *argv[])
{
    index_t j;
	index_t k;
	index_t m;
	cost_t  pcost;
	
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
           if (k != (j+1))
		   {
		       np[j][k] = np[j][k-1] + bb[k];
		   }
		   else
		   {
		       np[j][k] = bb[k];
		   }
           printf("np[%d][%d]=%d\n",j,k,np[j][k]);
		   q[j][k] = np[j][k] + e[j][k];
           printf("q[%d][%d]=%d\n",j,k,q[j][k]);
		   if (q[j][k] <= QI)
		   {
		       b[j][k] = q[j][k];
               printf("b[%d][%d]=%d\n",j,k,b[j][k]);
			   pb[j][k] = (1<<j)+(1<<k);
               printf("pb[%d][%d]=%d\n",j,k,pb[j][k]);
		   }
		   else
		   {
		       b[j][k] = LC;
			   pb[j][k] = 0;
		   }
		   for (m=j+1; m<k; m++)
		   {
               if ((q[j][m] <= QI) && (q[m+1][k] <= QI))
		       {
			       pcost = q[j][m] + q[m+1][k];
				   if (pcost <=  b[j][k])
				   {
		               b[j][k] = pcost;
                       printf("b[%d][%d]=%d\n",j,k,b[j][k]);
			           pb[j][k] = pb[j][m]+pb[m+1][k];
                       printf("pb[%d][%d]=%d\n",j,k,pb[j][k]);
				   }
			   }
		   }
		}
	}
    printf("np matrix:\n");
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
		    printf("%d, ", np[j][k]);
		}
		printf("\n");
	}
    printf("\n");
    printf("e matrix:\n");
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
		    printf("%d, ", e[j][k]);
		}
		printf("\n");
	}
    printf("\n");
    printf("q matrix:\n");
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
		    printf("%d, ", q[j][k]);
		}
		printf("\n");
	}
    printf("\n");
    printf("b matrix:\n");
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
		    printf("%d, ", b[j][k]);
		}
		printf("\n");
	}
    printf("\n");
    printf("pb matrix:\n");
    for (j=0; j<NI; j++)
	{
	    for (k=j+1; k<NI; k++)
		{
		    printf("%d, ", pb[j][k]);
		}
		printf("\n");
	}
    printf("\n");
    return 0;
}
