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

#define CAM_RES_X 2048
#define CAM_RES_Y 1536

using namespace ci;
using namespace ci::app;
using namespace std;

class AnotherSpinnakerApp : public App {
  public:
	  void setup() override;
	  void update() override;
	  void draw() override;

	  params::InterfaceGlRef	mParams;
	  float					mFPS;

	  SpinnakerCapture mSpinnakerCapture;
};

void AnotherSpinnakerApp::setup() {
	mFPS = getAverageFps();
	mParams = params::InterfaceGl::create("App Params", vec2(250, 340) * app::getWindowContentScale());
	mParams->setOptions("", "refresh=0.5");
	mParams->addParam("FPS", &mFPS);
	mParams->addParam("Camera Targ FPS", &mSpinnakerCapture.fps).min(1).max(55);
	mParams->addParam("Limit FPS", &mSpinnakerCapture.limitFPS);
	mParams->addParam("Debug Logs", &mSpinnakerCapture.mDebugLogs);
	mParams->addParam("Update Surface", &mSpinnakerCapture.mUpdateSurface);
	mParams->addParam("Update Texture", &mSpinnakerCapture.mUpdateTexture);
	mParams->addParam("Camera GetNextImage", &mSpinnakerCapture.mGetNextImage);


	mSpinnakerCapture.setup();
}

void AnotherSpinnakerApp::update() {
	mFPS = getAverageFps();
	mSpinnakerCapture.update();
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