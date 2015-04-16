#include "TriangulationViewer.h"

using namespace std;

TriangulationViewer::TriangulationViewer(TriangulationPtr &t, QMutex& triangulationmutex):
	e_Triangulation_ptr(t),
	e_TriangulationMutex_ptr(triangulationmutex),
	QGLViewer(){
}

// Draws a spiral
void TriangulationViewer::draw()
{
	// try to copy 
	TriangulationPtr	temp_triangulation_ptr;

	// Try to acquire lock and copy
	e_TriangulationMutex_ptr.lock();
	temp_triangulation_ptr = std::make_shared<Triangulation> (*e_Triangulation_ptr);
	e_TriangulationMutex_ptr.unlock();

	drawAxis();
	int a = 0;
	int i,j;
	
	glBegin(GL_TRIANGLES);
	for(auto it =  temp_triangulation_ptr->begin(); it != temp_triangulation_ptr->end(); ++it){
		float newr, newg, newb;
		float max = 130.0;
		float minm = 60.0;
		float cutoff = 90.0;
		float rangeone =  cutoff - minm;
		float rangetwo = max - cutoff;

		float high [3] = {0.8980, 0.1764, 0.1764};
		float mid  [3] = {0.8196, 0.5960, 0.1529};
		float low  [3] = {0.1882, 0.8666, 0.1294};
		
		
		newr = it->at(0).y() > cutoff ? 
			((high[0]-mid[0]) * (it->at(0).y() - cutoff) / rangetwo + mid[0]) :
			it->at(0).y() > minm ?
				(- (mid[0]-low[0]) * (cutoff - it->at(0).y()) / rangeone + mid[0]) :
				low[0];
		newg = it->at(0).y() > cutoff ? 
			((high[1]-mid[1]) * (it->at(0).y() - cutoff) / rangetwo + mid[1]) :
			it->at(0).y() > minm ?
				(- (mid[1]-low[1]) * (cutoff - it->at(0).y()) / rangeone + mid[1]) :
				low[1];
		newb = it->at(0).y() > cutoff ? 
			((high[2]-mid[2]) * (it->at(0).y() - cutoff) / rangetwo + mid[2]) :
			it->at(0).y() > minm ?
				(- (mid[2]-low[2]) * (cutoff - it->at(0).y()) / rangeone + mid[2]):
				low[2];

		glColor3f(newr,newg, newb);
		glVertex3f(it->at(0).x()/1000.0, it->at(0).y()/500.0, it->at(0).z()/1000.0);
		glVertex3f(it->at(1).x()/1000.0, it->at(1).y()/500.0, it->at(1).z()/1000.0);
		glVertex3f(it->at(2).x()/1000.0, it->at(2).y()/500.0, it->at(2).z()/1000.0);
	}
    glEnd();

}

void TriangulationViewer::init()
{
  // Restore previous viewer state.
  //restoreStateFromFile();
	// Light setup
	glDisable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	// Light default parameters
	const GLfloat light_ambient[4]  = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_specular[4] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_diffuse[4]  = {1.0, 1.0, 1.0, 1.0};

	glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, 3.0);
	glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,   10.0);
	glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.1f);
	glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.3f);
	glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);

	startAnimation();
	
}