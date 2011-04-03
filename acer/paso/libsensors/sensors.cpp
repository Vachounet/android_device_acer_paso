/*
  *  (c) Copyright Bosch Sensortec GmbH 2011

  *   Redistribution and use in source and binary forms, with or without
  *   modification, are permitted provided that the following conditions are
  *   met:
  *
  *  	 * Redistributions of source code must retain the above copyright
  *  	    notice, this list of conditions and the following disclaimer.
  *  	 * Redistributions in binary form must reproduce the above
  *  	    copyright notice, this list of conditions and the following
  *  	    disclaimer in the documentation and/or other materials provided
  *  	    with the distribution.
  *  	 * Neither the name of the author nor the names of its
  *  	    contributors may be used to endorse or promote products derived
  *  	    from this software without specific prior written permission.
  *
  *
  *   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
  *   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
  *   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
  *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
  *   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
  *   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
  *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
  *   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <hardware/sensors.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <math.h>
#include <poll.h>
#include <pthread.h>
#include <linux/input.h>
#include <cutils/atomic.h>
#include <cutils/log.h>

#define DEBUG_SENSOR		1

#define CONVERT                     (GRAVITY_EARTH / 256)
#define CONVERT_X                   -(CONVERT)
#define CONVERT_Y                   -(CONVERT)
#define CONVERT_Z                   (CONVERT)
#define SENSOR_NAME		"bma150"
#define INPUT_DIR               "/dev/input"
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

struct sensors_poll_context_t {
	struct sensors_poll_device_t device;
	int fd;
	char class_path[256];
};

static int poll__close(struct hw_device_t *dev)
{
	sensors_poll_context_t *ctx = (sensors_poll_context_t *)dev;
	if (ctx) {
		delete ctx;
	}

	return 0;
}

static int poll__activate(struct sensors_poll_device_t *dev,
        int handle, int enabled) {

	return 0;
}

static int set_sysfs_input_attr(char *class_path,
				const char *attr, char *value, int len)
{
	char path[256];
	int fd;
	if (class_path == NULL || *class_path == '\0'
	    || attr == NULL || value == NULL || len < 1) {
		return -EINVAL;
	}
	snprintf(path, sizeof(path), "%s/%s", class_path, attr);
	path[sizeof(path) - 1] = '\0';
	fd = open(path, O_RDWR);
	if (fd < 0) {
		return -errno;
	}
	if (write(fd, value, len) < 0) {
		close(fd);
		return -errno;
	}
	close(fd);

	return 0;
}

static int poll__setDelay(struct sensors_poll_device_t *device,
        int handle, int64_t ns) {

	sensors_poll_context_t *dev = (sensors_poll_context_t *)device;
	char buffer[20];
	int ms=ns/1000000;
	int bytes = sprintf(buffer, "%d\n", ms);

	set_sysfs_input_attr(dev->class_path,"delay",buffer,bytes);

	return 0;

}

static int poll__poll(struct sensors_poll_device_t *device,
        sensors_event_t* data, int count) {

	struct input_event event;
	int ret;
	sensors_poll_context_t *dev = (sensors_poll_context_t *)device;

	if (dev->fd < 0)
	return 0;

	while (1) {

		ret = read(dev->fd, &event, sizeof(event));

		if (event.type == EV_ABS) {

			switch (event.code) {
                        case ABS_X:
                                data->acceleration.x =
                                                -event.value * CONVERT_X;
                                break;
                        case ABS_Y:
                                data->acceleration.y =
                                                -event.value * CONVERT_Y;
                                break;
                        case ABS_Z:
                                data->acceleration.z =
                                                event.value * CONVERT_Z;
                                break;
/*
			case ABS_X:
				data->acceleration.y =
						-event.value * CONVERT_Y;
				break;
			case ABS_Y:
				data->acceleration.x =
						event.value * CONVERT_X;
				break;
			case ABS_Z:
				data->acceleration.z =
						-event.value * CONVERT_Z;
				break;
*/
			}
		} else if (event.type == EV_SYN) {

			data->timestamp =
			(int64_t)((int64_t)event.time.tv_sec*1000000000
					+ (int64_t)event.time.tv_usec*1000);
			data->sensor = 0;
			data->type = SENSOR_TYPE_ACCELEROMETER;
			data->acceleration.status = SENSOR_STATUS_ACCURACY_HIGH;


#ifdef DEBUG_SENSOR
			LOGD("Sensor data: t x,y,x: %f %f, %f, %f\n",
					data->timestamp / 1000000000.0,
							data->acceleration.x,
							data->acceleration.y,
							data->acceleration.z);
#endif

		return 1;

		}
	}

	return 0;
}


static int sensor_get_class_path(sensors_poll_context_t *dev)
{
	char *dirname = "/sys/class/input";
	char buf[256];
	int res;
	DIR *dir;
	struct dirent *de;
	int fd = -1;
	int found = 0;

	dir = opendir(dirname);
	if (dir == NULL)
		return -1;

	while((de = readdir(dir))) {
		if (strncmp(de->d_name, "input", strlen("input")) != 0) {
		    continue;
        	}

		sprintf(dev->class_path, "%s/%s", dirname, de->d_name);
		snprintf(buf, sizeof(buf), "%s/name", dev->class_path);

		fd = open(buf, O_RDONLY);
		if (fd < 0) {
		    continue;
		}
		if ((res = read(fd, buf, sizeof(buf))) < 0) {
		    close(fd);
		    continue;
		}
		buf[res - 1] = '\0';
		if (strcmp(buf, SENSOR_NAME) == 0) {
		    found = 1;
		    close(fd);
		    break;
		}

		close(fd);
		fd = -1;
	}
	closedir(dir);

	if (found) {
		return 0;
	}else {
		*dev->class_path = '\0';
		return -1;
	}

}

static int open_input_device(void)
{
	char *filename;
	int fd;
	DIR *dir;
	struct dirent *de;
	char name[80];
	char devname[256];
	dir = opendir(INPUT_DIR);
	if (dir == NULL)
		return -1;

	strcpy(devname, INPUT_DIR);
	filename = devname + strlen(devname);
	*filename++ = '/';

	while ((de = readdir(dir))) {
		if (de->d_name[0] == '.' &&
		    (de->d_name[1] == '\0' ||
		     (de->d_name[1] == '.' && de->d_name[2] == '\0')))
			continue;
		strcpy(filename, de->d_name);
		fd = open(devname, O_RDONLY);
		if (fd < 0) {
			continue;
		}


		if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), &name) < 1) {
			name[0] = '\0';
		}

		if (!strcmp(name, SENSOR_NAME)) {
#ifdef DEBUG_SENSOR
		LOGD("devname is %s \n", devname);
#endif
		} else {
			close(fd);
			continue;
		}
		closedir(dir);

		return fd;

	}
	closedir(dir);

	return -1;
}

static const struct sensor_t sSensorList[] = {

        { 	"BMA150 3-axis Accelerometer",
                "Bosch",
                1, 0,
                SENSOR_TYPE_ACCELEROMETER,
		4.0f*9.81f,
		(4.0f*9.81f)/256.0f,
		0.2f,
		0,
		{ }
	},

};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device);

static int sensors__get_sensors_list(struct sensors_module_t* module,
        struct sensor_t const** list)
{
	*list = sSensorList;

	return ARRAY_SIZE(sSensorList);
}

static struct hw_module_methods_t sensors_module_methods = {
	open : open_sensors
};

extern "C" const struct sensors_module_t HAL_MODULE_INFO_SYM = {
	common :{
		tag : HARDWARE_MODULE_TAG,
		version_major : 1,
		version_minor : 0,
		id : SENSORS_HARDWARE_MODULE_ID,
		name : "Bosch sensor module",
		author : "Bosch Sensortec",
		methods : &sensors_module_methods,
		dso : NULL,
		reserved : {},
	},

	get_sensors_list : sensors__get_sensors_list
};

static int open_sensors(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device)
{
	int status = -EINVAL;

	sensors_poll_context_t *dev = new sensors_poll_context_t();
	memset(&dev->device, 0, sizeof(sensors_poll_device_t));

	dev->device.common.tag = HARDWARE_DEVICE_TAG;
	dev->device.common.version  = 0;
	dev->device.common.module   = const_cast<hw_module_t*>(module);
	dev->device.common.close    = poll__close;
	dev->device.activate        = poll__activate;
	dev->device.setDelay        = poll__setDelay;
	dev->device.poll            = poll__poll;


	if(sensor_get_class_path(dev) < 0) {
		LOGD("g sensor get class path error \n");
	} else {
		dev->fd = open_input_device();
		*device = &dev->device.common;
		status = 0;
	}

	return status;
}