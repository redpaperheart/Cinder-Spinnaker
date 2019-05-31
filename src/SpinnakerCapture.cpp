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

	void SpinnakerCapture::setup(CameraOptions options){
		mOptions = options;
		mLatestFrame = ci::Surface::create(mOptions.res.x, mOptions.res.y, false, ci::SurfaceChannelOrder::RGB);
		mCamTexture = ci::gl::Texture::create(mOptions.res.x, mOptions.res.y);
	}
	void SpinnakerCapture::start() {
		if (mStarted) return; //already running

		mStarted = true;
		mShouldQuit = false;
		//camera setup all happens in the thread
		mConnected = true;
		mThread = std::shared_ptr<std::thread>(new std::thread(std::bind(&SpinnakerCapture::captureThreadFn, this, mOptions)));
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


	void SpinnakerCapture::update(){
		if (mJoinThread) {
			closeThread();
		}
		if (mUpdateTexture && isFrameAvailable()) {
			mCamTexture->update(*mLatestFrame);
			mNewFrame = false;
			if(mDebugLogs) ci::app::console() << "new frame ------------------" << std::endl;
		}
		else {
			if(mDebugLogs) ci::app::console() << "waiting on a new camera frame" << std::endl;
		}
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

		CI_LOG_I("Stream Buffer Count Mode set to manual...");

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
		CI_LOG_I("Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName());
		CI_LOG_I("Default Buffer Count: " << ptrBufferCount->GetValue());
		//CI_LOG_I("Maximum Buffer Count: " << ptrBufferCount->GetMax());

		//change handling mode to newest first
		//ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestFirst");
		//ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
		//CI_LOG_I("Change Buffer Handling Mode?: " << ptrHandlingModeEntry->GetDisplayName());

		ptrBufferCount->SetValue(1);
		//cout << "Buffer count now set to: " << ptrBufferCount->GetValue() << endl;
		CI_LOG_I("Buffer count now set to: " << ptrBufferCount->GetValue());
		////////////////////////////////////////////////////
	}

	void SpinnakerCapture::captureThreadFn(SpinnakerCapture::CameraOptions options) {
		ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
		ci::app::console() << "CAPTURE THREAD STARTED" << std::endl;
	
		Spinnaker::SystemPtr tSystem = Spinnaker::System::GetInstance();
		CameraList mCamList;
		try {
			//get all cameras
			mCamList = tSystem->GetCameras();
			if (mCamList.GetSize() == 0 || mCamList.GetSize() - 1 < options.camIndex) {
				ci::app::console() << "mCamList does not contain cam index:"<< options.camIndex<< " Camlist size: " << mCamList.GetSize() << std::endl;
				mCamList.Clear();
				//release the system ref
				try {
					ci::app::console() << "tSystem->ReleaseInstance();" << std::endl;
					tSystem->ReleaseInstance();
				}
				catch (Spinnaker::Exception &e) {
					ci::app::console() << "Error: " << e.what() << std::endl;
				}
				ci::app::console() << "End Capture Thread" << std::endl;
				mConnected = false;
				mJoinThread = true;
				return;
			}
		}
		catch (Spinnaker::Exception &e) {
			ci::app::console() << "Error: " << e.what() << std::endl;
		}

		//setup the camera
		CameraPtr mCam = mCamList.GetByIndex(options.camIndex);
		//once we have the camera clear the cam list
		mCamList.Clear();

		mCam->Init();
		//reduce the buffer of the camera, wa messy so pulled into it's own method
		setupCamera(mCam);

		//set camera from options
		mCam->ExposureAuto.SetValue(Spinnaker::ExposureAutoEnums(options.autoExposureValue));
		if (options.autoExposureValue == 0) mCam->ExposureTime.SetValue(options.exposureTime);

		mCam->GainAuto.SetValue(Spinnaker::GainAutoEnums(options.autoGainValue));
		if (options.autoGainValue == 0) mCam->Gain.SetValue(options.gain);

		mCam->BalanceWhiteAuto.SetValue(Spinnaker::BalanceWhiteAutoEnums(options.autoWhiteBalanceValue)); //can also be set to once

		mCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

		mCam->AcquisitionFrameRateEnable.SetValue(true);
		mCam->AcquisitionFrameRate.SetValue(options.fps);

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
					}
				}
				catch (Spinnaker::Exception &e) {
					ci::app::console() << "Error: " << e.what() << std::endl;
				}
			}
		}

		//clean up the camera
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

		//release the system ref
		try {
			ci::app::console() << "tSystem->ReleaseInstance();" << std::endl;
			tSystem->ReleaseInstance();
		}
		catch (Spinnaker::Exception &e) {
			ci::app::console() << "Error: " << e.what() << std::endl;
		}
	
		mConnected = false;
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

	void SpinnakerCapture::draw(){
		//draw the texture
		ci::gl::draw(mCamTexture);
	}
}
//namespace rph