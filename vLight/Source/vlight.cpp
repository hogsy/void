
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "std_lib.h"
#include "vlight.h"
#include "light.h"



/*
============
main
============
*/
int main (int argc, char **argv)
{
	printf ("======== vLight ========\n");

	long start_time;
	long end_time;
	time(&start_time);

	for (int i=1; i<argc; i++)
	{
		if (strcmp(argv[i], "-v") == 0)
			verbose = true;

		else if (strcmp(argv[i], "-log") == 0)
		{
			i++;
			flog = fopen(argv[i], "w");
			if (!flog)
				Error("couldn't open log file!");
		}

		else if (strcmp(argv[i], "-ambient") == 0)
		{
			i++;
			ambient[0] = (unsigned char)atoi(argv[i]);
			i++;
			ambient[1] = (unsigned char)atoi(argv[i]);
			i++;
			ambient[2] = (unsigned char)atoi(argv[i]);
		}

		else if (strcmp(argv[i], "-samples") == 0)
		{
			i++;
			samples = atoi(argv[i]);
			if (samples <= 0)
				samples = 0;
		}

		else if (argv[i][0] == '-')
			Error ("Unknown option \"%s\"", argv[i]);

		else
			break;
	}


	if (i != argc - 1)
		Error ("usage: vLight [options] bspfile");



	light_run(argv[i]);


	// log time
	time(&end_time);
	long seconds = end_time-start_time;
	long minutes = seconds/60;
	seconds %= 60;
	long hours = minutes / 60;
	minutes %= 60;
	v_printf("time elapsed - %2d:%2d:%2d (%d seconds)\n", hours, minutes, seconds, end_time-start_time);



	// close up log file
	if (flog)
		fclose(flog);

	return 0;
}


