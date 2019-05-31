#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "SpinnakerCapture.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SimpleSpinnakerApp : public App {
  public:
	void setup() override;
	void update() override;
	void draw() override;

	rph::SpinnakerCaptureRef mSpinnakerCaptureRef;
	rph::SpinnakerCapture::CameraOptions cameraOptions;
};

void SimpleSpinnakerApp::setup()
{
	//create a capture'er
	mSpinnakerCaptureRef = rph::SpinnakerCapture::create();
	//then start it
	mSpinnakerCaptureRef->setup(cameraOptions);
	mSpinnakerCaptureRef->start();
}

void SimpleSpinnakerApp::update()
{
	mSpinnakerCaptureRef->update();
}

void SimpleSpinnakerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
	{
		gl::ScopedMatrices mat;
		//render half size
		gl::scale(vec2(0.5f, 0.5f));
		mSpinnakerCaptureRef->draw();
	}
}

CINDER_APP( SimpleSpinnakerApp, RendererGl )
