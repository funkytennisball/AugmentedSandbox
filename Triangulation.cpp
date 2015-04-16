#include "Triangulation.h"

static int suddenly = 0;

TriangulationThread::TriangulationThread(TerrainPtr& p, std::shared_ptr<Triangulation>& t,
										 QMutex& depthmutex, QMutex& triangulationmutex,
										 std::atomic<bool> &depthReady, std::atomic<bool> &triangulationReady) :
		e_DepthReady(depthReady),
		e_TriangulationReady(triangulationReady),
		e_Triangulation(t),
		e_Terrain(p),
		e_PointsMutex_ptr(depthmutex),
		e_TriangulationMutex_ptr(triangulationmutex)
{
	
}

void TriangulationThread::iteration(){
	// hold until value is updated
	while(!e_DepthReady);

	// Create temp buffer

	TerrainPtr	temp_terrain_ptr;
	TriangulationPtr	temp_triangulation_ptr (new Triangulation);
	
	// Try to acquire lock and copy
	e_PointsMutex_ptr.lock();
	temp_terrain_ptr = std::make_shared<Terrain>(*e_Terrain);
	e_PointsMutex_ptr.unlock();

	auto it = temp_terrain_ptr->cloud.begin();
	auto it_end = temp_terrain_ptr->cloud.end();
	
	auto it_triangle = temp_triangulation_ptr->begin();
	while(it != it_end){
		if(it->x() < cDepthWidth - cSampleSize && it->z() < cDepthHeight - cSampleSize){
			(*it_triangle)[0] = *it;
			(*it_triangle)[1] = *(it+1);
			(*it_triangle)[2] = *(it+cPointWidth);
		}
		++it_triangle;
		
		if(it->x() > 0 && it->z() > 0){
			(*it_triangle)[0] = *it;
			(*it_triangle)[1] = *(it-1);
			(*it_triangle)[2] = *(it-cPointWidth);
		}
		++it;
		++it_triangle;
	}
	
	// Try to acquite lock and swap
	e_TriangulationMutex_ptr.lock();
	std::swap(e_Triangulation, temp_triangulation_ptr);

	if (!e_TriangulationReady){
		e_TriangulationReady = true;
	}
	e_TriangulationMutex_ptr.unlock();

}


void TriangulationThread::exception(){
}