# Lab 9: Real-Time Multimedia Streaming
# A simple RTSP multimedia server using GStreamer library
# https://gstreamer.freedesktop.org/documentation/application-development/introduction/gstreamer.html?gi-language=python

import gi

# Require GStreamer Version 1
gi.require_version('Gst', '1.0')
gi.require_version('GstRtspServer', '1.0')

from gi.repository import Gst, GLib, GstRtspServer

# Initialize GStreamer
Gst.init(None)


# Define the main function to create and run the RTSP server
def main():
    # Create the RTSP server
    server = GstRtspServer.RTSPServer.new()
    server.set_address("127.0.0.1")  # server address
    server.set_service("8554")  # server port

    # Get the mount points for the server
    mounts = server.get_mount_points()

    # Create a new media factory for the RTSP server
    # Exercise: 1. Run and play the sample movie on your machine
    #           2. Modify the pipeline to stream a video file
    #           3. Test the A/V streaming on localhost and then access the video from some other computer
    #           Challenge: Implement multicasting of video files
    
    
    factory = GstRtspServer.RTSPMediaFactory.new()
    pipeline_string = "filesrc location=/mnt/c/Users/habha/Desktop/codingSpace/SchoolCodes/year3/CN/LAB/pystreaming/chrome.mp4 ! videotestsrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96"
    # pipeline_string = "videotestsrc ! videoconvert ! x264enc ! rtph264pay name=pay0 pt=96"
    #pipeline_string += " ( audiotestsrc ! audioconvert ! audioresample ! opusenc ! rtpopuspay name=pay1 pt=97 )"
    factory.set_launch(pipeline_string)

    # Attach the factory to the mount points
    mounts.add_factory("/test", factory)

    # Start the RTSP server
    server.attach()

    # Start the main loop
    main_loop = GLib.MainLoop()
    main_loop.run()

if __name__ == '__main__':
    main()
