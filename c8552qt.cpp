//====================================================================
//
// (c) Borna Noureddin
// COMP 8552   British Columbia Institute of Technology
// Assignment 2
// main driver to test Quadtree
//
//====================================================================

#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <math.h>
#include "quadtree.h"

using namespace std;


int main()
{
    cout << "***** COMP 8552 assignment 2" << endl;
    
    QuadtreeTest qt;
    qt.useQuadTree = true;
    qt.Run();
    
    QuadtreeTest qt2;
    qt2.useQuadTree = false;
    qt2.Run();

    return 0;
}
