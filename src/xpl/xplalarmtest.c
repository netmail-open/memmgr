#include <config.h>
#include <stdlib.h>
#include <xpl.h>

int					result		= -1;
XplSemaphore		sem;
XplAlarmHandle		alarmhandle	= NULL;

/*
	Once

	Create an alarm that runs once, and does not repeat
*/
typedef struct
{
	XplAlarmP		alarm;
	int				runcount;
} AlarmRunOnceData;

static void AlarmRunOnceCB(void *cbdata)
{
	AlarmRunOnceData	*data	= cbdata;

	data->runcount++;
	XplAlarmDestroy(data->alarm);
}

static int AlarmRunOnce( XplThread_ thread )
{
	AlarmRunOnceData	data;
	int					i;

	memset(&data, 0, sizeof(data));

	if (!(data.alarm = XplAlarmCreate(AlarmRunOnceCB, &data, 1, 0, alarmhandle))) {
		fprintf(stderr, "Could not create alarm\n");

		result = 1;
		XplSemaPost(sem);
		return(result);
	}

	for (i = 0; i < 10; i++) {
		XplDelay(1000);

		if (data.runcount == 1) {
			result = 0;
			XplSemaPost(sem);
			return(result);
		}
	}

	fprintf(stderr,
		"Expected callback to be hit once, was hit %d time(s)\n",
		data.runcount);

	result = 2;
	XplSemaPost(sem);
	return(result);
}

/*
	Repeat

	Create a repeating alarm and make sure it runs x times, then cancel
*/
typedef struct
{
	XplAlarmP		alarm;
	int				runcount;
} AlarmRepeatData;

static void AlarmRepeatCB(void *cbdata)
{
	AlarmRepeatData	*data	= cbdata;

	data->runcount++;

	if (data->runcount == 5) {
		XplAlarmDestroy(data->alarm);
	}
}

static int AlarmRepeat( XplThread_ thread )
{
	AlarmRepeatData		data;
	int					i;

	memset(&data, 0, sizeof(data));

	if (!(data.alarm = XplAlarmCreate(AlarmRepeatCB, &data, 1, 1, alarmhandle))) {
		fprintf(stderr, "Could not create alarm\n");

		result = 1;
		XplSemaPost(sem);
		return(result);
	}

	for (i = 0; i < 10; i++) {
		XplDelay(1000);

		if (data.runcount == 5) {
			result = 0;
			XplSemaPost(sem);
			return(result);
		}
	}

	fprintf(stderr,
		"Expected callback to be hit 5 times, was hit %d time(s)\n",
		data.runcount);

	result = 2;
	XplSemaPost(sem);
	return(result);
}

/*
	Cancel

	Create an alarm but cancel it before it can run
*/
typedef struct
{
	XplAlarmP		alarm;
	int				runcount;
} AlarmCancelData;

static void AlarmCancelCB(void *cbdata)
{
	fprintf(stderr, "Alarm was canceled but still fired\n");
	DebugAssert(0);
}

static int AlarmCancel( XplThread_ thread )
{
	AlarmCancelData	data;
	int					i;

	memset(&data, 0, sizeof(data));

	if (!(data.alarm = XplAlarmCreate(AlarmCancelCB, &data, 1, 0, alarmhandle))) {
		fprintf(stderr, "Could not create alarm\n");
		result = 1;
		XplSemaPost(sem);
		return(result);
	}

	XplAlarmDestroy(data.alarm);
	for (i = 0; i < 5; i++) {
		XplDelay(1000);
	}

	result = 0;
	XplSemaPost(sem);
	return(result);
}

/*
	Multiple

	Create multiple alarms, and cancel some. Don't exit until the correct number
	have been fired.
*/
#define MULTI_ALARM_COUNT		30
typedef struct
{
	XplAlarmP		alarm[MULTI_ALARM_COUNT];
	int				runcount;
	XplSemaphore	sem;
} AlarmMultipleData;

static void AlarmMultipleCB(void *cbdata)
{
	AlarmMultipleData	*data	= cbdata;

	XplSemaPost(data->sem);
}

static int AlarmMultiple( XplThread_ thread )
{
	AlarmMultipleData	data;
	int					i;

	memset(&data, 0, sizeof(data));

	XplSemaInit(data.sem, 0);

	for (i = 0; i < MULTI_ALARM_COUNT; i++) {
		if (!(data.alarm[i] = XplAlarmCreate(AlarmMultipleCB, &data, 1, 0, alarmhandle))) {
			fprintf(stderr, "Could not create alarm\n");
			result = 1;
			XplSemaPost(sem);
			return(result);
		}

		/* Cancel every other alarm before it can fire */
		if (i % 2) {
			XplAlarmDestroy(data.alarm[i]);
			data.alarm[i] = NULL;
		}

		/* Add a bit of a delay */
		XplDelay(i * 10);
	}

	while (data.runcount < MULTI_ALARM_COUNT / 2) {
		XplSemaWait(data.sem);
		data.runcount++;
	}

	for (i = 0; i < MULTI_ALARM_COUNT; i++) {
		if (data.alarm[i]) {
			XplAlarmDestroy(data.alarm[i]);
			data.alarm[i] = NULL;
		}
	}

	if (data.runcount == MULTI_ALARM_COUNT / 2) {
		result = 0;
	} else {
		result = 1;
	}

	XplSemaPost(sem);
	return(result);
}



int XplMain(int argc, char **argv)
{
	int				a;
	XplThreadGroup	threadGroup;

	XplSemaInit(sem, 0);
	alarmhandle = XplAlarmInit();

	threadGroup = XplThreadGroupCreate( "alarm" );
	for (a = 1; a < argc; a++) {
		if (!stricmp(argv[a], "runonce")) {
			XplThreadStart( threadGroup, AlarmRunOnce, NULL, NULL );
		} else if (!stricmp(argv[a], "repeat")) {
			XplThreadStart( threadGroup, AlarmRepeat, NULL, NULL );
		} else if (!stricmp(argv[a], "cancel")) {
			XplThreadStart( threadGroup, AlarmCancel, NULL, NULL );
		} else if (!stricmp(argv[a], "multiple")) {
			XplThreadStart( threadGroup, AlarmMultiple, NULL, NULL );
		} else {
			result = 1;

			fprintf(stderr, "Unknown test: %s\n", argv[a]);
			XplSemaPost(sem);
		}

		XplSemaWait(sem);

		if (result) {
			break;
		}
	}

	XplAlarmShutdown(&alarmhandle);
	return(result);
}

