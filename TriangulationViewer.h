#ifndef TRIANGULATION_VIEWER_H
#define TRIANGULATION_VIEWER_H

#include <memory>

#include <qmutex.h>
#include <QGLViewer/qglviewer.h>

#include "stdefine.h"

class TriangulationViewer : public QGLViewer
{
public:
	TriangulationViewer(TriangulationPtr&, QMutex& );

protected :
	virtual void draw();
	virtual void init();

private:
	QMutex& e_TriangulationMutex_ptr;
	TriangulationPtr & e_Triangulation_ptr;
};

#endif