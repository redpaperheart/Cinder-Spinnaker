//Force dedicated graphics card on laptop
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/Log.h"
#include "cinder/params/Params.h"

#include "SpinnakerCapture.h"


using namespace ci;
using namespace ci::app;
using namespace std;

class AnotherSpinnakerApp : public App {
  public:
	  void setup() override;
	  void update() override;
	  void draw() override;

	  void startCapture();
	  void stopCapture();

	  float						mFPS; //app fps
	  bool						mCapturing = false;
	  params::InterfaceGlRef	mParams;
	  SpinnakerCaptureRef mSpinnakerCaptureRef;
	  SpinnakerCapture::CameraOptions cameraOptions;
};

void AnotherSpinnakerApp::setup() {
	mSpinnakerCaptureRef = SpinnakerCapture::create();

	mFPS = getAverageFps();
	mParams = params::InterfaceGl::create("App Params", vec2(250, 340) * app::getWindowContentScale());
	mParams->setOptions("", "refresh=0.5");
	mParams->addParam("FPS", &mFPS);

	mParams->addButton("Start Capture", [&]() { startCapture(); });
	mParams->addButton("Stop Capture", [&]() { stopCapture(); });
	mParams->addParam("Camera Started", &mCapturing, "", true);

	mParams->addParam("Update Surface", &mSpinnakerCaptureRef->mUpdateSurface);
	mParams->addParam("Update Texture", &mSpinnakerCaptureRef->mUpdateTexture);
	mParams->addParam("Camera GetNextImage", &mSpinnakerCaptureRef->mGetNextImage);

	mParams->addSeparator();
	//camera must be stopped then started again to use new options
	mParams->addParam<int>("List Index", &cameraOptions.camIndex, false).group("Cam Options");
	mParams->addParam<float>("Cam FPS", &cameraOptions.fps, false).group("Cam Options");
	mParams->addParam<float>("Res Width", &cameraOptions.res.x, true).group("Cam Options");
	mParams->addParam<float>("Res Height", &cameraOptions.res.y, true).group("Cam Options");
	mParams->addParam<int>("Auto Exposure", &cameraOptions.autoExposureValue, false).group("Cam Options");
	mParams->addParam<int>("Auto Gain", &cameraOptions.autoGainValue, false).group("Cam Options");
	mParams->addParam<int>("Auto White", &cameraOptions.autoWhiteBalanceValue, false).group("Cam Options");
	mParams->addParam<int>("Override Exposure", &cameraOptions.exposureTime, false).group("Cam Options");
	mParams->addParam<float>("Override Gain", &cameraOptions.gain, false).group("Cam Options");

	startCapture();
}

void AnotherSpinnakerApp::update() {
	mFPS = getAverageFps();
	mCapturing = mSpinnakerCaptureRef->isStarted();
	mSpinnakerCaptureRef->update();
}
void AnotherSpinnakerApp::startCapture() {
	//create cam options
	//SpinnakerCapture::CameraOptions opts;
	//opts.camIndex = 0;
	//opts.fps = 55;
	//opts.res = vec2(2048, 1536); //camera res for black fly s (not adjustable on blackfly s)

	////see CameraOptions struct for info on auto values enum
	//opts.autoExposureValue = 0;
	//opts.autoGainValue = 0;
	//opts.autoWhiteBalanceValue = 1;

	////non auto exposure and gain settings, be sure auto values are set to off
	//opts.exposureTime = 15000;
	//opts.gain = 10;

	//setup cam + start thread
	mSpinnakerCaptureRef->setup(cameraOptions);
	mSpinnakerCaptureRef->start();
	

	//mSpinnakerCapture.start();
}
void AnotherSpinnakerApp::stopCapture() {
	mSpinnakerCaptureRef->stop();
}

void AnotherSpinnakerApp::draw() {
	gl::clear(Color(1, 0, 0));
	{
		gl::ScopedMatrices mat;
		gl::scale(vec2(0.5f, 0.5f));
		mSpinnakerCaptureRef->draw();
	}

	mParams->draw();
}

void prepareSettings(App::Settings* settings) {
	settings->setWindowSize(1500, 900);
	settings->setMultiTouchEnabled(false);
	settings->setHighDensityDisplayEnabled(false);
}


CINDER_APP(AnotherSpinnakerApp, RendererGl, prepareSettings)