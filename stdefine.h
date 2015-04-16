#ifndef STDEFINE_H
#define STDEFINE_H
 
#include "qgl.h"

#include <array>
#include <memory>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <Eigen/Dense>

#include "World.h"
#include "stconsts.h"

#define NOMINMAX

typedef std::array<GLfloat, cArrayLength*3> Points;
typedef std::array<GLuint, cIndicesLength> Indices;
typedef std::array<GLfloat, cArrayLength*3> Normals;

typedef std::shared_ptr<Points> PointsPtr;
typedef std::shared_ptr<Indices> IndicesPtr;
typedef std::shared_ptr<Normals> NormalsPtr; 

typedef struct{
	PointsPtr points;
	IndicesPtr indices;
	NormalsPtr normals;

	static const int numPoints = cArrayLength;
	static const int numIndices = cIndicesLength;
} Terrain; 


typedef std::shared_ptr<Terrain> TerrainPtr;
typedef std::shared_ptr<World> WorldPtr;

#endif