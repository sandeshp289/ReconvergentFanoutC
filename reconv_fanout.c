/////////////////////////////////////////////////////////////////////////////////////
// File       : reconv_fanout.c
// Language   : C/C++
// Description: Determine all re-convergence gates specific to a gate number.
// Author     : Sandesh Prabhakar
// Created    : August 28, 2009
// Modified   : August 20, 2026
// Version    : 1.0
//
// Copyright (c) 2026 Sandesh Prabhakar
// All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum
{
   G_JUNK,         /* 0 */
   G_INPUT,        /* 1 */
   G_OUTPUT,       /* 2 */
   G_XOR,          /* 3 */
   G_XNOR,         /* 4 */
   G_DFF,          /* 5 */
   G_AND,          /* 6 */
   G_NAND,         /* 7 */
   G_OR,           /* 8 */
   G_NOR,          /* 9 */
   G_NOT,          /* 10 */
   G_BUF,          /* 11 */
};

struct node
{
	int data;
        int number; 
	node *link;
};

////////////////////////////////////////////////////////////////////////
// circuit class
////////////////////////////////////////////////////////////////////////

class circuit
{
    // circuit information
    int numgates;       // total number of gates (faulty included)
    int maxlevels;      // number of levels in gate level ckt
    int numInputs;	// number of inputs
    int numOutputs;	// number of outputs
    unsigned char *gtype;	// gate type
    short *fanin;		// number of fanins of gate
    short *fanout;		// number of fanouts of gate
    int *levelNum;		// level number of gate
    int **faninlist;		// fanin list of gate
    int **fanoutlist;		// fanout list of gate

    // you may wish to add more private variables here
    int *visited; 
    node **fanin_schedule;
    node *storegates;

public:
    circuit(char *);            // constructor

    // you may wish to add more function declarations (or public variables) here
    void dfs(int i, int gate, int pin_number, int* visited, short* fanout, int** fanoutlist); 
    void additem_fanin (int i, int gate, int pin_number);
    void set_intersection (int i, int j);
    void add_gates (int data);
    int check_storegates (int data);
    
    
};

////////////////////////////////////////////////////////////////////////
// constructor: reads in the *.lev file and builds basic data structure
// 		for the gate-level ckt
////////////////////////////////////////////////////////////////////////
circuit::circuit(char *cktName)
{
    FILE *inFile;
    char fName[40];
    int i, j, count;
    char c;
    int gatenum, junk;
    int f1, f2, f3;

    //added by Sandesh for project0
    int signal, gate;

    strcpy(fName, cktName);
    strcat(fName, ".lev");
    inFile = fopen(fName, "r");
    if (inFile == NULL)
    {
	fprintf(stderr, "Can't open .lev file\n");
	exit(-1);
    }

    numgates = maxlevels = 0;
    numInputs = numOutputs = 0;

    fscanf(inFile, "%d", &count);	// number of gates
    fscanf(inFile, "%d", &junk);	// skip the second line

    // allocate space for gates data structure
    gtype = new unsigned char[count];
    fanin = new short[count];
    fanout = new short[count];
    levelNum = new int[count];
    faninlist = new int * [count];
    fanoutlist = new int * [count];
   
    fanin_schedule = new node*[2000]; 
    storegates=new node;
    storegates=NULL;

    for(i=0; i<2000;i++)
      fanin_schedule[i]=NULL;

    //added by Sandesh for project0
    visited = new int[count];
    for (j=0; j<count; j++)
	{
         visited[j] = 0;
	}

    // now read in the circuit
    for (i=1; i<count; i++)
    {
	fscanf(inFile, "%d", &gatenum);
	fscanf(inFile, "%d", &f1);
	fscanf(inFile, "%d", &f2);
	fscanf(inFile, "%d", &f3);

	numgates++;
	gtype[gatenum] = (unsigned char) f1;
	if (gtype[gatenum] > 13)
	    printf("gate %d is an unimplemented gate type\n", gatenum);
	else if (gtype[gatenum] == G_INPUT)
	    numInputs++;
	else if (gtype[gatenum] == G_OUTPUT)
	    numOutputs++;

	f2 = (int) f2;
	levelNum[gatenum] = f2;

	if (f2 >= (maxlevels))
	    maxlevels = f2 + 5;

	fanin[gatenum] = (int) f3;
	// now read in the faninlist
	faninlist[gatenum] = new int[fanin[gatenum]];
	for (j=0; j<fanin[gatenum]; j++)
	{
	    fscanf(inFile, "%d", &f1);
	    faninlist[gatenum][j] = (int) f1;
	}

	for (j=0; j<fanin[gatenum]; j++) // followed by samethings
	    fscanf(inFile, "%d", &junk);

	// read in the fanout list
	fscanf(inFile, "%d", &f1);
	fanout[gatenum] = (int) f1;

	fanoutlist[gatenum] = new int[fanout[gatenum]];
	for (j=0; j<fanout[gatenum]; j++)
	{
	    fscanf(inFile, "%d", &f1);
	    fanoutlist[gatenum][j] = (int) f1;
	}

	// skip till end of line
	while ((c = getc(inFile)) != '\n' && c != EOF)
	    ;
    }	// for (i...)
    fclose(inFile);

    printf("Successfully read in circuit:\n");
    printf("\t%d PIs.\n", numInputs);
    printf("\t%d POs.\n", numOutputs);
    printf("\t%d total number of gates\n", numgates);
    printf("\t%d levels in the circuit.\n", maxlevels / 5);
    
    signal=0;
    
for(signal=1;signal<=numgates;signal++)  
 {
   int pin_number=0;
   if(fanout[signal]>1)
   {
    for(i=0;i<fanout[signal];i++)
    {
       for (j=1; j<count; j++)
	{
         visited[j] = 0;
	}
      gate=fanoutlist[signal][i]; 
          for(int k =0;k<fanin[gate];k++)
	  {
		if(faninlist[gate][k]==signal)
                   {
                    pin_number = k;
                    break;
		   }
	  }
      if(gtype[gate]!=5&&gtype[gate]!=2)
      dfs(i, gate, pin_number, visited, fanout, fanoutlist);
    }

    for(i=0;i<fanout[signal]-1;i++)
    	{
         for(j=i+1;j<fanout[signal];j++)
	 {
          set_intersection(i,j);
	 }
    	}

   printf("\n Reconvergent gates for signal #%d are:\n",signal); 
     
        node *newlink=storegates;
          while(newlink!=NULL)
	  {
           printf(" %d ",newlink->data);

           newlink=newlink->link;
	  }
        printf ("\n");
   }
   else
   {
   printf ("\n No Reconvergent gates exists for signal #%d \n",signal); 
   }

  for(i=0; i<2000;i++)
      fanin_schedule[i]=NULL;
  storegates=NULL;
 }    
      	    
}

////////////////////////////////////////////////////////////////////////
// other member functions here
////////////////////////////////////////////////////////////////////////

           //Depth First Search (DFS)
	   void circuit::dfs(int i, int gate, int pin_number, int* visited, short* fanout, int** fanoutlist)
	   	   {
                    int j, k; 
                    int pin_number_temp=0;
                    int gate_temp;
                    visited[gate] = 1;
                    additem_fanin(i,gate, pin_number);

                    for(j=0; j<fanout[gate]; j++)
                       {

                      gate_temp=fanoutlist[gate][j];
                      for(k =0;k<fanin[gate_temp];k++)
		       {
			if(faninlist[gate_temp][k]==gate)
                   		{
               			     pin_number_temp = k;
		                    break;
		   		}
	  		}
                       if(visited[gate_temp]==0)
		       {
                        if(gtype[gate_temp]!=5&&gtype[gate_temp]!=2)
                        dfs(i, gate_temp, pin_number_temp, visited, fanout, fanoutlist);
		       }
                       }
                    }


void circuit::additem_fanin (int i, int gate, int pin_number)

  {
	node *newlink = new node;
  	newlink->data = gate;
        newlink->number = pin_number;
	newlink->link = fanin_schedule[i];
        fanin_schedule[i] = newlink;
  }             

void circuit::set_intersection (int i, int j)

  {
	node *set1 = fanin_schedule[i];
        
             while(set1!=NULL)
	     {
                node *set2 = fanin_schedule[j];
                   while(set2!=NULL)
		   {
                     if((set1->data==set2->data)&&(set1->number!=set2->number)) 
                        {
                         int flag=0;
                         flag=check_storegates(set1->data);
                         if(flag==0)
                         add_gates(set1->data);
			}
                     set2=set2->link;
		   }
              set1=set1->link;
	     }
  }  

void circuit::add_gates (int data)
  	{
	node *newlink = new node;
	newlink->data = data;
	newlink->link = storegates;
        storegates = newlink;
	}

int circuit::check_storegates (int data)
  	{

	node *newlink = storegates;

          while(newlink!=NULL)
	    {
               if(newlink->data==data)
                 return 1;

             newlink=newlink->link;
	    }

        return 0;
	
	}

////////////////////////////////////////////////////////////////////////
// main starts here
////////////////////////////////////////////////////////////////////////
main(int argc, char *argv[])
{
    char cktName[20];
    circuit *ckt;
    int vecWidth, poNum;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <ckt>\n", argv[0]);
        exit(-1);
    }

    strcpy(cktName, argv[1]);

    // read in the circuit and build data structure
    ckt = new circuit(cktName);

    // NOW ADD YOUR FUNCTION HERE
}

