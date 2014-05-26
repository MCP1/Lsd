#include "fun_head.h"


object *supply;
MODELBEGIN







EQUATION("Init")
/*
Initialize the lattice:

- If PlotLattice !=0 then generate the lattice window

- Set the pointers to the neighbours of each cell

- If the cells have been assigned some product (generally this is not the case), store the sum for each product in "tempSales".
*/

v[10]=V("PlotLattice");

v[2]=V("NCol");
v[3]=V("NRow");
supply=SEARCHS(p->up,"Supply");

if(v[10]==1)
 init_lattice(600, v[3], v[2], "IdRow", "IdCol", "State", NULL, 0);
if(v[10]==2)
 init_lattice(300, v[3], v[2], "IdRow", "IdCol", "IdUsed", NULL, 0);

CYCLE(cur, "Row")
 {
  v[0]=VS(cur,"IdRow");

  CYCLES(cur, cur1, "Col")
  { 
   v[11]=VS(cur1,"IdUsed");
   if(v[11]>0)
    {
    cur2=SEARCH_CNDS(supply,"IdProd",v[11]);
    INCRS(cur2,"tempSales",1);
    }
   v[1]=VS(cur1,"IdCol");
   cur1->hook=supply;
   cur2=SEARCHS(cur1,"Nei"); //first neigh, E

   if(v[1]==v[2])
     {
     cur3=SEARCH_CNDS(cur,"IdCol",1); 
     }
   else
     {
     cur3=SEARCH_CNDS(cur,"IdCol",v[1]+1); 
     }
    cur2->hook=cur3;
    
   cur2=cur2->next; //second neigh, S

   if(v[0]==v[3])
     {
     cur3=SEARCH_CND("IdRow",1); 
     cur4=SEARCH_CNDS(cur3,"IdCol",v[1]);
     }
   else
     {
     cur3=SEARCH_CND("IdRow",v[0]+1); 
     cur4=SEARCH_CNDS(cur3,"IdCol",v[1]);

     }
    cur2->hook=cur4;
    
   cur2=cur2->next; //third neigh, W

   if(v[1]==1)
     {
     cur3=SEARCH_CNDS(cur,"IdCol",v[2]); 
     }
   else
     {
     cur3=SEARCH_CNDS(cur,"IdCol",v[1]-1); 
     }
    cur2->hook=cur3;


   cur2=cur2->next; //fourth neigh, N

   if(v[0]==1)
     {
     cur3=SEARCH_CND("IdRow",v[3]); 
     cur4=SEARCH_CNDS(cur3,"IdCol",v[1]);
     }
   else
     {
     cur3=SEARCH_CND("IdRow",v[0]-1); 
     cur4=SEARCH_CNDS(cur3,"IdCol",v[1]);

     }
    cur2->hook=cur4;

   }

      
  }

 
PARAMETER
RESULT( 1)

EQUATION("ActionDemand")
/*
Ensure that all cells computed their own "State" variable
*/
V("Init");
CYCLE(cur, "Row")
 {
  CYCLES(cur, cur1, "Col")
   {
    VS(cur1,"State");
   }

 }

RESULT(1 )


EQUATION("State")
/*
State can assume three different values: 0, 1 and 2.

If it is 0 it does nothing. State=0 means that the cell never used a product, nor it has been triggered to act (i.e. no neighbour uses a product.

State=1 means that the agent did not have a product, but the previous period a neighbour bought a product. Therefore, chooses a product (which one is decided by "Choose") and triggers the state of neighbours to be 1, subject to the neighbour has the "Flag" value of 0. Flag is 0 if you did never bought a product, or more than TimeReActivate steps have passed since last purchase.

State=2 means that the agent is currently using a product, and no neighbours had triggered it to make a new purchase.
*/
v[1]=0;
v[0]=VL("State",1);

if(v[0]==1)
 {
  //Triggered to act
  v[1]=VS(p->hook,"Choose");
  if(v[1]>0)
   {
   CYCLE(cur, "Nei")
    {
     v[5]=VS(cur->hook,"Flag");
     if(v[5]==0)
      {WRITES(cur->hook,"Flag",1); 
       WRITELS(cur->hook,"State",1, t);
      } 
    }
    WRITEL("IdUsed",v[1], 1);
    v[6]=V("IdRow");
    v[7]=V("IdCol");
   if(V("PlotLattice")==2)
    update_lattice(v[6], v[7], v[1]);
   if(V("PlotLattice")==1)
    update_lattice(v[6], v[7], 2);
    
   v[12]=V("TimeReActivate");
   WRITE("Flag",v[12]);
   v[1]=2;
   }
  else
   {WRITE("Flag",0);
    if(V("IdUsed")>0)
     v[1]=2;
    else 
     v[1]=0;
   } 

 
 }
else
 {
  if(v[0]==2 )
   {
    v[10]=V("Flag");
    if(v[10]<0)
     v[10]=INCR("Flag",1);
    v[13]=V("ProbReActivate");
 
    if(v[10]==0 && RND<v[13])
     v[1]=1;
    else
     v[1]=2; 
    
   }
  else
   v[1]=v[0];  
  } 
 if(v[1]==0 && V("IdUsed")>0)
  v[1]=INTERACT("BAH", v[1]); 
RESULT( v[1])


FUNCTION("Choose")
/*
Choose the product to buy for the calling cell.

The function scans all neighbours of the calling cell. When it finds the nei. that has chosen its product the previous period, then stores within the "Prod" objects (parameter "app") some probabilities.

The objects assigned positive probabilities are: the one chosen by the triggering nei.; and those around it for a lenght of "NumTail" (the total is always an odd number). Boundaries conditions are accounted for, so to keep the number of total objects with positive probabilities positive.

The concerned "Prod" are discarded if their PP is lesser than the q of the choosing agent. That is, a product to be considered needs to have a quality above the consumer's threshold.

Notice that if the consumer has two triggering neighbours, then it can choose among a double-size option set.

The probabioities assigned to each producer to be chosen are proportional to the PP of the producer raised to the power of alpha. Setting alpha to 0 assigns identical probabilities.

For statistical purposes the parameter "tempSales" of the newly chosen firm is increased, and that of the discarded product, if exist, is decreased.

*/

v[0]=VS(c,"q");


CYCLE(cur, "Prod")
 {
  WRITES(cur,"app",0);
  v[1]++;
 }


v[13]=V("alpha");
v[7]=0;
v[1]=V("NumProducers");
v[8]=V("NumTail");
i=(int)v[8]*2+1;
CYCLES(c,cur, "Nei")
 {
  v[2]=VLS(cur->hook,"State",2);
  v[3]=VLS(cur->hook,"State",1);
  if( v[2]==1 && v[3]==2)
   {
    v[4]=VS(cur->hook,"IdUsed");
    v[10]=v[4]-v[8];
    v[11]=v[4]+v[8];
    if(v[10]<1)
     {
      v[11]-=v[10]+1;
      v[10]=1;
     }    
    if(v[11]>v[1])
     {
      v[10]-=(v[11]-v[1]);
      v[11]-=(v[11]-v[1]);
     }
    cur1=SEARCH_CND("IdProd",v[10]);
    
    for( j=0; j<i; j++)
     {
      v[5]=VS(cur1,"PP");
      if(v[5]>v[0])
      {
       v[6]=pow(v[5],v[13]);
       INCRS(cur1,"app",v[6]);
       v[7]++;
      }
      cur1=go_brother(cur1);
     } 

   }//end of triggering neighours
  }//end of the scanning of neighborus

if(v[7]>0)
 {

 cur=RNDDRAW("Prod","app");
 v[4]=VS(cur,"IdProd");
 INCRS(cur,"tempSales",1);
 v[8]=VS(c,"IdUsed");
 if(v[8]>0)
  {
  cur1=SEARCH_CND("IdProd",v[8]);
  INCRS(cur1,"tempSales",-1);
  
  }

 }
else
 v[4]=0; 
RESULT(v[4] )

//5809250 I-A

EQUATION("NumProducers")
/*
Compute the number of producers currently in the model.
*/
v[0]=0;
CYCLE(cur, "Prod")
 v[0]++;
RESULT(v[0] )



EQUATION("PP")
/*
Quality of the product, increasing with the number of consumers.

The function is:
              [1-PP(0)]
PP(t)= 1 - ----------------
            [1+k*N(t-1)]^a
            
The function increases (with decreasing steps) from PP(0) (initial PP at time 0) up to 1 for N going to infinite. K and a influence the speed.
*/

v[0]=VL("Users",1);
v[1]=V("pp0");
v[2]=V("a");
v[3]=V("k");

v[4]=1+v[3]*v[0];

v[5]=1-(1-v[1])*1/pow(v[4],v[2]);

RESULT(v[5])

EQUATION("Users")
/*
UPdate the statistics of the number of users.

To speed things up, the computation is indirect. Previous period users are increased of the parameter "tempSales", which is the reset for the next period computation. Current net sales (new consumers minus abandoning ones) are stored in Sales.
*/
VS(p->up->next,"ActionDemand");
v[0]=VL("Users",1);
v[1]=V("tempSales");

WRITE("Sales",v[1]);
WRITE("tempSales",0);
RESULT(v[0]+v[1] )


EQUATION("CopyRow")
/*
Copy of the IdRow placed in Cell, to improve speed.
*/
v[0]=V("IdRow");
PARAMETER
RESULT(v[0] )


EQUATION("Entry")
/*
Entry of new producers, allowed in every PeriodEntry time steps
*/

v[0]=VL("Entry",1);

if(v[0]<0)
 END_EQUATION(v[0]+1)
/*
CYCLE_SAFE(cur, "Prod")
 {
 v[5]=VS(cur,"Users");
 if(v[5]==0)
  DELETE(cur);
 }
*/  
 
cur=SEARCH("Prod");
v[3]=VS(cur,"pp0");//pp0 of the very first product
cur=ADDOBJ_EX("Prod",cur);
v[2]=V("IssueIdProd");
WRITES(cur,"IdProd",v[2]);  


WRITELS(cur,"PP",v[3],t);
WRITES(cur,"pp0",v[3]);
WRITELS(cur,"Users",0,t);
WRITES(cur,"Sales",0);

v[4]=V("PeriodEntry");

RESULT(v[4] )

FUNCTION("IssueIdProd")
/*
Deliver the IdProd for entrants
*/

RESULT(val[0]+1 )



MODELEND




void close_sim(void)
{

}


