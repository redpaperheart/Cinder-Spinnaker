Cinder-Spinnaker
===================
Cinder block for integrating [Flir / Point Grey Research] Spinnaker SDK

##### Adding this block to Cinder
This block was built with version 0.9.1, so 0.9.0 and up should work. This block is also only available on Windows since the Spinnaker SDK is Windows only. The current Spinnaker SDK also only contains .dll files for x64 + toolset v140.

* First get a Camera that uses the Spinnaker SDK, this block was built with a [Flir Blackfly S](https://www.flir.com/products/blackfly-s-usb3/?model=BFS-U3-31S4C-C) 3.2MP Color
* Second download the [Spinnaker SDK](https://www.flir.com/products/spinnaker-sdk/)
  - This block was built with SDK v 1.20.0.15 but should work with newer versions as well
  - This block is also configured to work with the default Spinnaker SDK install location ```C:\Program Files\Point Grey Research\Spinnaker```
  - If you install the SDK at a different path you'll want to update either the cinderblock.xml paths or modify the paths in VS after you create a project 
* Add this block to your Cinder/blocks folder
* Use TinderBox and create a new Cinder project

There are 2 different sample projects included with the block. SimpleSpinnaker is a very simple barebones example and AnotherSpinnaker shows starting/stopping the capture as well as changing camera options (framerate, auto exposure)

##### Tips and Gotchas
- If the SDK is installed in 
- This was only tested with a Blackfly S which is color, has a high frame rate, and high resolution that is not configurable. Setting the frame rate to unsupported frame rates or resolutions for your camera may cause the app to crash
- Most of the Spinnaker capturing happens in another thread and then a Surface is updated with memCopy back to the main thread. If that thread gets slowed down you may start to see the buffer get backed up and the video feed will start to lag. I currently don't have a solution for this except to just not slow down that thread. You can also try slowing down your camera frame rate.

##### TODOS
- test framerate and resolution values before attempting to set them
- investigate image tearing: maybe caused from the thread memCopy to surface
- adding a capture manager class to reconnect cameras when disconnected + connected
- add functionality to query the cameras current exposure and gain levels
- fix the gross filter paths for the Spinnaker SDK directories, adds a filter folder for each dir ```C:\Program Files\Point Grey Research\Spinnaker```

License
-------
Copyright (c) 2019, Red Paper Heart, All rights reserved. To contact Red Paper Heart, email hello@redpaperheart.com or tweet @redpaperhearts

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
