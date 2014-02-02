#!/usr/bin/python -i

# rtl-sdr, turns your Realtek RTL2832 based DVB dongle into a SDR receiver
# Copyright (C) 2012 by Steve Markgraf <steve@steve-m.de>
# Copyright (C) 2012 by Hoernchen <la@tfc-server.de>
# Copyright (C) 2012 by Kyle Keen <keenerd@gmail.com>
# Copyright (C) 2013 by Elias Oenal <EliasOenal@gmail.com>
# Copyright (C) 2014 by Thomas Winningham <winningham@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


from rtl_fm_python_common import *

running = True
queue=[]

def rtl_fm_thread(args):
	rtl_fm_setup_and_go(args)
	while running==True:
		if len(queue) > 0:
			queue.pop()()
		sleep(1)
	rtl_fm_finish()

def make_rtl_fm_thread(args=sys.argv[1:],block=False):
	t = Thread(target=rtl_fm_thread,args=(args,))
	t.daemon=True
	t.start()
	if block:
		try:
			while True:
				sleep(1)
		except KeyboardInterrupt:
			running = False

def stop_thread():
	running = False


if __name__ == "__main__":
	make_rtl_fm_thread(block=False)

