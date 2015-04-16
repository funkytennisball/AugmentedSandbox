#ifndef TRIANGULATION_H
#define TRIANGULATION_H

#include "stconsts.h"
#include "InfiniteThread.h"
#include <qmutex.h>

#include <memory>
#include <atomic>

#include "stdefine.h"


class TriangulationThread : public InfiniteThread
{
private:
	// Global vars
	TerrainPtr& e_Terrain;
	std::shared_ptr<Triangulation>& e_Triangulation;

	QMutex& e_PointsMutex_ptr;
	QMutex& e_TriangulationMutex_ptr;

	std::atomic<bool>& e_DepthReady;
	std::atomic<bool>& e_TriangulationReady;
 
public:
	TriangulationThread(TerrainPtr&,  std::shared_ptr<Triangulation>&,
		  QMutex& /*pointsmutex*/, QMutex& /*triangulationmutex*/, 
		std::atomic<bool>& /*depthready*/, std::atomic<bool>& /*triangulationready*/);

private:
	void exception();
	void iteration();
 };

#endif