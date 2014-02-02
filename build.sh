gcc -I /usr/include/libusb-1.0 -I ./convenience/ -I ./getopt -shared -Wl,-soname,rtl_fm_python -o rtl_fm_python.so rtl_fm_python.c convenience/convenience.c getopt/getopt.c -lrtlsdr
