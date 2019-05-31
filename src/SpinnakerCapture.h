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
			CameraOptions() {}
			//almost always 0 unless you have more than one camera attached
			int camIndex = 0;

			//max 55 -- may crash if the fps is not compatible with cam
			//TODO put in checks/limits when setting fps
			float fps = 55;
			glm::vec2 res = glm::vec2(2048, 1536);

			//for auto values
			// 0 == OFF
			// 1 == Once (sets value once based on current lighting then sets itself to off)
			// 2 == Continuous (constantly updates)
			int autoExposureValue = 0;
			int autoGainValue = 0;
			int autoWhiteBalanceValue = 1;
			
			//non auto exposure and gain settings, be sure auto values are set to off
			int exposureTime = 15000;
			float gain = 10;
			
			//TODO
			//boolean +vec2 to resize the incoming data to something?
			bool resize = false;
			glm::vec2 resizeRes = glm::vec2(1000, 500);
		};
			
		void												setup(CameraOptions options= CameraOptions());
		CameraOptions										mOptions;
		void												update();
		void												draw();
		ci::gl::TextureRef									getCamTexture(){ return mCamTexture; }

		//start the thread
		void start();
		//stop the thread
		void stop();
		bool isStarted() {
			return mStarted;
		}

		bool												mNewFrame = false;
		bool												mDebugLogs = false;
		bool												mUpdateTexture = true;
		bool												mUpdateSurface = true;
		bool												mGetNextImage = true;

	private:
		void										PrintDeviceInfo(Spinnaker::GenApi::INodeMap & nodeMap);
		//is there a new frame from the thread
		bool										isFrameAvailable();
		bool										mStarted = false;
				
		ci::gl::TextureRef							mCamTexture;

		std::atomic<bool>							mShouldQuit=false;
		std::atomic<bool>							mJoinThread = false;
		std::atomic<bool>							mConnected = false;
		void										closeThread();
		std::shared_ptr<std::thread>				mThread;
		//do some additional settings for the camera (reduce buffer to 1 frame)
		void										setupCamera(Spinnaker::CameraPtr camera);

		//get image from the camera, convert it then copy into a surface
		int											AcquireImages(Spinnaker::CameraPtr pCam, Spinnaker::GenApi::INodeMap & nodeMap, Spinnaker::GenApi::INodeMap & nodeMapTLDevice);
		
		//most recent surface from the thread
		ci::SurfaceRef								mLatestFrame;
		void										captureThreadFn(SpinnakerCapture::CameraOptions options);
};

