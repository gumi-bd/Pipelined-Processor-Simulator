/*
Authors: Aditya Patil(CS20B004)||Karan Bardhan (CS20B036)
Main File
*/
#include <bits/stdc++.h>
#include <climits>
#include <iomanip>
#include "processor.h"

using namespace std;
int main()
{
    ifstream ICache,DCache,Regfile;

    ICache.open("ICache.txt");

    DCache.open("DCache.txt");

    Regfile.open("RF.txt");

    Processor  spp(&ICache,&DCache,&Regfile);
    spp.run();

    spp.store_data("ODCache.txt","Output.txt");
}

