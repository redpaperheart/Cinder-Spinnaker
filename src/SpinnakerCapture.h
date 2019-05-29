#pragma once

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "cinder/app/App.h"
#include <iostream>
#include <sstream> 
#include "cinder/Log.h"
#include "cinder/Surface.h"
#include "cinder/gl/gl.h"
#include "cinder/Thread.h"
//#include "cinder/ConcurrentCircularBuffer.h"
//#include "cinder/gl/Context.h"

//#include "rph/ConcurrentDeque.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace ci;
using namespace ci::app;
using namespace std;

class SpinnakerCapture{
	public:
		SpinnakerCapture();
		~SpinnakerCapture();
			
		void												setup(int index=0);
		void												update();
		int													mCamIndex;
		//void												loadTextureThreadFn(ci::gl::ContextRef sharedGlContext);
		void												draw();
		ci::gl::TextureRef									getCamTexture(){ return mCamTexture; }
		//ci::SurfaceRef										getCamSurface(){ return mCamSurfaceRef; }
		//void												saveFramesFromSurfaceRefVector();
		//void												loadVector();

		//bool												bStartRec = false;
		//ci::ConcurrentCircularBuffer<ci::gl::TextureRef>*	mTextureBuffer;
		
		bool												isFrameAvailable();
		bool												mNewFrame = false;
		int													mCaptureCount = 0;
		double												lastFrame;
		float												fps=55; //fps to set camera to max 55
		bool												limitFPS=false;
		bool												mDebugLogs = false;
		bool												mUpdateTexture = true;
		bool												mUpdateSurface = true;
		bool												mGetNextImage = true;

	private:
		int													AcquireImages( CameraPtr pCam,  INodeMap & nodeMap, INodeMap & nodeMapTLDevice );
		int													PrintDeviceInfo(INodeMap & nodeMap);
		
		int													mResX;
		int													mResY;

		
		
		
		//ci::SurfaceRef										mCamSurfaceRef;
		ci::gl::TextureRef									mCamTexture;

		/*SystemPtr											mSystem;
		CameraList											mCamList;
		CameraPtr											mCam = nullptr;*/
		//ImagePtr											mConvertedImage;
		void												convertImage(ImagePtr img);

		bool												mShouldQuit=false;
		shared_ptr<thread>									mThread;
		
		ci::SurfaceRef										mLatestFrame;
		void												captureThreadFn();

		//rph::ConcurrentDeque<ci::SurfaceRef>                mSurfaceQueue;
};

