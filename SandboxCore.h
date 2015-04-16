#ifndef SANDBOXCORE_H
#define SANDBOXCORE_H

#include <memory>
#include <qmutex.h>
#include <qwaitcondition.h>

#include "ModelViewer.h"
#include "Triangulation3DGraphicsItem.h"
#include "DepthFetcher.h"
#include "PhysicsThread.h"

class SandboxCore{
public:
	SandboxCore();
	~SandboxCore();

	void Execute();

public:
	void CreateModelViewer();
	void CreateTopDownViewer();

private:
	// UI
	ModelViewer * modelViewer;
	ModelViewer * subModelViewer;

	// Global vars
	TerrainPtr	m_Terrain;
	WorldPtr m_World;


	// Mutex
	QMutex m_DepthMutex;
	QMutex m_WorldMutex;
	QMutex m_Grid;

	// Status indicators
	std::atomic<bool> m_DepthReady;
	std::atomic<bool> m_WorldReady;

	// Threads
	DepthFetcherThread m_DepthFetcher;
	PhysicsThread m_Physics;
};

#endif