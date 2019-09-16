#pragma once
#include "cinder/app/App.h"
//#include "mason/Config.h"


namespace rph {
	using SpinnakerSettingsRef = std::shared_ptr<class SpinnakerSettings>;

	class SpinnakerSettings
	{
	public:
		static SpinnakerSettingsRef create() {
			return std::make_shared<SpinnakerSettings>();
		}
		static SpinnakerSettingsRef create(
			std::string _serial,
			int _fps = 55,
			glm::ivec2 _resolution = glm::ivec2(2048, 1536),
			int _autoExposureMode = 0,
			int _autoGainMode = 0,
			int _autoWhiteBalanceMode = 1,
			int _exposureTime = 15000,
			float _gain = 2,
			int _deviceIndex = -1
		)
		{
			return std::make_shared<SpinnakerSettings>(_serial, _fps, _resolution, _autoExposureMode, _autoGainMode, _autoWhiteBalanceMode, _exposureTime, _gain, _deviceIndex);
		}
		
		SpinnakerSettings() {};
		SpinnakerSettings(
			std::string _serial,
			int _fps,
			glm::ivec2 _resolution,
			int _autoExposureMode,
			int _autoGainMode,
			int _autoWhiteBalanceMode,
			int _exposureTime,
			float _gain,
			int _deviceIndex
			)
			: mDeviceSerial(_serial), mFPS(_fps), mResolution(_resolution),
			mAutoExposureMode(_autoExposureMode), mAutoGainMode(_autoGainMode), mAutoWhiteMode(_autoWhiteBalanceMode),
			mExposureTime(_exposureTime), mGain(_gain)
		{

		}
		~SpinnakerSettings() {};

		int						mDeviceIndex = -1; //index on the system for the device, if it remains -1 that means this setting has no device
		std::string				mDeviceSerial = "none";

		int mFPS = 55;
		glm::ivec2 mResolution = glm::vec2(2048, 1536);
		//for auto values
			// 0 == OFF
			// 1 == Once (sets value once based on current lighting then sets itself to off)
			// 2 == Continuous (constantly updates)
		const char* AUTO_MODES = "OFF\0ONCE\0CONTINUOUS";
		int mAutoExposureMode = 0;
		int mAutoGainMode = 0;
		int mAutoWhiteMode = 1;

		//non auto exposure and gain settings, be sure auto values are set to off
		int mExposureTime = 15000;
		float mGain = 2;
		 
	private:

	};
}