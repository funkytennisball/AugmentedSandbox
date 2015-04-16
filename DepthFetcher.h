#ifndef DEPTHFETCHER_H
#define DEPTHFETCHER_H

#undef NUI_CONFLICT

// C/C++
#include <memory>
#include <string>
#include <atomic>

// Lib .h
#include <qmutex.h>

#include "stdafx.h"
#include "NuiApi.h"
#undef min
#undef max

// Local .h
#include "stconsts.h"
#include "InfiniteThread.h"

#include "stdefine.h"

typedef std::array<Eigen::Vector3f, (cPointWidth-1) * (cPointHeight -1) *2> FaceNormal;
typedef std::shared_ptr<FaceNormal> FaceNormalPtr;

class DepthFetcherThread : public InfiniteThread
{
private:
	// Global vars
	TerrainPtr& e_Terrain;
	QMutex& e_PointsMutex_ptr;
	std::atomic<bool>& e_DepthReady;

	// Custom status message
	std::string m_StatusMessage;

	// Sensor 
	std::shared_ptr<INuiSensor> m_pNuiSensor;

	// event signaled when depth data is available
	HANDLE m_hNextDepthFrameEvent;
	HANDLE m_pDepthStreamHandle;

	// Data
	IndicesPtr indices;
 
public:
	DepthFetcherThread(TerrainPtr&,  QMutex&, std::atomic<bool> &);

private:
	HRESULT initKinect();
	void initData();

	void exception();
	void iteration();
 };

#endif