import dronecan
import can
import logging
import sys

logging.basicConfig(stream=sys.stdout, level=logging.DEBUG)
bus = can.interface.Bus(channel='/dev/ttyACM0', interface='slcan', bitrate=500000)


node = dronecan.make_node("/dev/ttyACM0", node_id=1, bitrate=500000)