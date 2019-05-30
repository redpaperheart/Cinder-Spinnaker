#include "SpinnakerCapture.h"

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace ci;
using namespace ci::app;
using namespace std;

SpinnakerCapture::SpinnakerCapture(){
	
}

SpinnakerCapture::~SpinnakerCapture(){
	//clean up references
	/*if (mResultImage!=nullptr) {
		mResultImage->Release();
	}*/
	mShouldQuit = true;
	try {
		mThread->join();
	}
	catch (...) {}
	/*mCam->DeInit();
	mSystem->ReleaseInstance();*/
}

void SpinnakerCapture::setup(int index){
	int mResX = 2048;
	int mResY = 1536;
	mLatestFrame = ci::Surface::create(mResX, mResY, false, ci::SurfaceChannelOrder::RGBA);
	mCamTexture = ci::gl::Texture::create(mResX, mResY);

	mCamIndex = index; //always 0 unless there's more than 1 cam, then pass in
	//camera setup all happens in the thread
	mThread = std::shared_ptr<std::thread>(new std::thread(std::bind(&SpinnakerCapture::captureThreadFn, this)));

}
int SpinnakerCapture::PrintDeviceInfo(INodeMap & nodeMap) {
	int result = 0;

	cout << endl << "*** DEVICE INFORMATION ***" << endl << endl;

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
		//cout << "Error: " << e.what() << endl;
		CI_LOG_I("Error: " << e.what());
		result = -1;
	}

	return result;
}
//TODO setup
bool SpinnakerCapture::isFrameAvailable() {
	return mNewFrame;
}


void SpinnakerCapture::update(){
	//ci::app::console() << "update texture" << std::endl;
	if (mUpdateTexture && isFrameAvailable()) {
		mCamTexture->update(*mLatestFrame);
		mNewFrame = false;
		if(mDebugLogs) ci::app::console() << "new frame ------------------" << std::endl;
	}
	else {
		if(mDebugLogs) ci::app::console() << "waiting on a new camera frame" << std::endl;
	}
}

int SpinnakerCapture::AcquireImages(CameraPtr pCam, INodeMap & nodeMap, INodeMap & nodeMapTLDevice) {
	
	int result = 0;
	try {
		ImagePtr nextImg = pCam->GetNextImage();
		if (!nextImg->IsIncomplete()) {
			//ci::app::console() << "AcquireImages nextImg" << std::endl;
			//convertImage(nextImg);
			//convert the image to proper format
			if (mUpdateSurface) {
				ImagePtr mConvertedImage;
				mConvertedImage = nextImg->Convert(PixelFormat_RGBa8, NEAREST_NEIGHBOR);

				int sizeX = 2048;
				int sizeY = 1536;
				//update the surface ref with the new data
				auto mCamPixelData = mConvertedImage->GetData();
				memcpy(mLatestFrame->getData(), mCamPixelData, sizeX * sizeY * 4);
				mNewFrame = true;
			}
			
			//mCaptureCount++;
		}
		/*else {
			ci::app::console() << "AcquireImages incomplete" << std::endl;
		}*/
		nextImg->Release();
		
	}
	catch (Spinnaker::Exception &e) {
		CI_LOG_I("Error: " << e.what());
		result = -1;
	}

	return result;
}
void SpinnakerCapture::captureThreadFn() {
	ci::ThreadSetup threadSetup; // instantiate this if you're talking to Cinder from a secondary thread
	ci::app::console() << "CAPTURE THREAD STARTED" << std::endl;
	
	//setup the system
	SystemPtr mSystem = Spinnaker::System::GetInstance();
	//get all cameras
	CameraList mCamList = mSystem->GetCameras();

	//TODO make sure there is a camera
	//otherwise bail and try again in a bit

	//setup the camera
	CameraPtr mCam = mCamList.GetByIndex(mCamIndex);
	mCam->Init();

	//reduce buffer stream to 1
	// Retrieve Stream Parameters device nodemap 
	Spinnaker::GenApi::INodeMap & sNodeMap = mCam->GetTLStreamNodeMap();

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

	CIntegerPtr ptrBufferCountMax = sNodeMap.GetNode("StreamBufferCountMax");
	if (!IsAvailable(ptrBufferCountMax) || !IsWritable(ptrBufferCountMax))
	{
		CI_LOG_I("Unable to set Buffer Count Max (Integer node retrieval). Aborting...");
	}

	// Display Buffer Info
	CI_LOG_I("Default Buffer Handling Mode: " << ptrHandlingModeEntry->GetDisplayName());
	CI_LOG_I("Default Buffer Count: " << ptrBufferCount->GetValue());
	CI_LOG_I("Maximum Buffer Count: " << ptrBufferCount->GetMax());

	//change handling mode to newest first
	//ptrHandlingModeEntry = ptrHandlingMode->GetEntryByName("NewestFirst");
	//ptrHandlingMode->SetIntValue(ptrHandlingModeEntry->GetValue());
	//CI_LOG_I("Change Buffer Handling Mode?: " << ptrHandlingModeEntry->GetDisplayName());

	ptrBufferCount->SetValue(1);

	//cout << "Buffer count now set to: " << ptrBufferCount->GetValue() << endl;
	CI_LOG_I("Buffer count now set to: " << ptrBufferCount->GetValue());

	mCam->ExposureAuto.SetValue(Spinnaker::ExposureAutoEnums::ExposureAuto_Off);
	mCam->GainAuto.SetValue(Spinnaker::GainAutoEnums::GainAuto_Off);
	mCam->AcquisitionMode.SetValue(AcquisitionMode_Continuous);

	mCam->AcquisitionFrameRateEnable.SetValue(true);
	mCam->AcquisitionFrameRate.SetValue(fps);

	mCam->BeginAcquisition();
	//lastFrame = getElapsedSeconds();

	while ((!mShouldQuit)) {
		//ci::app::console() << "while-- " << std::endl;
		if (mGetNextImage) {
			try {
				INodeMap & nodeMapTLDevice = mCam->GetTLDeviceNodeMap();
				INodeMap & nodeMap = mCam->GetNodeMap();
				AcquireImages(mCam, nodeMap, nodeMapTLDevice);
			}
			catch (Spinnaker::Exception &e) {
				ci::app::console() << "Error: " << e.what() << std::endl;
			}
		}
		
	}
	mCam->DeInit();
	mSystem->ReleaseInstance();
	ci::app::console() << "CAPTURE THREAD STOPPED" << std::endl;
}

void SpinnakerCapture::draw(){
	//draw the texture
	ci::gl::draw(mCamTexture);
}


//void SpinnakerCapture::saveFramesFromSurfaceRefVector(){
//	// Create a unique filename
//	/*ostringstream fodlerName;
//	fodlerName << to_string(mFolder);*/
//	
//	for(int i = 0 ; i < mTotalFramesToSave; i++){
//		ostringstream filename;
//		filename <<"SpinnakerCapture-";
//		filename << i << ".jpg";
//		mSavedSurfaceRefVector[i]->Save(filename.str().c_str());
//	}
//	mSavedSurfaceRefVector.clear();
//	mSavedSurfaceRefVector.resize(mTotalFramesToSave);
//	mImageCnt = 0;
//	mFolder++;
//	bStartRec = false;
//}

//void SpinnakerCapture::loadVector(){
//	mSavedSurfaceRefVector.clear();
//	for(int i = 0 ; i < mTotalFramesToSave; i++){
//		mSavedSurfaceRefVector[i] = mConvertedImage;
//		CI_LOG_I( "saving frame: " << mImageCnt );
//	}
//	
//}

//int SpinnakerCapture::RunSingleCamera(CameraPtr pCam) {
//	int result = 0;
//	//turn off auto exposure
//
//
//	try {
//		INodeMap & nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
//		INodeMap & nodeMap = pCam->GetNodeMap();
//		result = result | AcquireImages(pCam, nodeMap, nodeMapTLDevice);
//	}
//	catch (Spinnaker::Exception &e) {
//
//		CI_LOG_I("Error: " << e.what());
//		result = -1;
//	}
//
//	return result;
//}
//void SpinnakerCapture::convertImage(ImagePtr img) {
	//take an image, convert it to the format we want, then turn into a surface
	//mConvertedImage = img->Convert(PixelFormat_RGBa8, NEAREST_NEIGHBOR);

	//auto mCamPixelData = mConvertedImage->GetData();
	//memcpy(mCamSurfaceRef->getData(), mCamPixelData, mResX * mResY * 4);
	//mCamTexture->update(*mCamSurfaceRef);
//}

//get all the images until the result image is incomplete
		//ImagePtr nextImg;// = pCam->GetNextImage();
		//ImagePtr lastImg = nullptr;
		//bool emptyQ = false;

		//

		//while (emptyQ==false) {
		//	CI_LOG_I("while");
		//	nextImg = pCam->GetNextImage();
		//	if (nextImg->IsIncomplete()) {
		//		emptyQ = true;
		//		CI_LOG_I("incomplete");
		//		nextImg->Release();
		//	}
		//	else {
		//		//release last image cause we have a new one
		//		/*if (lastImg != nullptr) {
		//			lastImg->Release();
		//		}*/
		//		CI_LOG_I("lastimg = next");
		//		lastImg = nextImg;
		//	}
		//}
		//if (lastImg != nullptr) {
		//	CI_LOG_I("convertImage lastimg");
		//	convertImage(lastImg);
		//	lastImg->Release();
		//}

		//if (!nextImg->IsIncomplete()) {

		//}
		//ImagePtr lastImg = nullptr;
		//while (nextImg != nullptr !nextImg->IsIncomplete()) {
		//	//get the next one
		//	latestImage = nextImg;
		//	nextImg = pCam->GetNextImage();
		//}
		//convertImage(latestImage);
		//latestImage->Release();
		//nextImg->Release();
		/*
		mResultImage = pCam->GetNextImage();
		if (mResultImage->IsIncomplete()) {
			// Retreive and print the image status description
			CI_LOG_I("Image incomplete: " << Image::GetImageStatusDescription(mResultImage->GetImageStatus()));
		}
		else {
			mConvertedImage = mResultImage->Convert(PixelFormat_RGBa8, NEAREST_NEIGHBOR);

			auto mCamPixelData = mConvertedImage->GetData();
			memcpy(mCamSurfaceRef->getData(), mCamPixelData, mResX * mResY * 4);
			mCamTexture->update(*mCamSurfaceRef);

			//if( mImageCnt < mTotalFramesToSave && bStartRec){
			//	mSavedSurfaceRefVector[mImageCnt] = mConvertedImage;
			//	mImageCnt++;
			//	CI_LOG_I( "saving frame: " << mImageCnt );
			//}
		}
		mResultImage->Release();
		*/

