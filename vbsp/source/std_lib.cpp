
#include "std_lib.h"

/*
============
replacement printf
============
*/
bool		verbose	= false;
FILE		*flog	= NULL;
void v_printf(char *msg, ...)
{
	static char buff[1024];

	va_list args;
	va_start(args, msg);
	vsprintf(buff, msg, args);
	va_end(args);

	if (flog)
		fprintf(flog, "%s", buff);

	if (verbose)
		printf("%s", buff);
}



/*
============
Error - just drop everything and quit
============
*/
void Error(char *err, ...)
{
	char buff[1024];

	va_list args;
	va_start(args, err);
	vsprintf(buff, err, args);
	va_end(args);

	verbose = true;

	v_printf("\n************ ERROR ************\n");
	v_printf("%s", buff);
	v_printf("\n");

	exit (1);
}


/*
============
FError - same as error
============
*/
void FError(char *err, ...)
{
	char buff[1024];

	va_list args;
	va_start(args, err);
	vsprintf(buff, err, args);
	va_end(args);

	verbose = true;

	v_printf("\n************ ERROR ************\n");
	v_printf("%s", buff);
	v_printf("\n");

	exit (1);
}


/*
============
ComPrintf - same as error
============
*/
void ComPrintf(char *err, ...)
{
	char buff[1024];

	va_list args;
	va_start(args, err);
	vsprintf(buff, err, args);
	va_end(args);

	verbose = true;

	v_printf("\n************ ERROR ************\n");
	v_printf("%s", buff);
	v_printf("\n");

	exit (1);
}
