#include "ModelViewer.h"

#include <fstream>

#include "Common.h"

using namespace std;

ModelViewer::ModelViewer(TerrainPtr &p, QMutex& depthmutex, WorldPtr &w, QMutex& worldmutex, QString windowTitle):
	title(windowTitle),
	e_Terrain(p),
	e_TerrainMutex_ptr(depthmutex),
	e_World(w),
	e_WorldMutex_ptr(worldmutex),
	QGLViewer(),
	vertexFileName("shade.vert"),
	fragmentFileName("shade.frag"),
	sphi(0.32), stheta(0.69),
	sdepth(0.0017),
	stransx(-0.57), stransy(-0.84), stransz(0.0),
	zNear(-5.0001), zFar(1000.0),
	scalex(1.0), scaley(1.0), scalez(1.0),
	aspect(5.0/4.0),
	state(0){
}

ModelViewer::ModelViewer(TerrainPtr &p, QMutex& depthmutex, WorldPtr &w, QMutex& worldmutex,  QString windowTitle, int type):
	title(windowTitle),
	e_Terrain(p),
	e_TerrainMutex_ptr(depthmutex),
	e_World(w),
	e_WorldMutex_ptr(worldmutex),
	vertexFileName("shade.vert"),
	fragmentFileName("shade.frag"),
	sphi(0), stheta(1.57),
	sdepth(0.0032),
	stransx(-1), stransy(-1), stransz(0.0),
	zNear(0.1), zFar(1000.0),
	scalex(1.0), scaley(1.0), scalez(1.31),
	aspect(5.0/4.0),
	state(type){
	
}

void ModelViewer::setScaleXValue(int i){
	scalex = ((float) i)/10;
}

void ModelViewer::setScaleYValue(int i){
	scaley = ((float) i)/10;
}

void ModelViewer::setScaleZValue(int i){
	scalez = ((float) i)/10;
}

void ModelViewer::initializeGL(){
	glewInit();
	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;
	cout << "OpenGL version " << glGetString(GL_VERSION) << " supported" << endl;
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	
	this->setWindowTitle(title);

	initBuffers();
	initShaders();
	startAnimation();
}

// Draws a spiral
void ModelViewer::paintGL()
{
	TerrainPtr temp_terrain_ptr;
	WorldPtr temp_world_ptr;

	//cout << sphi << "," << stheta << "," << sdepth << "," << stransx << "," << stransy << "," << stransz << "," << zNear << "," << zFar << "," << aspect << endl;
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear Screen And Depth Buffer

	// Try to acquire lock and copy
	e_TerrainMutex_ptr.lock();
	temp_terrain_ptr = std::make_shared<Terrain> (*e_Terrain);
	e_TerrainMutex_ptr.unlock();

	// Water Data
	e_WorldMutex_ptr.lock();
	temp_world_ptr = std::make_shared<World> (*e_World);
	e_WorldMutex_ptr.unlock();

	// Link shader
	std::array<GLfloat, cArrayLength*3> TempWater;
	for(int i = 0; i< cArrayLength; ++i){
		TempWater.at(i*3) = (*temp_terrain_ptr->points).at(i*3);
		
		TempWater.at(i*3+1) = (temp_world_ptr->waterHeight).at(i);

		TempWater.at(i*3+2) = (*temp_terrain_ptr->points).at(i*3+2);
	}

	//for(int i = 0; i<cArrayLength; ++i){		
	//		temp_terrain_ptr->points->at(i*3 + 1) = 250 + temp_world_ptr->waterHeight.at(i);
	//	}

	//// Sub In Data
	glBufferSubData(GL_ARRAY_BUFFER, 0, cArrayLength * 3 * sizeof(GLfloat), (*temp_terrain_ptr->points).data());
	glBufferSubData(GL_ARRAY_BUFFER, cArrayLength * 3 * sizeof(GLfloat), cArrayLength * 3 * sizeof(GLfloat), (TempWater).data());

	// Scale data
	glm::mat4 Scale = glm::scale(glm::mat4(1.0f), glm::vec3(scalex, scaley, scalez));
	// MVP
	glm::mat4 Projection = glm::perspective(45.0f, aspect, -0.1f, 1000.0f);
	glm::mat4 ViewTranslate = glm::translate(glm::mat4(1.0f), glm::vec3(stransx, stransy, stransz));
	glm::mat4 ViewRotateX = glm::rotate(ViewTranslate, -stheta, glm::vec3(1.0f, 0.0f, 0.0f));
	glm::mat4 View = glm::rotate(ViewRotateX, sphi, glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 Model = glm::scale(glm::mat4(1.0f), glm::vec3(sdepth));

	MVP  =  View * Model;

	GLint mvpLocation = glGetUniformLocation(p, "mvp");
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, glm::value_ptr(MVP));
	GLint scaleLocation = glGetUniformLocation(p, "scale");
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(Scale));
	GLint intLocation = glGetUniformLocation(p, "state");
	glUniform1i(intLocation, 0);

	// Textures
	int iSamplerLoc = glGetUniformLocation(p, "gSampler");
	glUniform1i(iSamplerLoc, 0);
	
	dirt.bindTexture(0);
	int iSamplerLoc0 = glGetUniformLocation(p, "gSampler[0]");
	glUniform1i(iSamplerLoc0, 0);

	grass.bindTexture(1);
	int iSamplerLoc1 = glGetUniformLocation(p, "gSampler[1]");
	glUniform1i(iSamplerLoc1, 1);

	rocks.bindTexture(2);
	int iSamplerLoc2 = glGetUniformLocation(p, "gSampler[2]");
	glUniform1i(iSamplerLoc2, 2);

	lightwater.bindTexture(3);
	int iSamplerLoc3 = glGetUniformLocation(p, "gSampler[3]");
	glUniform1i(iSamplerLoc3, 3);

	water.bindTexture(4);
	int iSamplerLoc4 = glGetUniformLocation(p, "gSampler[4]");
	glUniform1i(iSamplerLoc4, 4);

	ocean.bindTexture(5);
	int iSamplerLoc5 = glGetUniformLocation(p, "gSampler[5]");
	glUniform1i(iSamplerLoc5, 5);

	// Set attrib pointer
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) *3,0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) *3,(char *) (19200 * 3 * sizeof(GLfloat)));
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) *2,(char *) (2 * 19200 * 3 * sizeof(GLfloat)));

	// Draw
	glDrawElements(GL_TRIANGLE_STRIP, temp_terrain_ptr->numIndices, GL_UNSIGNED_INT, 0); 

	// Water

		glUniform1i(intLocation, 1);
		//for(int i = 0; i<cArrayLength; ++i){		
		//	temp_terrain_ptr->points->at(i*3 + 1) += temp_world_ptr->waterHeight.at(i);
		//}
		//glBufferSubData(GL_ARRAY_BUFFER, 0, cArrayLength * 3 * sizeof(GLfloat), (*temp_terrain_ptr->points).data());

		glDrawElements(GL_TRIANGLE_STRIP, temp_terrain_ptr->numIndices, GL_UNSIGNED_INT, 0); 


	temp_terrain_ptr.reset();
}

void ModelViewer::mousePressEvent(QMouseEvent *event)
 {
     lastPos = event->pos();
	 
	 int gridX = (lastPos.x() * cDepthWidth) / (this->size().width() * cSampleSize);
	 int gridY = cPointHeight - (lastPos.y() * cDepthHeight) / (this->size().height() * cSampleSize);
	
	 
	 emit setPoint(gridX, gridY);
 }

 void ModelViewer::keyPressEvent( QKeyEvent * event )
{
	if( event->key() == Qt::Key_0 )
    {
        emit setPoint(-100, -100);
    } else if( event->key() == Qt::Key_1 ){
		emit setPoint(-101, -100);
	}  else if( event->key() == Qt::Key_2 ){
		emit setPoint(-102, -100);
	} else if( event->key() == Qt::Key_3 ){
		emit setPoint(-103, -100);
	} else if( event->key() == Qt::Key_4 ){
		emit setPoint(-104, -100);
	} else if( event->key() == Qt::Key_5 ){
		emit setPoint(-105, -100);
	}
}

 void ModelViewer::mouseMoveEvent(QMouseEvent *event)
 {
	 if(state==1)	return;
     int dx = event->x() - lastPos.x();
     int dy = event->y() - lastPos.y();

     if (event->buttons() & Qt::LeftButton) {
         sphi += dx/100.0;
		 stheta += dy/100.0;
	 } else if (event->buttons() & Qt::MiddleButton) {
		 if(sdepth > 0.0001 && sdepth < 0.08){
			sdepth += dy/10000.0;
		 }
     } else if (event->buttons() & Qt::RightButton) {
         stransx += dx/100.0;
		 stransy -= dy/100.0;
     }
     lastPos = event->pos();
 }

 void ModelViewer::mouseReleaseEvent(QMouseEvent * /* event */)
 {
     //emit clicked();

 }
void ModelViewer::printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		cout << infoLog << endl;
		free(infoLog);
	}
}

void ModelViewer::printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		cout << infoLog << endl;
		free(infoLog);
	}
}

void ModelViewer::initShaders(){
	char *vs = NULL, *fs = NULL, *fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = Common::TextFileRead(vertexFileName);
	fs = Common::TextFileRead(fragmentFileName);

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv, NULL);
	glShaderSource(f, 1, &ff, NULL);

	free(vs); free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);

	p = glCreateProgram();
	glAttachShader(p, v);
	glAttachShader(p, f);

	glLinkProgram(p);
	printProgramInfoLog(p);

	glUseProgram(p);
}

void ModelViewer::resizeGL(int width, int height)
{
	if (height == 0) {
		height = 1;
	}
	pMatrix.setToIdentity();
	pMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);
	glViewport(0, 0, width, height);
}

void ModelViewer::initBuffers(){
	indices = std::make_shared<Indices>();
	// Init indices
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

	TextureCoordPtr texCoord = make_shared<TextureCoord>();
	for(int j = 0; j<cPointHeight; j++){
		for(int i = 0; i<cPointWidth; i++){
			(*texCoord)[(i+j*cPointWidth)*2+0] = (i%255)/125.0;
			(*texCoord)[(i+j*cPointWidth)*2+1] = (j%255)/125.0;
		}
	}
	
	glGenBuffers(1, &vertexBufferID); //Num buffers, BufferID
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, (2 * cArrayLength * 3 + cArrayLength * 2)  * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 2 * cArrayLength * 3 * sizeof(GLfloat), cArrayLength * 2 * sizeof(GLfloat), texCoord->data());

	glEnableVertexAttribArray(0);	// Position
	glEnableVertexAttribArray(1);	// Normal
	glEnableVertexAttribArray(2);	// Texture
	// Elements
	glGenBuffers(1, &indexBufferID); //Generate an identifier for the index buffer ID
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID); //Link the index buffer to the element array buffer binding point
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cIndicesLength * sizeof(GLuint), (*indices).data(), GL_STATIC_DRAW);

	bool load;
	load = dirt.loadTexture2D("Dirt.jpg", true);
	assert(load);
	dirt.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	load = grass.loadTexture2D("Grass.jpg", true);
	assert(load);
	grass.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	load = rocks.loadTexture2D("Rocks.png", true);
	assert(load);
	rocks.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	load = water.loadTexture2D("Water.jpg", true);
	assert(load);
	water.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	load = lightwater.loadTexture2D("LightWater.jpg", true);
	assert(load);
	lightwater.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);

	load = ocean.loadTexture2D("Ocean.jpg", true);
	assert(load);
	ocean.setFiltering(CTexture::TEXTURE_FILTER_MAG_BILINEAR, CTexture::TEXTURE_FILTER_MIN_BILINEAR_MIPMAP);
	glEnable(GL_TEXTURE_2D);
}

