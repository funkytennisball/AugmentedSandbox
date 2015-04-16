#include "ModelWidget.h"

ModelWidget::ModelWidget(QWidget *parent, QGLWidget *shareWidget)
     : QGLWidget(parent, shareWidget)
 {
     clearColor = Qt::black;
     xRot = 0;
     yRot = 0;
     zRot = 0;
 #ifdef QT_OPENGL_ES_2
     program = 0;
 #endif
 }

 ModelWidget::~ModelWidget()
 {
 }

 void ModelWidget::initializeGL(){
	GLfloat verts[] =
	{
                //1st Triangle
		+0.0f, +1.0f, +0.0f, //Vertex 1
		-1.0f, -1.0f, -0.5f, //Vertex 2
		+1.0f, -1.0f, +0.5f, //Vertex 3
                //2nd Triangle
		+0.0f, -1.0f, +0.0f, //Vertex 1
		-1.0f, +1.0f, -0.5f, //Vertex 2
		+1.0f, +1.0f, +0.5f, //Vertex 3
	};
	GLuint myBufferID;
	glGenBuffers(1, &myBufferID); //Generate an identifier for the buffer ID
	glBindBuffer(GL_ARRAY_BUFFER, myBufferID); //Bind it to the array buffer binding point
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); //Send the size of the data and the data to the buffer assigned to the array buffer binding point.
 
	//Describing the data
	glEnableVertexAttribArray(0); //The first attribute to send to openGL (later redirected through a vertex shader)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0,0); //There are 3 floats per vertex...
}
 
void ModelWidget::paintGL(){
	glViewport(0, 0, width(), height()); //Fixes the resizing to change ratio of triangle
	glDrawArrays(GL_TRIANGLES, 0, 6); //Connect the 6 vertices into triangles (triangle=3, 6/3 = 2 triangles)
}