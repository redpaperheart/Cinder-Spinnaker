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
	  SpinnakerCapture mSpinnakerCapture;
};

void AnotherSpinnakerApp::setup() {
	mFPS = getAverageFps();
	mParams = params::InterfaceGl::create("App Params", vec2(250, 340) * app::getWindowContentScale());
	mParams->setOptions("", "refresh=0.5");
	mParams->addParam("FPS", &mFPS);

	mParams->addButton("Start Capture", [&]() { startCapture(); });
	mParams->addButton("Stop Capture", [&]() { stopCapture(); });
	mParams->addParam("Camera Started", &mCapturing, "", true);

	mParams->addParam("Update Surface", &mSpinnakerCapture.mUpdateSurface);
	mParams->addParam("Update Texture", &mSpinnakerCapture.mUpdateTexture);
	mParams->addParam("Camera GetNextImage", &mSpinnakerCapture.mGetNextImage);

	//create cam options
	SpinnakerCapture::CameraOptions opts;
	opts.camIndex = 0;
	opts.fps = 55;
	opts.res = vec2(2048, 1536); //camera res for black fly s (not configurable)
	//setup cam + start thread
	mSpinnakerCapture.setup(opts);
	mSpinnakerCapture.start();
}

void AnotherSpinnakerApp::update() {
	mFPS = getAverageFps();
	mCapturing = mSpinnakerCapture.isStarted();
	mSpinnakerCapture.update();
}
void AnotherSpinnakerApp::startCapture() {
	mSpinnakerCapture.start();
}
void AnotherSpinnakerApp::stopCapture() {
	mSpinnakerCapture.stop();
}

void AnotherSpinnakerApp::draw() {
	gl::clear(Color(1, 0, 0));
	{
		gl::ScopedMatrices mat;
		gl::scale(vec2(0.5f, 0.5f));
		mSpinnakerCapture.draw();
	}

	mParams->draw();
}

void prepareSettings(App::Settings* settings) {
	settings->setWindowSize(1500, 900);
	settings->setMultiTouchEnabled(false);
}


CINDER_APP(AnotherSpinnakerApp, RendererGl, prepareSettings)