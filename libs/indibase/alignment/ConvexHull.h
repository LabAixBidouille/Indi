#ifndef CONVEXHULL_H
#define CONVEXHULL_H

// This C++ code is based on the c code described below
// it was ported to c++ by Roger James in December 2013
// !!!!!!!!!!!!!!!!!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!
// This must code must use integer coordinates. A naive conversion
// to floating point WILL NOT work. For the reasons behind this
// have a look at at section 4.3.5 of the O'Rourke book. For more
// information try http://www.mpi-inf.mpg.de/departments/d1/ClassroomExamples/
// For INDI alignment purposes we need to scale floating point coordinates
// into the integer space before using this algorithm.

/*
This code is described in "Computational Geometry in C" (Second Edition),
Chapter 4.  It is not written to be comprehensible without the
explanation in that book.

Input: 3n integer coordinates for the points.
Output: the 3D convex hull, in postscript with embedded comments
        showing the vertices and faces.

Compile: gcc -o chull chull.c (or simply: make)

Written by Joseph O'Rourke, with contributions by
  Kristy Anderson, John Kutcher, Catherine Schevon, Susan Weller.
Last modified: May 2000
Questions to orourke@cs.smith.edu.

--------------------------------------------------------------------
This code is Copyright 2000 by Joseph O'Rourke.  It may be freely
redistributed in its entirety provided that this copyright notice is
not removed.
--------------------------------------------------------------------
*/

#include <cstring> // I like to use NULL

class ConvexHull
{
    public:
        ConvexHull() : vertices(NULL), edges(NULL), faces(NULL), debug(false), check(false) {}
        virtual ~ConvexHull() {}

    enum { X = 0, Y = 1, Z = 2 };

    template <class Type>
        static void add(Type& head, Type p)
        {
            if (NULL != head)
            {
                p->next = head;
                p->prev = head->prev;
                head->prev = p;
                p->prev->next = p;
            }
            else
            {
                head = p;
                head->next = head->prev = p;
            }
        };

    template <class Type>
        static void remove(Type& head, Type p)
        {
             if (NULL != head)
             {
				if ( head == head->next )
					head = NULL;
				else if ( p == head )
					head = head->next;
				p->next->prev = p->prev;
				p->prev->next = p->next;
				delete p;
            }
        };

    template <class Type>
        static void swap(Type& t, Type& x, Type& y)
        {
            t = x;
            x = y;
            y = t;
        };

    // Explicit forward declarations
    struct tVertexStructure;
    struct tFaceStructure;
    struct tEdgeStructure;

    /* Define structures for vertices, edges and faces */
    typedef struct tVertexStructure tsVertex;
    typedef tsVertex *tVertex;

    typedef struct tEdgeStructure tsEdge;
    typedef tsEdge *tEdge;

    typedef struct tFaceStructure tsFace;
    typedef tsFace *tFace;

    struct tVertexStructure {
       int      v[3];
       int	    vnum;
       tEdge    duplicate;  // pointer to incident cone edge (or NULL)
       bool     onhull;     // True iff point on hull.
       bool	    mark;       // True iff point already processed.
       tVertex  next, prev;
    };

    struct tEdgeStructure {
       tFace    adjface[2];
       tVertex  endpts[2];
       tFace    newface;    // pointer to incident cone face.
       bool     delete_it;  //  True iff edge should be delete.
       tEdge    next, prev;
    };

    struct tFaceStructure {
       tEdge    edge[3];
       tVertex  vertex[3];
       bool	    visible;    // True iff face visible from new point.
       tFace    next, prev;
    };

    /* Define flags */
    static const bool  ONHULL = true;
    static const bool REMOVED = true;
    static const bool VISIBLE = true;
    static const bool PROCESSED = true;
    static const int SAFE = 1000000;		/* Range of safe coord values. */

    tVertex vertices;
    tEdge edges;
    tFace faces;
    bool debug;
    bool check;

    void PrintObj( void );
    void SubVec( int a[3], int b[3], int c[3]);
    void Print( void );
    void EdgeOrderOnFaces ( void );
    void CheckEndpts ( void );
    void CheckEuler(int V, int E, int F );
    void Convexity( void );
    void Consistency( void );
    void Checks( void );
    void CleanUp( tVertex *pvnext );
    void CleanEdges( void );
    void CleanFaces( void );
    void CleanVertices( tVertex *pvnext );
    void MakeCcw( tFace f, tEdge e, tVertex p );
    tFace MakeConeFace( tEdge e, tVertex p );
    bool AddOne( tVertex p );
    void ConstructHull( void );
    void PrintOut( tVertex v );
    void PrintVertices( void );
    void PrintEdges( void );
    void PrintFaces( void );
    int VolumeSign(tFace f, tVertex p);
    int Volumei( tFace f, tVertex p );
    tFace MakeNullFace( void );
    tEdge MakeNullEdge( void );
    tFace MakeFace( tVertex v0, tVertex v1, tVertex v2, tFace f );
    bool Collinear( tVertex a, tVertex b, tVertex c );
    void DoubleTriangle( void );
    void PrintPoint( tVertex p );
    tVertex	MakeNullVertex( void );
    void ReadVertices( void );

    protected:
    private:
};

#endif // CONVEXHULL_H
