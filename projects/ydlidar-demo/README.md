download and build ydlidar sdk from https://github.com/YDLIDAR/YDLidar-SDK
```
cmake .. -DBUILD_SHARED_LIBS=ON -DBUILD_EXAMPLES=OFF -DCMAKE_INSTALL_PREFIX=/usr/local

```

# video streaming
download and build libcamera  
from https://git.libcamera.org/libcamera/libcamera.git

stream camera from raspberrypi
```
export GST_PLUGIN_PATH=/usr/local/lib/gstreamer-1.0; export LD_LIBRARY_PATH=/usr/local/lib; export PATH=/usr/local/bin:$PATH
args=(
    libcamerasrc camera-name='/base/soc/i2c0mux/i2c@1/imx219@10' !
    video/x-raw,width=640,height=480,framerate=30/1 !
    videoconvert !
    v4l2h264enc !
    'video/x-h264,level=(string)3,profile=baseline' !
    rtph264pay !
    queue !
    udpsink host=192.168.X.X port=8082
)
gst-launch-1.0 ${args}
```

receive video
```
args=(
    udpsrc port=8082 caps="application/x-rtp,media=(string)video" !
    rtpjitterbuffer !
    rtph264depay !
    avdec_h264 !
    videoconvert !
    waylandsink
)
gst-launch-1.0 ${args}
```
