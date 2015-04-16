#ifndef DEPTH_VIEWER_H
#define DEPTH_VIEWER_H

#include <memory>
#include <GL/glew.h>

#include <QtGui>
#include <qmutex.h>
#include <QGLViewer/qglviewer.h>

#include <glm/glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stdefine.h"

#include "CTexture.h"

typedef std::array<float, 2*cArrayLength> TextureCoord;
typedef std::shared_ptr<TextureCoord> TextureCoordPtr;

class ModelViewer : public QGLViewer
{
	Q_OBJECT

public:
	ModelViewer(TerrainPtr &, QMutex&, WorldPtr &, QMutex&,QString windowTitle);
	ModelViewer(TerrainPtr &p, QMutex& depthmutex, WorldPtr &, QMutex&, QString windowTitle, int type);

private slots:

signals:
    void setPoint(int, int);
protected :
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();
	virtual void initShaders();
	virtual void initBuffers();

	//void resizeGL(int w, int h);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

	void keyPressEvent(QKeyEvent *event);

private:
	int state;

	QDialog* popupControl;

	QGLViewer * sub;

	QMutex& e_TerrainMutex_ptr;
	TerrainPtr & e_Terrain;
	QMutex& e_WorldMutex_ptr;
	WorldPtr & e_World;

	GLuint vertexBufferID;
	GLuint indexBufferID;
	 QPoint lastPos;

	
	
	IndicesPtr indices;

	private slots:
		void setScaleXValue(int);
		void setScaleYValue(int);
		void setScaleZValue(int);
private:
    int xAtPress, yAtPress;
	

private:
float scalex, scaley, scalez;
QString title;
QMatrix4x4 pMatrix;

////////////////////////////////////////////////////////////////////////////////////////
// Textures
CTexture grass;
CTexture rocks;
CTexture dirt; 
CTexture water; 
CTexture lightwater; 
CTexture ocean; 

////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////
// Shaders
// Shaders ID
GLuint p, v, f;
// Shaders path
char * vertexFileName;
char * fragmentFileName;

void printProgramInfoLog(GLuint obj);
void printShaderInfoLog(GLuint obj);
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
// MVP
glm::mat4  MVP;
float sphi, stheta;
float sdepth;
float stransx, stransy, stransz;
float zNear, zFar;
float aspect;
////////////////////////////////////////////////////////////////////////////////////////

};

#endif