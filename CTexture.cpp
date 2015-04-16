#include "CTexture.h"
#include "FreeImage.h"
#include "GL\glew.h"


CTexture::CTexture(void):
	bMipMapsGenerated(false)
{
}

bool CTexture::loadTexture2D(std::string a_sPath, bool bGenerateMipMaps) 
{ 
   FREE_IMAGE_FORMAT fif = FIF_UNKNOWN; 
   FIBITMAP* dib(0); 

   fif = FreeImage_GetFileType(a_sPath.c_str(), 0); // Check the file signature and deduce its format

   if(fif == FIF_UNKNOWN) // If still unknown, try to guess the file format from the file extension
      fif = FreeImage_GetFIFFromFilename(a_sPath.c_str()); 

   if(fif == FIF_UNKNOWN) // If still unkown, return failure
      return false; 

   if(FreeImage_FIFSupportsReading(fif)) // Check if the plugin has reading capabilities and load the file
      dib = FreeImage_Load(fif, a_sPath.c_str()); 
   if(!dib) 
      return false; 

   BYTE* bDataPointer = FreeImage_GetBits(dib); // Retrieve the image data

   iWidth = FreeImage_GetWidth(dib); // Get the image width and height
   iHeight = FreeImage_GetHeight(dib); 
   iBPP = FreeImage_GetBPP(dib); 

   // If somehow one of these failed (they shouldn't), return failure
   if(bDataPointer == NULL || iWidth == 0 || iHeight == 0) 
      return false; 

   // Generate an OpenGL texture ID for this texture
   glGenTextures(1, &uiTexture); 
   glBindTexture(GL_TEXTURE_2D, uiTexture); 

   int iFormat = iBPP == 24 ? GL_BGR : iBPP == 8 ? GL_LUMINANCE : 0; 
   int iInternalFormat = iBPP == 24 ? GL_RGB : GL_DEPTH_COMPONENT;  

   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iWidth, iHeight, 0, iFormat, GL_UNSIGNED_BYTE, bDataPointer); 

   if(bGenerateMipMaps)glGenerateMipmap(GL_TEXTURE_2D); 

   FreeImage_Unload(dib); 

   glGenSamplers(1, &uiSampler); 

   sPath = a_sPath; 
   bMipMapsGenerated = bGenerateMipMaps; 

   return true; // Success
}

void CTexture::setFiltering(int a_tfMagnification, int a_tfMinification)
{
	// Set magnification filter
	if(a_tfMagnification == TEXTURE_FILTER_MAG_NEAREST)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	else if(a_tfMagnification == TEXTURE_FILTER_MAG_BILINEAR)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set minification filter
	if(a_tfMinification == TEXTURE_FILTER_MIN_NEAREST)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	else if(a_tfMinification == TEXTURE_FILTER_MIN_BILINEAR)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	else if(a_tfMinification == TEXTURE_FILTER_MIN_NEAREST_MIPMAP)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	else if(a_tfMinification == TEXTURE_FILTER_MIN_BILINEAR_MIPMAP)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	else if(a_tfMinification == TEXTURE_FILTER_MIN_TRILINEAR)
		glSamplerParameteri(uiSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	tfMinification = a_tfMinification;
	tfMagnification = a_tfMagnification;
}


void CTexture::bindTexture(int iTextureUnit)
{
	glActiveTexture(GL_TEXTURE0+iTextureUnit);
	glBindTexture(GL_TEXTURE_2D, uiTexture);
	glBindSampler(iTextureUnit, uiSampler);
}

void CTexture::releaseTexture()
{
	glDeleteSamplers(1, &uiSampler);
	glDeleteTextures(1, &uiTexture);
}

int CTexture::getMinificationFilter()
{
	return tfMinification;
}

int CTexture::getMagnificationFilter()
{
	return tfMagnification;
}