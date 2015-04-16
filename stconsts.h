#ifndef STCONSTS_H
#define STCONSTS_H

// global
static const int        cDepthWidth  = 640;
static const int        cDepthHeight = 480;

static const int		cSampleSize  = 4;
static const int		timeSpan = 0.2;

static const int		cPointWidth = cDepthWidth/cSampleSize;	
static const int		cPointHeight = cDepthHeight/cSampleSize;
static const int		cArrayLength = cPointWidth * cPointHeight;
static const int		cIndicesLength = cPointWidth * cPointHeight + cPointWidth * (cPointHeight - 2);
static const int		cTriangleCount =  (cDepthHeight-1) * (cDepthWidth-1) * 2 ;

// flags
static const int		cInvalidDepth = -999;
#endif