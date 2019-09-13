#include "SpinnakerCapture.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace ci;
using namespace ci::app;
using namespace std;

namespace rph {
	SpinnakerCapture::SpinnakerCapture(){
	
	}

	SpinnakerCapture::~SpinnakerCapture(){
		stop();
	}

	void SpinnakerCapture::setup(SpinnakerSettingsRef settings){
		//mOptions = options;
		mSettings = settings;
		mLatestFrame = ci::Surface::create(settings->mResolution.x, settings->mResolution.y, false, ci::SurfaceChannelOrder::RGB);
		mCamTexture = ci::gl::Texture::create(settings->mResolution.x, settings->mResolution.y);
	}

	void SpinnakerCapture::update() {
		if (mJoinThread) {
			closeThread();
		}
		if (mUpdateTexture && isFrameAvailable()) {
			mCamTexture->update(*mLatestFrame);
			mNewFrame = false;
			lastFrameTime = getElapsedSeconds();
			if (mDebugLogs) ci::app::console() << "new frame ------------------" << std::endl;
		}
		else {
			if (mDebugLogs) ci::app::console() << "waiting on a new camera frame" << std::endl;
		}
	}
	void SpinnakerCapture::draw() {
		//draw the texture
		ci::gl::draw(mCamTexture);
	}

	void SpinnakerCapture::applySettings(SpinnakerSettingsRef settings) {
		//try to update the exposure
		if (mCam) {
			//set camera from options
			mCam->ExposureAuto.SetValue(Spinnaker::ExposureAutoEnums(settings->mAutoExposureMode));
			if (settings->mAutoExposureMode == 0) mCam->ExposureTime.SetValue(settings->mExposureTime);

			mCam->GainAuto.SetValue(Spinnaker::GainAutoEnums(settings->mAutoGainMode));
			if (settings->mAutoGainMode == 0) mCam->Gain.SetValue(settings->mGain);

			mCam->BalanceWhiteAuto.SetValue(Spinnaker::BalanceWhiteAutoEnums(settings->mAutoWhiteMode)); //can also be set to once

			mCam->AcquisitionFrameRateEnable.SetValue(true);
			mCam->AcquisitionFrameRate.SetValue(settings->mFPS);
		}
		
	}
	void SpinnakerCapture::start() {
		if (mStarted) return; //already running
		CI_LOG_I("----------------starting");
		mStarted = true;
		mShouldQuit = false;
		//camera setup all happens in the thread
		mConnected = true;
		mThread = std::shared_ptr<std::thread>(new std::thread(std::bind(&SpinnakerCapture::captureThreadFn, this, mSettings)));
	}
	void SpinnakerCapture::stop() {
		if (!mStarted) return; //can't stop; not running
		//stop the thread
		mShouldQuit = true;
		closeThread();
	
		mStarted = false;
	}
	void SpinnakerCapture::closeThread() {
		mJoinThread = false;
		CI_LOG_I("closeThread");
		try {
			CI_LOG_I("joinable: " << mThread->joinable());
			mThread->join();
		}
		catch (...) {
			//CI_LOG_I("Error: "<< ex.what());
		}
	}
	void SpinnakerCapture::PrintDeviceInfo(INodeMap & nodeMap) {
		cout << endl << "*** DEVICE INFORMATION ***" << endl;

		try {
			FeatureList_t features;
			CCategoryPtr category = nodeMap.GetNode("DeviceInformation");
			if (IsAvailable(category) && IsReadable(category)) {
				category->GetFeatures(features);

				FeatureList_t::const_iterator it;
				for (it = features.begin(); it != features.end(); ++it) {
					CNodePtr pfeatureNode = *it;
					cout << pfeatureNode->GetName() << " : ";
					CValuePtr pValue = (CValuePtr)pfeatureNode;
					cout << (IsReadable(pValue) ? pValue->ToString() : "Node not readable");
					cout << endl;
				}
			}
			else
			{
				cout << "Device control information not available." << endl;
			}
		}
		catch (Spinnaker::Exception &e) {
			CI_LOG_I("Error: " << e.what());
		}
	}

	bool SpinnakerCapture::isFrameAvailable() {
		return mNewFrame;
	}

	void SpinnakerCapture::setupCamera(CameraPtr camera) {
		//set buffer to maual then set buffer_count to 1

		////////////////////////////////////////////////////
		// Retrieve Stream Parameters device nodemap 
		Spinnaker::GenApi::INodeMap & sNodeMap = camera->GetTLStreamNodeMap();

		// Retrieve Buffer Handling Mode Information
		CEnumerationPtr ptrHandlingMode = sNodeMap.GetNode("StreamBufferHandlingMode");
		if (!IsAvailable(ptrHandlingMode) || !IsWritable(ptrHandlingMode))
		{
			cout << "Unable to set Buffer Handling mode (node retrieval). Aborting..." << endl << endl;
			//return -1;
		}

		CEnumEntryPtr ptrHandlingModeEntry = ptrHandlingMode->GetCurrentEntry();
		if (!IsAvailable(ptrHandlingModeEntry) || !IsReadable(ptrHandlingModeEntry))
		{
			CI_LOG_I("Unable to set Buffer Handling mode (Entry retrieval). Aborting...");
			//return -1;
		}

		// Set stream buffer Count Mode to manual
		CEnumerationPtr ptrStreamBufferCountMode = sNodeMap.GetNode("StreamBufferCountMode");
		if (!IsAvailable(ptrStreamBufferCountMode) || !IsWritable(ptrStreamBufferCountMode))
		{
			CI_LOG_I("Unable to set Buffer Count Mode (node retrieval). Aborting...");
			//return -1;
		}

		CEnumEntryPtr ptrStreamBufferCountModeManual = ptrStreamBufferCountMode->GetEntryByName("Manual");
		if (!IsAvailable(ptrStreamBufferCountModeManual) || !IsReadable(ptrStreamBufferCountModeManual))
		{
			CI_LOG_I("Unable to set Buffer Count Mode entry (Entry retrieval). Aborting...");
		}

		ptrStreamBufferCountMode->SetIntValue(ptrStreamBufferCountModeManual->GetValue());

		//CI_LOG_I("Stream Buffer Count Mode set to manual...");

		// Retrieve and modify Stream Buffer Count
		CIntegerPtr ptrBufferCount = sNodeMap.GetNode("StreamBufferCountManual");
		if (!IsAvailable(ptrBufferCount) || !IsWritable(ptrBufferCount))
		{
			CI_LOG_I("Unable to set Buffer Count (Integer node retrieval). Aborting...");
		}

		/*CIntegerPtr ptrBufferCountMax = sNodeMap.GetNode("StreamBufferCountMax");
		if (!IsAvailable(ptrBufferCountMax) || !IsWritable(ptrBufferCountMax))
		{
			CI_LOG_I("Unable to set Buffer Count Max (Integer node retrieval). Aborting...");
		}*/

		// Display Buffer Info
		//CI_LOG_I("Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName());
		//CI_LOG_I("Default Buffer Count: " << ptrBufferCount->GetValue());

		//change handling mode to newest first
		//ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestFirst");
		//ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
		//CI_LOG_I("Change Buffer Handling Mode?: " << ptrHandlingModeEntry->GetDisplayName());

		ptrBufferCount->SetValue(1);
		//cout << "Buffer count now set to: " << ptrBufferCount->GetValue() << endl;
		//CI_LOG_I("Buffer count now set to: " << ptrBufferCount->GetValue());
		////////////////////////////////////////////////////
	}

	void SpinnakerCapture::captureThreadFn(SpinnakerSettingsRef settings) {
		ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
		ci::app::console() << "CAPTURE THREAD STARTED" << std::endl;
	
		Spinnaker::SystemPtr tSystem = Spinnaker::System::GetInstance();
		CameraList mCamList;
		//CI_LOG_I("finding cam");
		ci::app::console() << "finding cam" << std::endl;
		try {
			//CI_LOG_I("0");
			mCamList = tSystem->GetCameras();
			//CI_LOG_I("A");
			if (settings->mDeviceIndex >= 0) {
				//CI_LOG_I("B");
				if (mCamList.GetSize() > 0 || mCamList.GetSize() - 1 >= settings->mDeviceIndex) {
					mCam = mCamList.GetByIndex(settings->mDeviceIndex);
					settings->mDeviceSerial = mCam->DeviceSerialNumber.GetValue(true);
				}
				else {
					//CI_LOG_I("mCamList does not contain cam index:" << settings->mDeviceIndex << " Camlist size: " << mCamList.GetSize());
					//ci::app::console() << "mCamList does not contain cam index:" << settings->mDeviceIndex << " Camlist size: " << mCamList.GetSize() << std::endl;
				}
			}
			else {
				//setup by serial, useful when using mutiple cams
				//CI_LOG_I("cam from serial:" << settings->mDeviceSerial);
				try {
					mCam = mCamList.GetBySerial(settings->mDeviceSerial);
					for (int i = 0; i < mCamList.GetSize(); i++) {
						if (mCamList[i] == mCam) {
							settings->mDeviceIndex = i;
						}
					}
				}
				catch (Spinnaker::Exception& e) {
					//ci::app::console() << "Error: " << e.what() << std::endl;
					//CI_LOG_I("Find cam by serial error: " << e.what());
				}
				//find the camera in the list
			}
		}
		catch (Spinnaker::Exception& e) {
			ci::app::console() << "Error: " << e.what() << std::endl;
		}
		//CI_LOG_I("found a cam complete, validating cam");
		
		mCamList.Clear();
		//make sure we found a cam
		ci::app::console() << "validate cam" << std::endl;
		if (mCam == NULL) {
			ci::app::console() << "no cam found closing" << std::endl;
			//release the system ref
			//try {
			//	ci::app::console() << "tSystem->ReleaseInstance();" << std::endl;
			//	tSystem->ReleaseInstance();
			//}
			//catch (Spinnaker::Exception& e) {
			//	ci::app::console() << "Error: " << e.what() << std::endl;
			//}
			ci::app::console() << "End Capture Thread" << std::endl;
			mConnected = false;
			mStarted = false;
			mJoinThread = true;
			//CI_LOG_I("closed");
			return;
		}

		//setup the camera
		mCam->Init();

		//reduce the buffer of the camera, messy so pulled into it's own method
		setupCamera(mCam);
		
		//settings like fps and autoexposure
		applySettings(settings);
		
		//start capturing
		mCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
		mCam->BeginAcquisition();

		while ((!mShouldQuit)) {
			if (mGetNextImage) {
				try {
					INodeMap & nodeMapTLDevice = mCam->GetTLDeviceNodeMap();
					INodeMap & nodeMap = mCam->GetNodeMap();
					if (AcquireImages(mCam, nodeMap, nodeMapTLDevice) < 0) {
						ci::app::console() << "error acquiring images" << std::endl;
						//close thread
						mShouldQuit = true;
						//camera disconnected, reset index
						mSettings->mDeviceIndex = -1;
					}
				}
				catch (Spinnaker::Exception &e) {
					ci::app::console() << "Error: " << e.what() << std::endl;
				}
			}
		}

		try {
			ci::app::console() << "mCam->EndAcquisition()" << std::endl;
			mCam->EndAcquisition();
		}
		catch (Spinnaker::Exception &e) {
			ci::app::console() << "Error: " << e.what() << std::endl;
		}
		try {
			ci::app::console() << "mCam->DeInit();" << std::endl;
			mCam->DeInit();
			mCam = nullptr;
		}
		catch (Spinnaker::Exception &e) {
			ci::app::console() << "Error: " << e.what() << std::endl;
		}

		mConnected = false;
		mStarted = false;
		mJoinThread = true;
		
		ci::app::console() << "CAPTURE THREAD STOPPED" << std::endl;
	}
	int SpinnakerCapture::AcquireImages(CameraPtr pCam, INodeMap & nodeMap, INodeMap & nodeMapTLDevice) {

		int result = 0;
		try {
			ImagePtr nextImg = pCam->GetNextImage(1000);
			if (!nextImg->IsIncomplete()) {
				if (mUpdateSurface) {
					ImagePtr mConvertedImage;
					mConvertedImage = nextImg->Convert(PixelFormat_RGB8, NEAREST_NEIGHBOR);
					auto mCamPixelData = mConvertedImage->GetData();
					memcpy(mLatestFrame->getData(), mCamPixelData, pCam->Width.GetValue() * pCam->Height.GetValue() * 3);
					mNewFrame = true;
				}
			}
			else {
				ci::app::console() << "nextImg->IsIncomplete()" << std::endl;
			}
			nextImg->Release();
		}
		catch (Spinnaker::Exception &e) {
			CI_LOG_I("Error: " << e.what());
			result = -1;
		}

		return result;
	}
}
//namespace rph