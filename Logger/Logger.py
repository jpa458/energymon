#!/usr/bin/python

## JeeLinkLogger.py
## 2013-09-16 <jpa458 at gmail dot com> http://opensource.org/licenses/mit-license.php
## Creates daily rolling log files of incoming data from a JeeLink module
## Only writes valid data lines

## Code inspired by http://www.sensorbay.com/2013/01/project-1-home-energy-monitor-with.html 

import serial, datetime, os, sys

serial_port = serial.Serial('/dev/ttyUSB0', 57600)
log_path = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else '.'
if not os.path.exists(log_path): os.makedirs(log_path)
data_file = None; last = None

while 1:
	data = serial_port.readline()
	now = datetime.datetime.now()

	if last == None or now.day > last.day:
		if data_file is not None:
			data_file.flush(); data_file.close()
		file_name = os.path.join(log_path, now.strftime('%Y%m%d_energy.csv'))
		data_file = open(file_name, 'a', 1024)
	
	if data.startswith('OK'):
		data_file.write(now.strftime('%H:%M:%S ') + data)
		data_file.flush()
	last=now
