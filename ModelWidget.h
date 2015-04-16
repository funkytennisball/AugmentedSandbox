#pragma once

#include <GL/glew.h>

#include <QtGui>
#include <QGLWidget>

class ModelWidget : public QGLWidget
{
public:
	ModelWidget(QWidget *parent = 0, QGLWidget *shareWidget = 0);;
	~ModelWidget();
protected:
    void initializeGL();
	void paintGL();

private:
    void makeObject();

    QColor clearColor;
    QPoint lastPos;
    int xRot;
    int yRot;
    int zRot;
    GLuint textures[6];
    QVector<QVector3D> vertices;
    QVector<QVector2D> texCoords;
};

