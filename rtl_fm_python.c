#include "rtl_fm.c"
#include "getopt/getopt.h"
#include "rtl-sdr.h"
#include "convenience/convenience.h"

#ifndef _WIN32
        struct sigaction sigact;
#endif
        int r, opt;
        int dev_given = 0;
        int custom_ppm = 0;



void  lib_loop(){
	while (!do_exit) {
		usleep(100000);
	}
}


void  lib_init_first()
{

	dongle_init(&dongle);
	demod_init(&demod);
	output_init(&output);
	controller_init(&controller);

}

void no_exit_usage(void)
{
	fprintf(stderr,
		"rtl_fm, a simple narrow band FM demodulator for RTL2832 based DVB-T receivers\n\n"
		"Use:\trtl_fm -f freq [-options] [filename]\n"
		"\t-f frequency_to_tune_to [Hz]\n"
		"\t    use multiple -f for scanning (requires squelch)\n"
		"\t    ranges supported, -f 118M:137M:25k\n"
		"\t[-M modulation (default: fm)]\n"
		"\t    fm, wbfm, raw, am, usb, lsb\n"
		"\t    wbfm == -M fm -s 170k -o 4 -A fast -r 32k -l 0 -E deemp\n"
		"\t    raw mode outputs 2x16 bit IQ pairs\n"
		"\t[-s sample_rate (default: 24k)]\n"
		"\t[-d device_index (default: 0)]\n"
		"\t[-g tuner_gain (default: automatic)]\n"
		"\t[-l squelch_level (default: 0/off)]\n"
		//"\t    for fm squelch is inverted\n"
		//"\t[-o oversampling (default: 1, 4 recommended)]\n"
		"\t[-p ppm_error (default: 0)]\n"
		"\t[-E enable_option (default: none)]\n"
		"\t    use multiple -E to enable multiple options\n"
		"\t    edge:   enable lower edge tuning\n"
		"\t    dc:     enable dc blocking filter\n"
		"\t    deemp:  enable de-emphasis filter\n"
		"\t    direct: enable direct sampling\n"
		"\t    offset: enable offset tuning\n"
		"\tfilename ('-' means stdout)\n"
		"\t    omitting the filename also uses stdout\n\n"
		"Experimental options:\n"
		"\t[-r resample_rate (default: none / same as -s)]\n"
		"\t[-t squelch_delay (default: 10)]\n"
		"\t    +values will mute/scan, -values will exit\n"
		"\t[-F fir_size (default: off)]\n"
		"\t    enables low-leakage downsample filter\n"
		"\t    size can be 0 or 9.  0 has bad roll off\n"
		"\t[-A std/fast/lut choose atan math (default: std)]\n"
		//"\t[-C clip_path (default: off)\n"
		//"\t (create time stamped raw clips, requires squelch)\n"
		//"\t (path must have '\%s' and will expand to date_time_freq)\n"
		//"\t[-H hop_fifo (default: off)\n"
		//"\t (fifo will contain the active frequency)\n"
		"\n"
		"Produces signed 16 bit ints, use Sox or aplay to hear them.\n"
		"\trtl_fm ... | play -t raw -r 24k -es -b 16 -c 1 -V1 -\n"
		"\t           | aplay -r 24k -f S16_LE -t raw -c 1\n"
		"\t  -M wbfm  | play -r 32k ... \n"
		"\t  -s 22050 | multimon -t raw /dev/stdin\n\n");
}

void no_exit_sanity_checks(void)
{
        if (controller.freq_len == 0) {
                fprintf(stderr, "Please specify a frequency.\n");
        }

        if (controller.freq_len >= FREQUENCIES_LIMIT) {
                fprintf(stderr, "Too many channels, maximum %i.\n", FREQUENCIES_LIMIT);
        }

        if (controller.freq_len > 1 && demod.squelch_level == 0) {
                fprintf(stderr, "Please specify a squelch level.  Required for scanning multiple frequencies.\n");
        }

}



void  lib_process_args(int argc, char **argv)
{

	
	while ((opt = getopt(argc, argv, "d:f:g:s:b:l:o:t:r:p:E:F:A:M:h")) != -1) {
		switch (opt) {
		case 'd':
			dongle.dev_index = verbose_device_search(optarg);
			dev_given = 1;
			break;
		case 'f':
			if (controller.freq_len >= FREQUENCIES_LIMIT) {
				break;}
			if (strchr(optarg, ':'))
				{frequency_range(&controller, optarg);}
			else
			{
				controller.freqs[controller.freq_len] = (uint32_t)atofs(optarg);
				controller.freq_len++;
			}
			break;
		case 'g':
			dongle.gain = (int)(atof(optarg) * 10);
			break;
		case 'l':
			demod.squelch_level = (int)atof(optarg);
			break;
		case 's':
			demod.rate_in = (uint32_t)atofs(optarg);
			demod.rate_out = (uint32_t)atofs(optarg);
			break;
		case 'r':
			output.rate = (int)atofs(optarg);
			demod.rate_out2 = (int)atofs(optarg);
			break;
		case 'o':
			fprintf(stderr, "Warning: -o is very buggy\n");
			demod.post_downsample = (int)atof(optarg);
			if (demod.post_downsample < 1 || demod.post_downsample > MAXIMUM_OVERSAMPLE) {
				fprintf(stderr, "Oversample must be between 1 and %i\n", MAXIMUM_OVERSAMPLE);}
			break;
		case 't':
			demod.conseq_squelch = (int)atof(optarg);
			if (demod.conseq_squelch < 0) {
				demod.conseq_squelch = -demod.conseq_squelch;
				demod.terminate_on_squelch = 1;
			}
			break;
		case 'p':
			dongle.ppm_error = atoi(optarg);
			custom_ppm = 1;
			break;
		case 'E':
			if (strcmp("edge",  optarg) == 0) {
				controller.edge = 1;}
			if (strcmp("dc", optarg) == 0) {
				demod.dc_block = 1;}
			if (strcmp("deemp",  optarg) == 0) {
				demod.deemph = 1;}
			if (strcmp("direct",  optarg) == 0) {
				dongle.direct_sampling = 1;}
			if (strcmp("offset",  optarg) == 0) {
				dongle.offset_tuning = 1;}
			break;
		case 'F':
			demod.downsample_passes = 1;  /* truthy placeholder */
			demod.comp_fir_size = atoi(optarg);
			break;
		case 'A':
			if (strcmp("std",  optarg) == 0) {
				demod.custom_atan = 0;}
			if (strcmp("fast", optarg) == 0) {
				demod.custom_atan = 1;}
			if (strcmp("lut",  optarg) == 0) {
				atan_lut_init();
				demod.custom_atan = 2;}
			break;
		case 'M':
			if (strcmp("fm",  optarg) == 0) {
				demod.mode_demod = &fm_demod;}
			if (strcmp("raw",  optarg) == 0) {
				demod.mode_demod = &raw_demod;}
			if (strcmp("am",  optarg) == 0) {
				demod.mode_demod = &am_demod;}
			if (strcmp("usb", optarg) == 0) {
				demod.mode_demod = &usb_demod;}
			if (strcmp("lsb", optarg) == 0) {
				demod.mode_demod = &lsb_demod;}
			if (strcmp("wbfm",  optarg) == 0) {
				controller.wb_mode = 1;
				demod.mode_demod = &fm_demod;
				demod.rate_in = 170000;
				demod.rate_out = 170000;
				demod.rate_out2 = 32000;
				demod.custom_atan = 1;
				//demod.post_downsample = 4;
				demod.deemph = 1;
				demod.squelch_level = 0;}
			break;
		case 'h':
		default:
			no_exit_usage();
			break;
		}
	}


	/* quadruple sample_rate to limit to Δθ to ±π/2 */
	demod.rate_in *= demod.post_downsample;

	if (!output.rate) {
		output.rate = demod.rate_out;}

	no_exit_sanity_checks();

	if (controller.freq_len > 1) {
		demod.terminate_on_squelch = 0;}

	if (argc <= optind) {
		output.filename = "-";
	} else {
		output.filename = argv[optind];
	}

	ACTUAL_BUF_LENGTH = lcm_post[demod.post_downsample] * DEFAULT_BUF_LENGTH;

	if (!dev_given) {
		dongle.dev_index = verbose_device_search("0");
	}

	if (dongle.dev_index < 0) {
                fprintf(stderr, "Device index less than zero.\n");
	}


}

void  lib_output_open()
{
	if (strcmp(output.filename, "-") == 0) { /* Write samples to stdout */
		output.file = stdout;
#ifdef _WIN32
		_setmode(_fileno(output.file), _O_BINARY);
#endif
	} else {
		output.file = fopen(output.filename, "wb");
		if (!output.file) {
			fprintf(stderr, "Failed to open %s\n", output.filename);
		}
	}
}

void  lib_input_open()
{
	r = rtlsdr_open(&dongle.dev, (uint32_t)dongle.dev_index);
	if (r < 0) {
		fprintf(stderr, "Failed to open rtlsdr device #%d.\n", dongle.dev_index);
	}

}

void  lib_register_signal_handler(){
	#ifndef _WIN32
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	sigaction(SIGPIPE, &sigact, NULL);
#else
	SetConsoleCtrlHandler( (PHANDLER_ROUTINE) sighandler, TRUE );
#endif

}

void lib_go()
{
	if (demod.deemph) {
		demod.deemph_a = (int)round(1.0/((1.0-exp(-1.0/(demod.rate_out * 75e-6)))));
	}
	if (dongle.gain == AUTO_GAIN) {
		verbose_auto_gain(dongle.dev);
	} else {
		dongle.gain = nearest_gain(dongle.dev, dongle.gain);
		verbose_gain_set(dongle.dev, dongle.gain);
	}
	verbose_ppm_set(dongle.dev, dongle.ppm_error);
	verbose_reset_buffer(dongle.dev);
	pthread_create(&controller.thread, NULL, controller_thread_fn, (void *)(&controller));
	usleep(100000);
	pthread_create(&output.thread, NULL, output_thread_fn, (void *)(&output));
	pthread_create(&demod.thread, NULL, demod_thread_fn, (void *)(&demod));
	pthread_create(&dongle.thread, NULL, dongle_thread_fn, (void *)(&dongle));

}

void  lib_print_frequency(){
	fprintf(stderr,"Frequency: %i.\n",controller.freqs[controller.freq_now]);
}

uint32_t lib_get_frequency(){
	return controller.freqs[controller.freq_now];
}

int lib_get_s_level(){
	int s = rms(demod.lowpassed, demod.lp_len, 1);
	if (s > 0) {
		return s;
	} else {
		return 0;
	} 
}

void  lib_set_frequency(uint32_t new_frequency){
	optimal_settings(new_frequency, demod.rate_in);
	controller.freqs[controller.freq_now] = new_frequency;
	rtlsdr_set_center_freq(dongle.dev, dongle.freq);
}

void  lib_set_squelch_level(int l) {
	demod.squelch_level = l;
}

void  util_set_wbfm_options(){
	controller.wb_mode = 1;
	demod.custom_atan = 1;
	demod.deemph = 1;
	demod.squelch_level = 0;
}

void  util_set_other_demod_options(){
	controller.wb_mode = 0;
	demod.custom_atan = 0;
	demod.deemph = 0;
	demod.squelch_level = 0;
}

void  lib_set_demod_wbfm(){
	util_set_wbfm_options();
	demod.mode_demod = &fm_demod;
}

void  lib_set_demod_fm(){
	util_set_other_demod_options();
	demod.mode_demod = &fm_demod;
}

void  lib_set_demod_am(){
	util_set_other_demod_options();
	demod.mode_demod = &am_demod;
}

void  lib_set_demod_lsb(){
	util_set_other_demod_options();
	demod.mode_demod = &lsb_demod;
}

void  lib_set_demod_usb(){
	util_set_other_demod_options();
	demod.mode_demod = &usb_demod;
}

void  lib_set_demod_raw(){
	util_set_other_demod_options();
	demod.mode_demod = &raw_demod;
}


char lib_get_demod_mode(){
	if (demod.mode_demod==&fm_demod){
		if (controller.wb_mode==1){
			return 'w';
		} else {
			return 'f';
		}
	}
	if (demod.mode_demod==&am_demod){
		return 'a';
	}
	if (demod.mode_demod==&lsb_demod){
		return 'l';
	}
	if (demod.mode_demod==&usb_demod){
		return 'u';
	}
	if (demod.mode_demod==&raw_demod){
		return 'r';
	}
}

void lib_set_auto_gain(){
	r = rtlsdr_set_tuner_gain_mode(dongle.dev, 0);
	dongle.gain = -100;
}

void lib_set_gain(int g){
	if (g >= 0){
		int ng;
		rtlsdr_set_tuner_gain_mode(dongle.dev, 1);
		ng = nearest_gain(dongle.dev,g*10);
		rtlsdr_set_tuner_gain(dongle.dev, ng);
		dongle.gain=ng;
	}
}

void lib_set_real_gain(int g){
	if (g > -100){
		rtlsdr_set_tuner_gain_mode(dongle.dev, 1);
		rtlsdr_set_tuner_gain(dongle.dev, g);
		dongle.gain=g;
	} else {
		lib_set_auto_gain();
	}
}

int lib_get_tuner_gains_count(){
	int count,r;
	rtlsdr_set_tuner_gain_mode(dongle.dev, 1);
	return rtlsdr_get_tuner_gains(dongle.dev, NULL);
}

void lib_get_tuner_gains(int *gains){
	int count,r;
	r = rtlsdr_set_tuner_gain_mode(dongle.dev, 1);
	count = rtlsdr_get_tuner_gains(dongle.dev, gains);
}

int lib_get_gain(){
	if (dongle.gain == -100){
		return -100;
	} else {
		return rtlsdr_get_tuner_gain(dongle.dev);
	}
}


uint32_t lib_frequency_convert(char *s){
	return (uint32_t)atofs(s);	
}


void  lib_stop()
{
	sighandler(0);
	rtlsdr_cancel_async(dongle.dev);
	pthread_join(dongle.thread, NULL);
	safe_cond_signal(&demod.ready, &demod.ready_m);
	pthread_join(demod.thread, NULL);
	safe_cond_signal(&output.ready, &output.ready_m);
	safe_cond_signal(&controller.hop, &controller.hop_m);
	pthread_join(controller.thread, NULL);
	demod_cleanup(&demod);
	output_cleanup(&output);
	controller_cleanup(&controller);

}

void  lib_output_close()
{
	if (output.file != stdout) {
		fclose(output.file);}
}

