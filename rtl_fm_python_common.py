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

from __future__ import print_function
import ctypes, sys
from time import sleep
from threading import Thread

printstderr = lambda x : print(x,file=sys.stderr)


fm  = ctypes.CDLL('./rtl_fm_python.so')
get_s_level 	= fm.lib_get_s_level
get_frequency 	= fm.lib_get_frequency
set_demod_fm 	= fm.lib_set_demod_fm
set_demod_wbfm 	= fm.lib_set_demod_wbfm
set_demod_am 	= fm.lib_set_demod_am
set_demod_lsb 	= fm.lib_set_demod_lsb
set_demod_usb 	= fm.lib_set_demod_usb
set_demod_raw 	= fm.lib_set_demod_raw
set_frequency 	= lambda f 	: fm.lib_set_frequency(ctypes.c_uint32(f))
set_squelch 	= lambda l 	: fm.lib_set_squelch_level(ctypes.c_int(l))
get_demod	= lambda 	: chr(fm.lib_get_demod_mode())
str_to_freq	= fm.lib_frequency_convert

def process_args(l):
	c=len(l)+1
	argc=ctypes.c_int(c)
	argv_var=ctypes.c_char_p *c
	argu = ['rtl_fm'] + l
	argv=argv_var(*argu)
	return (argc,argv)

def mag(value, e, suffix):
	t = float('1e%s' % e)
	i = int(value / float(t))
	r = int(value % float(t))
	r = str(r).rjust(e,'0')
	s = "%s.%s" % (i,r)
	while (s[-1]=="0" or s[-1]=="."):
		s=s[:-1]
	return s + suffix

def freq_to_str(f):
	if f >= 1000000000:
		return mag(f,9,'G')
	if f >= 1000000:
		return mag(f,6,'M')
	if f >= 1000:
		return mag(f,3,'K')
	return str(f)		

set_freq_human 	= lambda f	: set_frequency(str_to_freq(f))
get_freq_human	= lambda 	: freq_to_str(get_frequency())

def set_demod(c):
	if c=='w' : set_demod_wbfm()
	if c=='f' : set_demod_fm()
	if c=='a' : set_demod_am()
	if c=='l' : set_demod_lsb()
	if c=='u' : set_demod_usb()
	if c=='r' : set_demod_raw()

def get_gains():
	c=fm.lib_get_tuner_gains_count()
	b=(ctypes.c_int * c)()
	fm.lib_get_tuner_gains(b)
	return list(b)

def set_gain(g):
	fm.lib_set_real_gain(g)

def get_gain():
	return fm.lib_get_gain()

def get_auto_gain():
	return get_gain()==-100

def set_gain_human(g):
	fm.lib_set_gain(g)

def set_auto_gain():
	fm.lib_set_auto_gain()



def rtl_fm(args=["--help"]):
	fm.main(*process_args(args))

def rtl_fm_setup_and_go(args):
	fm.lib_init_first()
	fm.lib_process_args(*process_args(args))
	fm.lib_input_open()
	fm.lib_output_open()
	fm.lib_go()

def rtl_fm_loop():
	fm.lib_loop()

def rtl_fm_finish():
	fm.lib_stop()
	fm.lib_output_close()


def rtl_fm_wrapped(args=["--help"]):
	rtl_fm_setup_and_go(args)
	try:
		while True:
			sleep(0.01)
	except KeyboardInterrupt:
		rtl_fm_finish()










