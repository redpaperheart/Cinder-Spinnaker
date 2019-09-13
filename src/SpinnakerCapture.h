#pragma once

#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include "cinder/app/App.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/Surface.h"
#include "cinder/Thread.h"

#include "SpinnakerSettings.h"

/**********************
Notes
- resizing the surface is not very efficient, even in the thread
- however if resizing to smaller than half or quarter at a lower fps this may be possible
- resizing is currently disabled
- windows only :(

TODO
- there's image tearing in the surface frames on horizontal motion
	- does not seem to be related to vsync
	- perhaps coming from the mem copy from the image to the surface
- get the current exposure and gain values from camera if set to auto or 1
*********************/
namespace rph {
	using SpinnakerCaptureRef = std::shared_ptr<class SpinnakerCapture>;
	class SpinnakerCapture {
	public:
		static SpinnakerCaptureRef create() {
			return std::make_shared<SpinnakerCapture>();
		}

		SpinnakerCapture();
		~SpinnakerCapture();

//		struct CameraOptions {
//			CameraOptions() {}
//			//almost always 0 unless you have more than one camera attached
//			int camIndex = 0; //start as -1?
//			std::string camSerial = ""; //serial for camera device if known
//			//max 55 -- may crash if the fps is not compatible with cam
//			//TODO put in checks/limits when setting fps
//			int fps = 55;
//			glm::ivec2 res = glm::vec2(2048, 1536);
//
//			//for auto values
//			// 0 == OFF
//			// 1 == Once (sets value once based on current lighting then sets itself to off)
//			// 2 == Continuous (constantly updates)
////TODO change these to 'modes'
//			int autoExposureValue = 0;
//			int autoGainValue = 0;
//			int autoWhiteBalanceValue = 1;
//
//			//non auto exposure and gain settings, be sure auto values are set to off
//			int exposureTime = 15000;
//			float gain = 10;
//			//const char* AUTO_MODES = "OFF\0ONCE\0CONTINUOUS";
//		};
		
		void												setup(SpinnakerSettingsRef settings = SpinnakerSettings::create("default"));
		//CameraOptions										mOptions;

		SpinnakerSettingsRef	mSettings;

		void applySettings(SpinnakerSettingsRef settings);
		void												update();
		void												draw();
		ci::gl::TextureRef									getCamTexture() { return mCamTexture; }

		//start the thread
		void start();
		//stop the thread
		void stop();
		bool isStarted() {
			return mStarted;
		}
		//TODO add getters for size

		bool												mNewFrame = false;
		bool												mDebugLogs = false;
		bool												mUpdateTexture = true;
		bool												mUpdateSurface = true;
		bool												mGetNextImage = true;
		double lastFrameTime = 0; //time in seconds that the last frame was grabbed
	private:
		void										PrintDeviceInfo(Spinnaker::GenApi::INodeMap & nodeMap);
		//is there a new frame from the thread
		
		bool										isFrameAvailable();
		bool										mStarted = false;

		ci::gl::TextureRef							mCamTexture;

		Spinnaker::CameraPtr mCam;
		std::atomic<bool>							mShouldQuit = false;
		std::atomic<bool>							mJoinThread = false;
		//std::atomic<bool>							mConnected = false;
		void										closeThread();
		std::shared_ptr<std::thread>				mThread;
		//do some additional settings for the camera (reduce buffer to 1 frame)
		void										setupCamera(Spinnaker::CameraPtr camera);

		//get image from the camera, convert it then copy into a surface
		int											AcquireImages(Spinnaker::CameraPtr pCam, Spinnaker::GenApi::INodeMap & nodeMap, Spinnaker::GenApi::INodeMap & nodeMapTLDevice);

		//most recent surface from the thread
		ci::SurfaceRef								mLatestFrame;
		void										captureThreadFn(SpinnakerSettingsRef settings);

		
	};
}
//namespace rph

