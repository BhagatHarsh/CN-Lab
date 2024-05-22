import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GLib

# Initialize GStreamer
Gst.init(None)

# Define the pipeline
# pipeline_string = (
#     "videotestsrc ! video/x-raw, format=BGR ! autovideoconvert ! ximagesink"
# )

pipeline_string = (
    "filesrc location=/mnt/c/Users/habha/Desktop/codingSpace/SchoolCodes/year3/CN/LAB/pystreaming/chrome.mp4 ! decodebin ! videoconvert ! autovideosink"
)

# Create the pipeline
pipeline = Gst.parse_launch(pipeline_string)

# Start playing
pipeline.set_state(Gst.State.PLAYING)

# Wait until error or EOS
bus = pipeline.get_bus()
msg = bus.timed_pop_filtered(
    Gst.CLOCK_TIME_NONE, Gst.MessageType.ERROR | Gst.MessageType.EOS
)

# Free resources
pipeline.set_state(Gst.State.NULL)
