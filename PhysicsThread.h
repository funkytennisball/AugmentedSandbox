#pragma once

#define SIDELENGTH = 4

// C/C++
#include <atomic>

// Lib .h
#include "qatomic.h"
#include <qmutex.h>

// local.h
#include "InfiniteThread.h"
#include "stdefine.h"

class PhysicsThread  : public InfiniteThread
{
	Q_OBJECT

private:
	// Global vars
	TerrainPtr& e_Terrain;
	QMutex& e_TerrainMutex_ptr;
	std::atomic<bool>& e_DepthReady;

	WorldPtr& e_World;
	QMutex& e_WorldMutex_ptr;
	std::atomic<bool>& e_WorldReady;

	const float g;
	const float step;
	const float invdx;
int oldi;
	int oldj;
	QAtomicPointer<std::array<float, cArrayLength>> fillInWater;
	QAtomicInt reset;
	QAtomicPointer<float> inc;

	// local
	TerrainPtr temp_terrain_ptr;
	WorldPtr temp_world_ptr;
	TerrainPtr prev_terrain_ptr;
private slots:
	void setValue(int, int);

public:
	PhysicsThread(TerrainPtr& tp, QMutex& terrainMutex, std::atomic<bool> &depthReady,
		WorldPtr& wp, QMutex& worldMutex, std::atomic<bool> &worldReady);
	~PhysicsThread(void);

	float PhysicsThread::interpolate(Grid &grid, float x, float y);
	float PhysicsThread::interpolate2(Grid &grid, float x, float y);
	void PhysicsThread::advectArray(Grid &grid, Grid &horvert, Grid &ververt, int velocityType);
	void PhysicsThread::advectArray2(Grid &grid, Grid &horvert, Grid &ververt, int velocityType);

	void exception();
	void iteration();
};

