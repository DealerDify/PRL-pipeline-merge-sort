/*
PRL projekt 1 - PipeLine Merge Sort
autor: Havlicek Lukas (xhavli46)
*/

#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <queue>
#include <iostream>
#include <fstream>
#include <math.h>

#define TAG 0
#define NUM_PROCS 5//musi byt log2 (NUM_INPUT) +1
#define NUM_INPUT 16

void master()
{
   std::queue<unsigned> master_q;

   std::ifstream file("numbers");
   unsigned x;
   int cnt = 0;
   while(file.good())
   {
      x = file.get(); //nacteni 1 bytu
      if(!file.good())
         break;//eof
      master_q.push(x);
      cnt++;
   }
   if(cnt!= NUM_INPUT)
   {//nespravny pocet cisel na vstupu -> konec
      MPI_Abort(MPI_COMM_WORLD,MPI_ERR_COUNT);
      return;
   }
   while(!master_q.empty())
   {
      x = master_q.front();
      master_q.pop();
      if(master_q.empty())
      {//posledni prvek je vytisknut bez mezery + novy radek
         printf("%d\n", x);
      }
      else
      {
         printf("%d ", x);
      }
      MPI_Send(&x, 1, MPI_UNSIGNED, 1, TAG, MPI_COMM_WORLD);//zasleme dalsimu procesoru cisla (id 1)
   }

}

void other_procs(int myid,int numprocs, int n)
{
   
   MPI_Status stat;
   std::queue<unsigned> q1;
   std::queue<unsigned> q2;
   int q1_len = 0;
   int q2_len = 0;
   int max_q_len = pow(2.0,myid-1);//velikost fronty
   int q1_out = max_q_len;
   int q2_out = max_q_len;
   bool add_to_1 = true;
   bool out = false;
   for(int i = 0; i < n ; i++)
   {//for pres N prvku (kazdy procesor musi tolik prvku prijmout, ale protoze se neodesila hned od zacatku, tak je odesilani zbytku dokonceno za forem)
      unsigned x;
      MPI_Recv(&x, 1, MPI_UNSIGNED, myid-1, TAG, MPI_COMM_WORLD, &stat);//prijmeme od predchoziho procesoru
      if(add_to_1)
      {
         q1.push(x);
         q1_len++;
         if(q1_len >= max_q_len)
         {//naplnila se prvni fronta, tak zacneme plnit druhou
            add_to_1 = false;
            q1_len=0;
         }
      }
      else
      {//prvni fronta byla plna, tak pridavame prvky zde
         if(!out)
            out=true;//prijimame prvni prvek, po zaplneni prvni fronty, tj zaciname i odesilat prvky
         q2.push(x);
         q2_len++;
         if(q2_len >= max_q_len)
         {//druha fronta prijala vsechny sva cisla, zacneme davat zase do prvni
            add_to_1 = true;
            q2_len=0;
         }
         
      }
      if(out && (numprocs-1)!=myid )
      {//posledni procesor uz dal neposila, ale tiskne(reseno za forem)
         if((q1_out > 0 && q1.front() < q2.front()) || q2_out ==0)
         {//pokud je druha fronta prazdna nebo pokud je prvek na prvni fronte mensi
            x = q1.front();
            q1.pop();
            MPI_Send(&x, 1, MPI_UNSIGNED, myid+1, TAG, MPI_COMM_WORLD);//zasleme dalsimu procesoru
            q1_out--;
         }
         else
         {//pokud je prvni fronta prazdna nebo pokud je prvek na druhe fronte vetsi
            x = q2.front();
            q2.pop();
            MPI_Send(&x, 1, MPI_UNSIGNED, myid+1, TAG, MPI_COMM_WORLD);//zasleme dalsimu procesoru
            q2_out--;
         }

         if(q1_out == 0 && q2_out == 0)
         {//odeslaly se vsechny cisla z front
            q1_out = max_q_len;
            q2_out = max_q_len;
            out = false;
         }
      }
   }//end for

   if((numprocs-1)==myid)
   {//posleni procesor prijal vsechna cisla, tak je vytiskneme serazene
      while(out)
      {
         unsigned x;
         if((q1_out > 0 && q1.front() < q2.front()) || q2_out ==0)
         {
            x = q1.front();
            q1.pop();
            printf("%d\n",x);//posledni procesor uz dal nezasila, ale tiskne
            q1_out--;
         }
         else
         {
            x = q2.front();
            q2.pop();
            printf("%d\n",x);//posledni procesor uz dal nezasila, ale tiskne
            q2_out--;
         }

         if(q1_out == 0 && q2_out == 0)
         {//odeslaly se vsechny cisla z front
            q1_out = max_q_len;
            q2_out = max_q_len;
            out = false;
         }
      }
   }
   else
   {//nejsme posledni procesor, tj prijala se vsechna data ale zatim se vsechna neodeslala
      while(out)
      {
         unsigned x;
         if((q1_out > 0 && q1.front() < q2.front()) || q2_out ==0)
         {//pokud je druha fronta prazdna nebo pokud je prvek na prvni fronte mensi
            x = q1.front();
            q1.pop();
            MPI_Send(&x, 1, MPI_UNSIGNED, myid+1, TAG, MPI_COMM_WORLD);//zasleme dalsimu procesoru
            q1_out--;
         }
         else
         {//pokud je prvni fronta prazdna nebo pokud je prvek na druhe fronte vetsi
            x = q2.front();
            q2.pop();
            MPI_Send(&x, 1, MPI_UNSIGNED, myid+1, TAG, MPI_COMM_WORLD);//zasleme dalsimu procesoru
            q2_out--;
         }

         if(q1_out == 0 && q2_out == 0)
         {//odeslaly se vsechny cisla z front
            q1_out = max_q_len;
            q2_out = max_q_len;
            out = false;
         }
      }
   }
   
}

int main(int argc, char *argv[])
{
   int numprocs;
   int myid;
   int i;
   MPI_Status stat;

   MPI_Init(&argc, &argv);                   //init
   MPI_Comm_size(MPI_COMM_WORLD, &numprocs); //pocet procesu
   MPI_Comm_rank(MPI_COMM_WORLD, &myid);     //id sveho procesu

   if (myid == 0)
   {
      if(numprocs != NUM_PROCS)
      {//nespravny pocet procesoru -> konec
         MPI_Abort(MPI_COMM_WORLD,MPI_ERR_COUNT);
         return 1;
      }
      master();
   }
   else
   {
      other_procs(myid,numprocs,16);//radime 16 cisel
   }

   MPI_Finalize();
   return 0;
}
