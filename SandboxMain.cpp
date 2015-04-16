
#include <qapplication.h>
#include <qgraphicsscene.h>
#include <qrect.h>
#include <qgraphicsview.h>

#include <CGAL/point_generators_2.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Regular_triangulation_euclidean_traits_2.h>
#include <CGAL/Regular_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>

#include <CGAL/Qt/Converter.h>
#include <CGAL/Qt/GraphicsViewNavigation.h>
#include <CGAL/Qt/RegularTriangulationGraphicsItem.h>
#include <CGAL/Qt/VoronoiGraphicsItem.h>

#include "SandboxCore.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Iso_rectangle_2 Iso_rectangle_2;

typedef double Weight;
typedef CGAL::Regular_triangulation_euclidean_traits_2<K,Weight> Gt; 
typedef CGAL::Regular_triangulation_2<Gt> Regular;
typedef CGAL::Delaunay_triangulation_2<Gt> Delaunay;

int main(int argc, char ** argv)
{
    QApplication app(argc, argv);
	
	SandboxCore sandboxCore;
	sandboxCore.Execute();
	sandboxCore.CreateModelViewer();

    return app.exec();
}