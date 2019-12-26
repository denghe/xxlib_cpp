#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <sys/inotify.h>  
#include <unistd.h>  

#define EVENT_NUM 12  

const char* event_str[EVENT_NUM] =
{
	"IN_ACCESS",
	"IN_MODIFY",
	"IN_ATTRIB",
	"IN_CLOSE_WRITE",
	"IN_CLOSE_NOWRITE",
	"IN_OPEN",
	"IN_MOVED_FROM",
	"IN_MOVED_TO",
	"IN_CREATE",
	"IN_DELETE",
	"IN_DELETE_SELF",
	"IN_MOVE_SELF"
};

int main(int argc, char* argv[])
{
	int fd;
	int wd;
	ssize_t len;
	ssize_t nread;
	char buf[BUFSIZ];
	struct inotify_event* event;
	int i;

	//if (argc < 2)
	//{
	//	fprintf(stderr, "%s path\n", argv[0]);
	//	return -1;
	//}

	fd = inotify_init();
	if (fd < 0)
	{
		fprintf(stderr, "inotify_init failed\n");
		return -1;
	}

	//wd = inotify_add_watch(fd, "./", IN_ALL_EVENTS);
	wd = inotify_add_watch(fd, "./main.cpp", IN_CLOSE_WRITE);
	if (wd < 0)
	{
		fprintf(stderr, "inotify_add_watch %s failed\n", argv[1]);
		return -1;
	}

	buf[sizeof(buf) - 1] = 0;
	while ((len = read(fd, buf, sizeof(buf) - 1)) > 0)
	{
		nread = 0;
		while (len - nread > 0)
		{
			event = (struct inotify_event*) & buf[nread];
			for (i = 0; i < EVENT_NUM; i++)
			{
				if ((event->mask >> i) & 1)
				{
					if (event->len > 0)
						fprintf(stdout, "%s --- %s\n", event->name, event_str[i]);
					else
						fprintf(stdout, "%s --- %s\n", " ", event_str[i]);
				}
			}
			nread = nread + sizeof(struct inotify_event) + event->len;
			//len = len - sizeof(struct inotify_event) - event->len;
		}
	}

	return 0;
}
