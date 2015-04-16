#include <qgraphicsscene.h>
#include <qrect.h>
#include <qgraphicsview.h>

#include "SandboxCore.h"

SandboxCore::SandboxCore():
	m_Terrain (),
	m_DepthReady(false),
	m_DepthFetcher(m_Terrain, m_DepthMutex, m_DepthReady),
	m_WorldReady(false),
	m_Physics(m_Terrain, m_DepthMutex, m_DepthReady, m_World, m_WorldMutex, m_WorldReady)
{
	m_World = std::make_shared<World>();
}

SandboxCore::~SandboxCore(){
}

void SandboxCore::Execute(){
	// start fetching data
	m_DepthFetcher.start();
	m_Physics.start();

	// open topdown view
	CreateTopDownViewer();
}

void SandboxCore::CreateModelViewer(){
	while(!m_DepthReady && !m_WorldReady);

	modelViewer = new ModelViewer(m_Terrain, m_DepthMutex, m_World, m_WorldMutex, "Terrain View");	
	modelViewer->show();
}

void SandboxCore::CreateTopDownViewer(){
	while(!m_DepthReady && !m_WorldReady);
	subModelViewer = new ModelViewer(m_Terrain, m_DepthMutex, m_World, m_WorldMutex, "Top Down View", 1);
	subModelViewer->show();

	while(!m_Physics.isRunning());

	QObject::connect(subModelViewer, SIGNAL(setPoint(int, int)),
                   &m_Physics, SLOT(setValue(int, int)));
}

