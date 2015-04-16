#include "DepthFetcher.h"

#include <iostream>
#include <chrono>
#include <thread>
#include <queue>
#include <QtDebug>
#include <pcl/filters/voxel_grid.h>

DepthFetcherThread::DepthFetcherThread(TerrainPtr& p, QMutex& mutex, std::atomic<bool> &depthReady) :
		e_DepthReady(depthReady),
		e_Terrain(p),
		e_PointsMutex_ptr(mutex)
{
	initKinect();
	initData();
}

void DepthFetcherThread::initData(){
	assert(cPointWidth > 1);
	assert(cPointHeight > 1);

	indices = std::make_shared<Indices>();

	int x=0;
	int y = 0;
	byte state = 0;	// 0 = down pending right, 1 = down pending left, 2 =  right up, 3 = left up, 4 = loop pending dpr, 5 = loop pending dpl

	for(int i=0; i<cIndicesLength; i++){
		(*indices)[i] = x + y * cPointWidth;
		switch(state){
		case 0:
			y = y + 1;
			if (x >= cPointWidth - 1){
				state = 5;
			} else {
				state = 2;
			}
		break;
		case 1:
			y = y + 1;
			if (x <= 0){
				state = 4;
			} else {
				state = 3;
			}
		break;
		case 2:
			y = y - 1;
			x = x + 1;
			state = 0;
		break;
		case 3:
			x = x - 1;
			y = y - 1;
			state = 1;
		break;
		case 4:
			state = 0;
		break;
		case 5:
			state = 1;
		break;
		}
	}
}

HRESULT DepthFetcherThread::initKinect(){
	INuiSensor * pNuiSensor;
	HRESULT hr;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
			m_pNuiSensor.reset(pNuiSensor);
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (NULL != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using depth
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH); 
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when depth data is available
            m_hNextDepthFrameEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

            // Open a depth image stream to receive depth frames
            hr = m_pNuiSensor->NuiImageStreamOpen(
                NUI_IMAGE_TYPE_DEPTH,
                NUI_IMAGE_RESOLUTION_640x480,
                0,
                2,
                m_hNextDepthFrameEvent,
                &m_pDepthStreamHandle);
        }
    }

    if (NULL == m_pNuiSensor || FAILED(hr))
    {
		m_StatusMessage = "No ready Kinect found!";
        return E_FAIL;
    }

	return hr;
}
void DepthFetcherThread::iteration(){
	HRESULT hr;
    NUI_IMAGE_FRAME imageFrame;

    // Attempt to get the depth frame
	
	// TO BE DONE
	// check if kinect is found
	while(m_pNuiSensor == NULL);

    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);

    if (FAILED(hr))
    {
        return;
    }

    BOOL nearMode;
    INuiFrameTexture* pTexture;

    // Get the depth image pixel texture
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(
        m_pDepthStreamHandle, &imageFrame, &nearMode, &pTexture);
    if (FAILED(hr))
    {
        m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
		return;
    }

    NUI_LOCKED_RECT LockedRect;

    // Lock the frame data so the Kinect knows not to modify it while we're reading it
    pTexture->LockRect(0, &LockedRect, NULL, 0);
	
	// Initialize temp buffer
	TerrainPtr	temp_terain_ptr(new Terrain);
	
	int it = 0;

    // Make sure we've received valid data
	temp_terain_ptr->indices = std::make_shared<Indices>(*indices); // copy indices
	temp_terain_ptr->points  = std::make_shared<Points>();
	temp_terain_ptr->normals  = std::make_shared<Normals>();

	PointsPtr pointsptr = temp_terain_ptr->points;
	NormalsPtr normalsptr = temp_terain_ptr->normals;

    if (LockedRect.Pitch != 0)
    {
        const NUI_DEPTH_IMAGE_PIXEL * pBufferRun = reinterpret_cast<const NUI_DEPTH_IMAGE_PIXEL *>(LockedRect.pBits);

		// Get the min and max reliable depth for the current frame
        int minDepth =  (400 << NUI_IMAGE_PLAYER_INDEX_SHIFT) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
        int maxDepth = ((700 << NUI_IMAGE_PLAYER_INDEX_SHIFT) | NUI_IMAGE_PLAYER_INDEX_MASK) >> NUI_IMAGE_PLAYER_INDEX_SHIFT;
        // end pixel is start + width*height - 1
		
        const NUI_DEPTH_IMAGE_PIXEL * pBufferEnd = pBufferRun + (cDepthWidth * cDepthHeight);

		int i=0;
		int j=0;
		
		int maxj = cDepthWidth - cDepthWidth % cSampleSize  - cSampleSize;
		int pbuffinc = cDepthWidth % cSampleSize + cSampleSize + cDepthWidth * (cSampleSize - 1);
		
		std::queue<Eigen::Vector2i> holes;

		int min = 9999;
		int max = -9999;

		while(pBufferRun < pBufferEnd){
			USHORT depth = pBufferRun->depth;

			int intensity = static_cast<int>(depth >= minDepth && depth <= maxDepth ? maxDepth - depth  : 0);
			
			min = (intensity < min && intensity != 0) ? intensity : min;
			max = (intensity > max) ? intensity : max;

			// check for deviation
			if(e_DepthReady){
				if(abs(intensity-(*e_Terrain->points)[it+1]) < 10.0f){
					(*pointsptr)[it] = j;
					(*pointsptr)[it+1] = (*e_Terrain->points)[it+1];
					(*pointsptr)[it+2] = i;

					goto increment;
				}
			}

			if(intensity > 15){
				(*pointsptr)[it] = j;
				(*pointsptr)[it+1] = intensity;
				(*pointsptr)[it+2] = i;
			} else {
				(*pointsptr)[it] = j;
				(*pointsptr)[it+1] = cInvalidDepth;
				(*pointsptr)[it+2] = i;
				holes.push(Eigen::Vector2i(j,i));
			}
			
			increment:
			it = it + 3;

			j = (j >= maxj) ? 0 : (j + cSampleSize);
			i = j == 0 ? i+cSampleSize : i;

			pBufferRun = j==0 ? pBufferRun + pbuffinc :  pBufferRun + cSampleSize ;
		}

		// hole filling
		int itr = 0;
		int lim = 4;

		while(!holes.empty()){
			Eigen::Vector2i p = holes.front();
			holes.pop();
			std::vector<int> average;

			int c=0;
			int p0 = p[0]/cSampleSize;
			int p1 = p[1]/cSampleSize;

			GLfloat curx = (*pointsptr)[(p0+(p1)*cPointWidth) * 3];
			GLfloat cury = (*pointsptr)[(p0+(p1)*cPointWidth) * 3 + 1];
			GLfloat curz = (*pointsptr)[(p0+(p1)*cPointWidth) * 3 + 2];

			int y;
			if(curx - 1 >= 0 && curz - 1 >= 0){
				y = (*pointsptr)[(p0-1+(p1-1)*cPointWidth) * 3 + 1];
	
				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			if(curx - 1 >= 0){
				y = (*pointsptr)[(p0-1+(p1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			if(curx - 1 >= 0 && curz + 1 < cPointHeight ){
				y = (*pointsptr)[(p0-1+(p1+1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			
			if(curz + 1 < cPointHeight){
				y = (*pointsptr)[(p0+(p1+1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}

			if(curz - 1 >= 0){
				y = (*pointsptr)[(p0+(p1-1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			if(curx + 1 < cPointWidth){
				y = (*pointsptr)[(p0+1+(p1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			
			if(curx + 1 < cPointWidth && curz - 1 >= 0){
				y = (*pointsptr)[(p0+1+(p1-1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}
			if(curx + 1< cPointWidth && curz + 1 < cPointHeight){
				y = (*pointsptr)[(p0+1+(p1+1)*cPointWidth) * 3 + 1];

				if(y != cInvalidDepth){
					average.push_back(y);
					++c;
				}
			}

			if(c < lim){
				holes.push(p);
			} else {
				if(c == 0){ 
					(*pointsptr)[(p0+(p1)*cPointWidth) * 3 + 1] = (min+max)/2; 
				} else {
					int sum = 0;
					for(int a : average){
						sum = a + sum;
					}
					(*pointsptr)[(p0+(p1)*cPointWidth) * 3 + 1] = sum / average.size();
				}
			}

			itr++;

			if(itr>4000 && itr <6000){
				lim = 3;
			} else if (itr >=6000 && itr < 8000){
				lim = 2;
			} else if (itr >=8000 && itr < 1252000){
				lim = 1;
			} else {
				lim = 0;
			}
		}
    }

	// Recheck points
	bool skipwork = false;
	if(e_DepthReady){
		float sumdiff = 0.0;

		for (int i =0; i< cArrayLength; ++i){
			float change = abs((*pointsptr)[i*3+1]-(*e_Terrain->points)[i*3+1]) ;
			if(change < 12.0f && change > 80.0f){
				(*pointsptr)[i*3+1] = (*e_Terrain->points)[i*3+1];
				change = 0;
			}
			sumdiff += change;
		}
		if(sumdiff  <2000.0){
			skipwork = true;
			*pointsptr = (*e_Terrain->points);
			//for (int i =0; i< cArrayLength; ++i){

			//	(*pointsptr)[i*3+1] = (*e_Terrain->points)[i*3+1];
			//	change = 0;
			//}
			//sumdiff += change;
		
		} else {
			//std::cout << sumdiff << std::endl;
		}
	}
    // We're done with the texture so unlock it
    pTexture->UnlockRect(0);

    pTexture->Release();

	// Start building normals

	// precalculate face normals
	if(!skipwork){
	FaceNormalPtr faceNormalsPtr = std::make_shared<FaceNormal>();
	auto normalIt = faceNormalsPtr->begin();
	for(int y=0; y<cPointHeight-1; ++y){
		for(int x=0; x<cPointWidth-1; ++x){
			Eigen::Vector3f a(cSampleSize, (*pointsptr)[(x+1 + y*cPointWidth) * 3 + 1] - (*pointsptr)[(x + y*cPointWidth) * 3 + 1], 0);
			Eigen::Vector3f b(0, (*pointsptr)[(x + (y+1)*cPointWidth) * 3 + 1] - (*pointsptr)[(x + y*cPointWidth) * 3 + 1], cSampleSize);
			Eigen::Vector3f c(-cSampleSize, (*pointsptr)[(x + (y+1)*cPointWidth) * 3 + 1] - (*pointsptr)[(x+1 + (y+1)*cPointWidth) * 3 + 1], 0);
			Eigen::Vector3f d(0, (*pointsptr)[(x+1 + (y)*cPointWidth) * 3  + 1] - (*pointsptr)[(x+1 + (y+1)*cPointWidth) * 3], -cSampleSize);

			Eigen::Vector3f u = b.cross(a);
			Eigen::Vector3f v = d.cross(c);

			u.normalize();
			v.normalize();

			*normalIt = u;
			*(normalIt + 1) = v;

			normalIt = normalIt + 2;
		}
	}
	for(int y=0; y<cPointHeight; ++y){
		for(int x=0; x<cPointWidth; ++x){
			Eigen::Vector3f normal(0, 0, 0);
			
			if(x>0 && y>0) normal += (*faceNormalsPtr)[((x-1) + (y-1)*(cPointWidth-1)) *2 + 1];
			if(x<cPointWidth-1 && y>0) normal += (*faceNormalsPtr)[(x + (y-1)*(cPointWidth-1)) *2] + (*faceNormalsPtr)[(x + (y-1)*(cPointWidth-1)) *2 + 1];
			if(x<cPointWidth-1 && y<cPointHeight-1) normal += (*faceNormalsPtr)[(x + y*(cPointWidth-1)) *2];
			if(x>0 && y<cPointHeight-1) normal += (*faceNormalsPtr)[(x-1 + y*(cPointWidth-1)) *2] + (*faceNormalsPtr)[(x-1 + y*(cPointWidth-1)) *2 + 1];
			
			normal.normalize();
			(*normalsptr)[(x+y*cPointWidth) * 3] = normal.x();
			(*normalsptr)[(x+y*cPointWidth) * 3+1] = normal.y();
			(*normalsptr)[(x+y*cPointWidth) * 3+2] = normal.z();
		}
	}
	}
	
	// Acquire lock and swap
	e_PointsMutex_ptr.lock();
	std::swap(e_Terrain, temp_terain_ptr);

	if (!e_DepthReady) e_DepthReady = true;
	
	e_PointsMutex_ptr.unlock();
		
	m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);
}

void DepthFetcherThread::exception(){
	std::cout << "omg" << std::endl;
}