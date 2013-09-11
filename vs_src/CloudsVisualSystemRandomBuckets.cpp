//
//  CloudsVisualSystemRandomBuckets.cpp
//

#include "CloudsVisualSystemRandomBuckets.h"
#include "CloudsRGBDVideoPlayer.h"


/**
 * -flipfloping random buckets fbo.
 *		-accumlating noise values that get emptied once they pass 1. each noise value gets divided by a scaling coeffecient(quin used 200)
 *
 * -scalable vbo of n * n * n buckets(cubes)
 *		-vertex attribute per cube: cubeCenter for sampling
 *		-vertex attribute per vertex: vertex position(-.5 <-> .5)
 *		-the shader scales and repositions the cubes based on the scale so that they don't overlap the edges of the broader cube
 *
 * -fancy rendering
 */
 
 

//These methods let us add custom GUI parameters and respond to their events
void CloudsVisualSystemRandomBuckets::selfSetupGui(){

//	connectorGui = new ofxUISuperCanvas("CONNECTORS", gui);
//	connectorGui->copyCanvasStyle(gui);
//	connectorGui->copyCanvasProperties(gui);
//	connectorGui->setName("Custom");
//	connectorGui->setWidgetFontSize(OFX_UI_FONT_SMALL);
//	
//	connectorGui->addSlider("Num Particles", 50, 64*64, &generator.numParticles);
//	connectorGui->addToggle("Draw Connections", &generator.drawConnections);
//	connectorGui->addSlider("Min Connection Distance", 1, 100, &generator.minDistance);
//	connectorGui->addSlider("Boundary Size", 100, 1000, &generator.boundarySize);
//	
//	connectorGui->addSlider("Max Connections", 1, 10, &generator.maxConnections);
//
//	
////	ofAddListener(customGui->newGUIEvent, this, &CloudsVisualSystemRandomBuckets::selfGuiEvent);
//	
//	guis.push_back(connectorGui);
//	guimap[connectorGui->getName()] = connectorGui;
}


void CloudsVisualSystemRandomBuckets::selfGuiEvent(ofxUIEventArgs &e){
	if(e.widget->getName() == "Custom Button"){
		cout << "Button pressed!" << endl;
	}
}

//Use system gui for global or logical settings, for exmpl
void CloudsVisualSystemRandomBuckets::selfSetupSystemGui(){
	
}

void CloudsVisualSystemRandomBuckets::guiSystemEvent(ofxUIEventArgs &e){
	
}
//use render gui for display settings, like changing colors
void CloudsVisualSystemRandomBuckets::selfSetupRenderGui(){

}

void CloudsVisualSystemRandomBuckets::guiRenderEvent(ofxUIEventArgs &e){
	
}

// selfSetup is called when the visual system is first instantiated
// This will be called during a "loading" screen, so any big images or
// geometry should be loaded here
void CloudsVisualSystemRandomBuckets::selfSetup()
{
	
	//create a huge vbo filled with cubes
	//
	
	//make our base cube vertices and indices
	vector<ofVec3f> cubeVertices(8);
	//bottom
	cubeVertices[0].set(-.5, -.5,-.5);
	cubeVertices[1].set(-.5, -.5, .5);
	cubeVertices[2].set( .5, -.5, .5);
	cubeVertices[3].set( .5, -.5,-.5);
	
	//top
	cubeVertices[4].set(-.5, .5,-.5);
	cubeVertices[5].set(-.5, .5, .5);
	cubeVertices[6].set( .5, .5, .5);
	cubeVertices[7].set( .5, .5,-.5);
	
	//indices. quads so we need for indices per face
	vector<ofIndexType> cubeIndices;
	//bottom and top face
	cubeIndices.push_back(0);
	cubeIndices.push_back(1);
	cubeIndices.push_back(2);
	cubeIndices.push_back(3);
	cubeIndices.push_back(4);
	cubeIndices.push_back(5);
	cubeIndices.push_back(6);
	cubeIndices.push_back(7);
	
	//left and right face
	cubeIndices.push_back(4);
	cubeIndices.push_back(5);
	cubeIndices.push_back(1);
	cubeIndices.push_back(0);
//	cubeIndices.push_back(6);
//	cubeIndices.push_back(7);
//	cubeIndices.push_back(3);
//	cubeIndices.push_back(2);
	
	//front and back face
	cubeIndices.push_back(5);
	cubeIndices.push_back(6);
	cubeIndices.push_back(2);
	cubeIndices.push_back(1);
//	cubeIndices.push_back(0);
//	cubeIndices.push_back(3);
//	cubeIndices.push_back(7);
//	cubeIndices.push_back(4);
	
	cout << "started populating cubes at: " << ofGetElapsedTimeMillis() << endl;
	
	int dim = 20;//<--- dim * dim * dim = our number of cubes
	int cubeVertexCount=cubeVertices.size(), cubeIndexCount=cubeIndices.size(), vertexCount=0;;
	ofIndexType index;
	vector<ofIndexType> indices( dim * dim * dim * cubeIndexCount );
	vector<ofVec3f> vertices( dim * dim * dim * cubeVertexCount );
	vector<ofVec3f> cubeCenters( vertices.size() );
	vector<ofVec2f> tc( vertices.size() );
	
	bCullFace = true;
	cubeDim = dim;
	
	//ping ponging randomness textures
	float numTextureSamples = cubeDim * cubeDim * cubeDim;// * cubeVertices.size() ;
	int texDim = ceil( sqrt( numTextureSamples ));
	
	pingFbo.allocate( texDim, texDim, GL_RGBA32F );
	pongFbo.allocate( texDim, texDim, GL_RGBA32F );
	
	pingFbo.begin();
	ofClear(0, 0, 0);
	pingFbo.end();
	
	pongFbo.begin();
	ofClear(0, 0, 0);
	pongFbo.end();
	
	ping = &pingFbo;
	pong = &pongFbo;
	
	vector<float> initialRandomVals( pingFbo.getWidth() * pingFbo.getHeight() );
	for (int i=0; i<pingFbo.getWidth(); i++)
	{
		for (int j=0; j<pingFbo.getHeight(); j++)
		{
			initialRandomVals[i*pingFbo.getHeight() + j] = ofRandom(-1,2);
		}
	}
	
	pingFbo.getTextureReference(0).loadData( &initialRandomVals[0], pingFbo.getWidth(), pingFbo.getHeight(), GL_LUMINANCE );
	pongFbo.getTextureReference(0).loadData( &initialRandomVals[0], pingFbo.getWidth(), pingFbo.getHeight(), GL_LUMINANCE );
	
	pingFbo.getTextureReference(0).setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	pongFbo.getTextureReference(0).setTextureMinMagFilter(GL_NEAREST, GL_NEAREST);
	
	//add a whole bunch cubes to the vbo
	int currentIndex;
	int tcX = 0, tcY = 0;
	for(int i=0; i<dim; i++)
	{
		for (int j=0; j<dim; j++)
		{
			for (int k=0; k<dim; k++)
			{
				//store our cube's starting index
				index = vertexCount;
				
				//add a cubes worth of vertices & vertex attributes(centers, tex coords)
				for (int l=0; l<cubeVertices.size(); l++)
				{
					currentIndex = i*dim*dim*cubeVertexCount + j*dim*cubeVertexCount + k*cubeVertexCount + l;
					
					//corners will be combined with the cube center in the shader
					vertices[ currentIndex ] = cubeVertices[l];
					vertexCount++;
					
					//cube centers & an offset to center everything at 0,0,0
					cubeCenters[ currentIndex ] = ofVec3f(i,j,k) - ofVec3f(.5 * dim);
					
					//cube texCoord
					tc[ currentIndex ].set( tcX, tcY);// (i*dim*dim + j*dim) % texDim, (i*dim*dim + j*dim + k) % texDim );
				}
				tcX+=1;
				if( tcX > texDim )
				{
//					cout <<texDim << " : " << tcX << ", "<< tcY<< ", " << tc[ currentIndex ] << endl;
					tcX = 0;
					tcY+=1;
				}
				
				//add a cubes worth of indices
				for (int l=0; l<cubeIndices.size(); l++)
				{
					indices[ i*dim*dim*cubeIndexCount + j*dim*cubeIndexCount + k*cubeIndexCount + l] = index + cubeIndices[l];
				}
			}
		}
	}
	
	
	indexCount = indices.size();
	cubeColor.set(1, 1, 1,.01);
	
	cubesShader.load( getVisualSystemDataPath() + "shaders/normalShader" );
	randomnessShader.load( getVisualSystemDataPath() + "shaders/randomness" );
	
	
//	int tcCount = 0, tcTotal = tc.size();
//	for(int i=0; i<texDim; i++)
//	{
//		for(int j=0; j<texDim; j++)
//		{
//			if (tcCount < tcTotal)
//			{
//				tc[tcCount].set(i,j);
//				tcCount++;
//			}
//		}
//	}
	
	cubesVbo.setVertexData( &vertices[0], vertices.size(), GL_STATIC_DRAW );
	cubesVbo.setNormalData( &cubeCenters[0], cubeCenters.size(), GL_STATIC_DRAW );
	cubesVbo.setTexCoordData( &tc[0], tc.size(), GL_STATIC_DRAW );
	cubesVbo.setIndexData( &indices[0], indices.size(), GL_STATIC_DRAW );
	
	cout << "ended populating cubes at: " << ofGetElapsedTimeMillis() << endl;
	cout << "vertexCount: "<< vertexCount << endl;
	cout << "indexCount: "<< indexCount << endl;
}

// selfPresetLoaded is called whenever a new preset is triggered
// it'll be called right before selfBegin() and you may wish to
// refresh anything that a preset may offset, such as stored colors or particles
void CloudsVisualSystemRandomBuckets::selfPresetLoaded(string presetPath){
	
}

// selfBegin is called when the system is ready to be shown
// this is a good time to prepare for transitions
// but try to keep it light weight as to not cause stuttering
void CloudsVisualSystemRandomBuckets::selfBegin(){
	
}

//do things like ofRotate/ofTranslate here
//any type of transformation that doesn't have to do with the camera
void CloudsVisualSystemRandomBuckets::selfSceneTransformation(){
	
}

//normal update call
void CloudsVisualSystemRandomBuckets::selfUpdate(){
	
//	generator.update();

}

// selfDraw draws in 3D using the default ofEasyCamera
// you can change the camera by returning getCameraRef()
void CloudsVisualSystemRandomBuckets::selfDraw()
{
	//ping pong the fbos
	ofFbo* swapper = ping;
	ping = pong;
	pong = swapper;
	
	ofRandom(1);
	
	
	ping->begin();
	ofClear(255);
	//draw randomness to ping using a shader
	
	randomnessShader.begin();
	randomnessShader.setUniformTexture("pong", pong->getTextureReference(), 0);
	randomnessShader.setUniform1f("randSeed", ofGetElapsedTimef() );
	
	pong->draw( 0, 0, ping->getWidth(), ping->getHeight() );
//	pong->draw(0, pong->getHeight(), pong->getWidth(), -pong->getHeight() );
	
	randomnessShader.end();
	
	ping->end();
	
	
	//render attributes: make these into sliders
	float gridScale = 20 * 20. / cubeDim;
	
	
	//transform and draw
	ofPushStyle();
	ofPushMatrix();
	ofScale(gridScale,gridScale,gridScale);
	
	ofSetColor(cubeColor);

	glDisable(GL_DEPTH_TEST);
	
//	ofEnableBlendMode( OF_BLENDMODE_MULTIPLY );
	ofEnableAlphaBlending();
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	cubesShader.begin();
	cubesShader.setUniform4f("color", cubeColor.r, cubeColor.g, cubeColor.b, cubeColor.a );
	cubesShader.setUniform1f("minY", cubeDim * -.5 );
	cubesShader.setUniform1f("maxY", cubeDim * .5 );
	cubesShader.setUniformTexture("randomness", ping->getTextureReference(), 0 );
	cubesShader.setUniform1f( "maxCubeScale", cubeDim );
	cubesShader.setUniform1f( "rExpo", 2.5);
	cubesShader.setUniform1f( "rScale", 1.1 );
	
	cubesVbo.drawElements( GL_QUADS, indexCount );
	
	cubesShader.end();
	
	ofPopMatrix();
	ofPopStyle();
	
}

// draw any debug stuff here
void CloudsVisualSystemRandomBuckets::selfDrawDebug(){
//	generator.drawBins();
}

// or you can use selfDrawBackground to do 2D drawings that don't use the 3D camera
void CloudsVisualSystemRandomBuckets::selfDrawBackground(){

	//turn the background refresh off
	//bClearBackground = false;
	
	if (guis[0]->isVisible())
	{
		pingFbo.draw( 10, ofGetHeight()-20 - pingFbo.getHeight()*2, pingFbo.getWidth(), pingFbo.getHeight());
		pongFbo.draw( 10, ofGetHeight()-10 - pongFbo.getHeight(), pongFbo.getWidth(), pongFbo.getHeight());
	}
	
}
// this is called when your system is no longer drawing.
// Right after this selfUpdate() and selfDraw() won't be called any more
void CloudsVisualSystemRandomBuckets::selfEnd(){
	
//	simplePointcloud.clear();
	
}
// this is called when you should clear all the memory and delet anything you made in setup
void CloudsVisualSystemRandomBuckets::selfExit(){
	cubesVbo.clear();
}

//events are called when the system is active
//Feel free to make things interactive for you, and for the user!
void CloudsVisualSystemRandomBuckets::selfKeyPressed(ofKeyEventArgs & args){
	if(args.key == 'l')
	{
		cubesShader.load( getVisualSystemDataPath() + "shaders/normalShader" );
		randomnessShader.load( getVisualSystemDataPath() + "shaders/randomness" );
	}
}
void CloudsVisualSystemRandomBuckets::selfKeyReleased(ofKeyEventArgs & args){
	
}

void CloudsVisualSystemRandomBuckets::selfMouseDragged(ofMouseEventArgs& data){
	
}

void CloudsVisualSystemRandomBuckets::selfMouseMoved(ofMouseEventArgs& data){
	
}

void CloudsVisualSystemRandomBuckets::selfMousePressed(ofMouseEventArgs& data){
	
}

void CloudsVisualSystemRandomBuckets::selfMouseReleased(ofMouseEventArgs& data){
	
}