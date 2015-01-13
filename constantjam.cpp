#include <stdio.h>
#include <iostream>

#include "osal_wi.h"

const char warning[] =
	"Warning:\n"
	"- Some devices stop operating properly when in monitor mode\n"
	"  and attempting to transmit packets while you are jamming!\n"
	"  Setting force_channel_idle may help if you experience problems.\n"
	"- Some devices stop recieving frames after being jammed for a long time.\n"
	"  Perhaps because they configured an unrealisticly high noise-floor level.\n"
	"  Plug the device you jammed in/out if it no longer seems to be working!\n\n";

int main(int argc, char *argv[])
{
	wi_dev dev;

	if (argc != 3) {
		printf("Usage: contjam interface channel\n");
		return 1;
	}

	if (osal_wi_open(argv[1], &dev) < 0) return 1;

	printf("%s", warning);

	osal_wi_setchannel(&dev, atoi(argv[2]));
	osal_wi_constantjam_start(&dev);

	printf("Press any key to stop jamming...\n");
	getchar();

	osal_wi_constantjam_stop(&dev);

	return 0;
}

