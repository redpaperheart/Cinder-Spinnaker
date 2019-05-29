//Force dedicated graphics card on laptop
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class SimpleSpinnakerApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void SimpleSpinnakerApp::setup()
{
}

void SimpleSpinnakerApp::mouseDown( MouseEvent event )
{
}

void SimpleSpinnakerApp::update()
{
}

void SimpleSpinnakerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( SimpleSpinnakerApp, RendererGl )
