#include "PhysicsThread.h"

#include <Eigen/Dense>
using namespace Eigen;

#define ARR2D(u, x, y, w) ((*u)[(y) * w + (x)])
#define BOUND(a, lim) (a < lim) ? (a >0) ? a : 1 : lim -1



PhysicsThread::	PhysicsThread(TerrainPtr& tp, QMutex& terrainMutex, std::atomic<bool> &depthReady,
		WorldPtr& wp, QMutex& worldMutex, std::atomic<bool> &worldReady):
	g(-10),
	step(0.01),
	inc(new float(0.01)),
	invdx(1.0/cSampleSize),
	reset(false),
	e_DepthReady(depthReady),
	e_Terrain(tp),
	e_TerrainMutex_ptr(terrainMutex),
	e_WorldReady(depthReady),
	e_World(wp),
	e_WorldMutex_ptr(worldMutex),
	fillInWater(new std::array<float, cArrayLength>())
{
	
}


PhysicsThread::~PhysicsThread(void)
{
}

void PhysicsThread::setValue(int x, int y){
	if(x==-100&& y==-100){
		reset= 1;

		return;
	}

	if(x<-100 && y ==-100){
		if(x==-101){
			*inc = 0.01;
		} else if (x==-102){
			*inc = 0.1;
		} else if (x==-103){
			*inc = 0.5;
		} else if (x==-104){
			*inc = 1.0;
		} else if (x==-105){
			*inc = 2.0;
		}

		return;
	}

	int floodsize = 10;
	for(int j =0; j<floodsize; ++j)
		for(int i =0; i<floodsize; ++i){
			int halfflood = floodsize/2;
			if(x-halfflood+i > 0 && x - halfflood+i <cPointWidth-1 && y-halfflood+j > 0 && y - halfflood+j <cPointHeight-1 )
				ARR2D(fillInWater, x-halfflood+i, y-halfflood+j, cPointWidth) += *inc;
		}
		reset = 2;
}

float PhysicsThread::interpolate(Grid &grid, float x, float y){
	const int X = (int)x;
	const int Y = (int)y;
	const float s1 = x - X;
	const float s0 = 1.f - s1;
	const float t1 = y - Y;
	const float t0 = 1.f - t1;
	return  s0*(t0* ARR2D(&grid, X, Y, cPointWidth) + t1*ARR2D(&grid, X, Y+1, cPointWidth)) +
		s1*(t0*ARR2D(&grid, X+1, Y, cPointWidth) + t1*ARR2D(&grid, X+1, Y+1, cPointWidth));
}

float PhysicsThread::interpolate2(Grid &grid, float x, float y){
	int SIZET = cPointWidth;
	const int X = (int)x;
	const int Y = (int)y;
	const float s1 = x - X;
	const float s0 = 1.f - s1;
	const float t1 = y - Y;
	const float t0 = 1.f - t1;
	return  s0*(t0* grid[X + SIZET*Y] + t1*grid[X + SIZET*(Y + 1)]) +
		s1*(t0*grid[(X + 1) + SIZET*Y] + t1*grid[(X + 1) + SIZET*(Y + 1)]);

}



void PhysicsThread::advectArray2(Grid &grid, Grid &vel_x, Grid &vel_y, int velocityType) {
	Grid temp;
	int SIZET = cPointWidth;
	for (int i = 1; i<cPointWidth - 1; i++)
		for (int j = 1; j<cPointHeight - 1; j++) {
		const int index = i + j*cPointWidth;

		// distinguish different cases to interpolate the velocity
		float u = 0.0, v = 0.0;
		switch (velocityType) {
		case 0: // heights
			u += (vel_x[index] + vel_x[index + 1]) *0.5;
			v += (vel_y[index] + vel_y[index + SIZET]) *0.5;
			break;
		case 1: // x velocity
			u += vel_x[index];
			v += (vel_y[index] + vel_y[index + 1] + vel_y[index + SIZET] + vel_y[index + SIZET + 1]) *0.25;
			break;
		case 2: // y velocity
			u += (vel_x[index] + vel_x[index + 1] + vel_x[index + SIZET] + vel_x[index + SIZET + 1]) *0.25;
			v += vel_y[index];
			break;
		default: // invalid
			exit(1);
		}

		// backtrace position
		float srcpi = (float)i - u * step * invdx;
		float srcpj = (float)j - v * step * invdx;

		// clamp range of accesses
		if (srcpi<0.) srcpi = 0.;
		if (srcpj<0.) srcpj = 0.;
		if (srcpi>SIZET - 1.) srcpi = SIZET - 1.;
		if (srcpj>cPointHeight - 1.) srcpj = cPointHeight - 1.;

		// interpolate source value
		temp[index] = interpolate2(grid, srcpi, srcpj);
		}

	// copy back...
	for (int i = 1; i<cPointWidth - 1; i++)
		for (int j = 1; j<cPointHeight - 1; j++) {
		const int index = i + j*SIZET;
		grid[index] = temp[index];
		}
}


void PhysicsThread::advectArray(Grid &grid, Grid &horvert, Grid &ververt, int velocityType) {
			Grid temp;
	for (int i = 1; i<cPointWidth - 1; i++)
		for (int j = 1; j<cPointHeight - 1; j++) {

		float u = 0, v = 0;
		switch (velocityType) {
		case 0: // heights
			u += (ARR2D(&horvert, i, j, cPointWidth) + ARR2D(&horvert, i+1, j, cPointWidth)) *0.5;
			v += (ARR2D(&ververt, i, j, cPointWidth) + ARR2D(&ververt, i, j+1, cPointWidth)) *0.5;
			break;
		case 1: // x velocity
			u += ARR2D(&horvert, i, j, cPointWidth);
			v += (ARR2D(&ververt, i, j, cPointWidth) + ARR2D(&ververt, i+1, j, cPointWidth) 
				+ ARR2D(&ververt, i, j+1, cPointWidth) + ARR2D(&ververt, i+1, j+1, cPointWidth)) *0.25;
			break;
		case 2: // y velocity
			u += (ARR2D(&horvert, i, j, cPointWidth) + ARR2D(&horvert, i+1, j, cPointWidth) 
				+ ARR2D(&horvert, i, j+1, cPointWidth) + ARR2D(&horvert, i+1, j+1, cPointWidth)) *0.25;
			v +=  ARR2D(&ververt, i, j, cPointWidth);
			break;
		default: // invalid
			exit(1);
		}

		// backtrace position
		float srcpi = (float)i - u * step * invdx;
		float srcpj = (float)j - v * step * invdx;
			
		// clamp range of accesses
		if (srcpi<0.) srcpi = 0.;
		if (srcpj<0.) srcpj = 0.;
		if (srcpi>cPointWidth - 1.) srcpi = cPointWidth - 1.;
		if (srcpj>cPointHeight - 1.) srcpj = cPointHeight - 1.;

		// interpolate source value

		temp.at(i+j*cPointWidth) = interpolate(grid, srcpi, srcpj);
		}

	// copy back...
	for (int i = 1; i<cPointWidth - 1; i++)
		for (int j = 1; j<cPointHeight - 1; j++) {
		const int index = i + j*cPointWidth;
		grid[index] = temp[index];
		}
}

static int otl = 0;
void PhysicsThread::iteration(){

	// Try to acquire lock and copy
	while(!e_DepthReady);
	while(!e_WorldReady);

	// Terrain Data
	if(otl==0){
	e_TerrainMutex_ptr.lock();
	temp_terrain_ptr = std::make_shared<Terrain> (*e_Terrain);
	e_TerrainMutex_ptr.unlock();
	otl ++;
	}

	// Check if change in terrain
	float sumdiff = 0.0;
	if(prev_terrain_ptr != NULL){
		for (int i = 0; i < cPointWidth; i++){

			sumdiff += abs(temp_terrain_ptr->points->at(i*3+1)-prev_terrain_ptr->points->at(i*3+1));
		}	
	}


	
	// Water Data
	e_WorldMutex_ptr.lock();
	temp_world_ptr = std::make_shared<World> (*e_World);
	e_WorldMutex_ptr.unlock();

	
	if(sumdiff >  150){
		temp_terrain_ptr = prev_terrain_ptr;
		temp_world_ptr->vertVelocity.fill(0);
		temp_world_ptr->horVelocity.fill(0);
	} 
	auto it_start = temp_world_ptr->waterHeight.begin();
	auto it_end = temp_world_ptr->waterHeight.end();
	
	auto horVelocity = &(temp_world_ptr->horVelocity);
	auto vertVelocity = &(temp_world_ptr->vertVelocity);
	auto height = &(temp_world_ptr->waterHeight);
	auto ground = temp_terrain_ptr->points;

	// Starts filling Water
	if(reset == 2){
		std::transform(it_start, it_end, fillInWater->begin(), it_start, [](float original, float addition){ return original+addition;});
		reset = 0;
	}
	fillInWater->fill(0);	// clear array

	if(reset==1){
		height->fill(0);
		vertVelocity->fill(0);
		horVelocity->fill(0);
		reset = 0;
		otl = 1;
	}

	
	// SWE Algorithm
	advectArray2(*height, *horVelocity, *vertVelocity,0);
	advectArray2(*horVelocity, *horVelocity, *vertVelocity,1);
	advectArray2(*vertVelocity, *horVelocity, *vertVelocity,2);
	
	// Update Height
	for (int i = 1; i<cPointWidth - 1; i++)
		for (int j = 1; j<cPointHeight - 1; j++) {
		const int index = i + j*cPointWidth;
		float dh = -0.5 * ARR2D(height, i, j, cPointWidth) * invdx * (
			(ARR2D(horVelocity, i+1, j, cPointWidth) - ARR2D(horVelocity, i, j, cPointWidth)) +
			(ARR2D(vertVelocity, i, j+1, cPointWidth) - ARR2D(vertVelocity, i, j, cPointWidth)));

		ARR2D(height, i, j, cPointWidth) += dh * step;
		}


	Grid eph;
	for (int i = 0; i < cPointWidth; i++)
	for (int j = 0; j<cPointHeight; j++)
		 {
			ARR2D(&eph, i, j, cPointWidth) = ARR2D(height, i, j, cPointWidth) + ground->at((i+j*cPointWidth) * 3 + 1);
		}


	// Update Velocities
			
	for (int j = 2; j<cPointHeight-1; ++j){
		for (int i = 1; i<cPointWidth-1; ++i){
			ARR2D(horVelocity, i, j, cPointWidth) += g * step * invdx *
				(ARR2D(&eph,i,j, cPointWidth) - ARR2D(&eph,i-1,j, cPointWidth));
		}
	}

	for (int j = 1; j<cPointHeight-1; ++j){
		for (int i = 2; i<cPointWidth-1; ++i){
			ARR2D(vertVelocity, i, j, cPointWidth) += g * step * invdx * 
				(ARR2D(&eph,i,j, cPointWidth) - ARR2D(&eph,i,j-1, cPointWidth));
		}
	}

		for (int i = 0; i<cPointWidth; i++) {
			const int index1 = i + 0 * cPointWidth;
			const int index2 = i + (cPointHeight - 1)*cPointWidth;

			(*height)[index1] = (*height)[index1 + cPointWidth];
			(*height)[index2] = (*height)[index2 - cPointWidth];
		}
		for (int j = 0; j<cPointHeight; j++) {
			const int index1 = 0 + j*cPointWidth;
			const int index2 = (cPointWidth - 1) + j*cPointWidth;
			(*height)[index1] = (*height)[index1 + 1];
			(*height)[index2] = (*height)[index2 - 1];
		}

	prev_terrain_ptr = std::make_shared<Terrain>(*temp_terrain_ptr);

	// Copy back
	e_WorldMutex_ptr.lock();
	std::swap(e_World, temp_world_ptr);
	e_WorldMutex_ptr.unlock();

}

void PhysicsThread::exception(){

}