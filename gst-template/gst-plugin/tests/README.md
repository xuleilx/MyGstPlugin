gcc test.c -o test `pkg-config --cflags --libs gstreamer-1.0`

gst-launch-1.0 fakesrc ! myfilter silent=true ! fakesink

gst-launch-1.0 filesrc location=/home/xuleilx/Videos/test.h264 ! h264parse ! decodebin ! videoconvert ! myfilter silent=true ! xvimagesink
