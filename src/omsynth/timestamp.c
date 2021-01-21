/*
* Copyright (c) Newcastle University, UK.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

// Epoch time helper functions
// Dan Jackson

#ifdef _WIN32
	#define _CRT_SECURE_NO_WARNINGS
	#define timegm _mkgmtime
#elif __APPLE__				// Fix for Mac build by Alberto Corbi Bellot
#else
	#define _BSD_SOURCE		// Both of these lines
	#include <features.h>	// ...needed for timegm() in time.h on Linux
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/timeb.h>

#include "timestamp.h"


// Returns the number of seconds since the epoch
double TimeNow()
{
	struct timeb tp;
	ftime(&tp);
	return ((unsigned long long)tp.time * 1000 + tp.millitm) / 1000.0;
}


// Convert an epoch time to a string time representation ("YYYY-MM-DD hh:mm:ss.fff")
char *TimeString(double t, char *buff)
{
	static char staticBuffer[TIME_MAX_STRING] = { 0 };	// "2000-01-01 20:00:00.000|"
	if (buff == NULL) { buff = staticBuffer; }			// Static buffer is not thread safe
	time_t tn = (time_t)t;
	struct tm *tmn = gmtime(&tn);
	float sec = tmn->tm_sec + (float)(t - (time_t)t);
	sprintf(buff, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 1900 + tmn->tm_year, tmn->tm_mon + 1, tmn->tm_mday, tmn->tm_hour, tmn->tm_min, (int)sec, (int)((sec - (int)sec) * 1000));
	return buff;
}


// Parse a string time representation ("YYYY-MM-DD hh:mm:ss.fff") in to seconds since the epoch
double TimeParse(const char *timeString)
{
	int index = 0;
	char *token = NULL;
	char *p;
	char end = 0;
	struct tm tm0 = { 0 };
	double fraction = 0;
	char tstr[TIME_MAX_STRING];
	char err = 0;

	//strcpy_s(tstr, sizeof(tstr), timeString);
	{
		size_t len = strlen(timeString);
		if (len >= TIME_MAX_STRING - 1) { len = TIME_MAX_STRING - 1; }
		memcpy(tstr, timeString, len);
		tstr[len] = '\0';
	}

	for (p = tstr; !end; p++)
	{
		end = (*p == '\0');
		if (token == NULL && *p >= '0' && *p <= '9')
		{
			token = p;
		}
		if (token != NULL && (*p < '0' || *p > '9'))
		{
			*p = '\0';
			int value = atoi(token);
			int digits = (int)strlen(token);
			token = NULL;
			if (index == 0) { if (value < 1900 || value > 2100) { err = 1; } tm0.tm_year = value - 1900; }		// since 1900
			else if (index == 1) { if (value < 1 || value > 12) { err = 1; } tm0.tm_mon = value - 1; }	// 0-11
			else if (index == 2) { if (value < 1 || value > 31) { err = 1; } tm0.tm_mday = value; }		// 1-31
			else if (index == 3) { if (value < 0 || value > 23) { err = 1; } tm0.tm_hour = value; }		// 
			else if (index == 4) { if (value < 0 || value > 59) { err = 1; } tm0.tm_min = value; }		// 
			else if (index == 5) { if (value < 0 || value > 59) { err = 1; } tm0.tm_sec = value; }		// 
			else if (index == 6) { fraction = value / pow(10.0, (double)digits); }				// 
			index++;
		}
	}
	if (index < 5) { err = 1; }
	if (err != 0) { return 0; }
	double t = (double)timegm(&tm0) + fraction;
	return t;
}

