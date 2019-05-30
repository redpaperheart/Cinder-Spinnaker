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

class SpinnakerCapture{
	public:
		SpinnakerCapture();
		~SpinnakerCapture();

		struct CameraOptions{
			int camIndex = 0;
			float fps = 55;
			vec2 resolution = vec2(2048, 1536);
		};
			
		void												setup(int index=0);
		void												update();
		int													mCamIndex;
		void												draw();
		ci::gl::TextureRef									getCamTexture(){ return mCamTexture; }
		
		float												fps = 55; //fps to set camera to max 55

		bool												mNewFrame = false;
		int													mCaptureCount = 0;
		double												lastFrame;
		
		bool												limitFPS=false;
		bool												mDebugLogs = false;
		bool												mUpdateTexture = true;
		bool												mUpdateSurface = true;
		bool												mGetNextImage = true;

	private:
		int													AcquireImages(Spinnaker::CameraPtr pCam, Spinnaker::GenApi::INodeMap & nodeMap, Spinnaker::GenApi::INodeMap & nodeMapTLDevice );
		int													PrintDeviceInfo(Spinnaker::GenApi::INodeMap & nodeMap);
		bool												isFrameAvailable();
		
		//int													mResX;
		//int													mResY;		
		ci::gl::TextureRef									mCamTexture;

		bool												mShouldQuit=false;
		std::shared_ptr<std::thread>								mThread;
		
		ci::SurfaceRef										mLatestFrame;
		void												captureThreadFn();
};

