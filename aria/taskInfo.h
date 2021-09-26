#ifndef TASKINFO_H
#define TASKINFO_H

#include <QString>

#include <aria2/aria2.h>

struct TaskInfo{

	void operator=(const TaskInfo &other)
	{
		if(!other.name.isEmpty())
			name = other.name;

		memcpy(&dnspeed, &other.dnspeed, offsetof(TaskInfo, fileData) - offsetof(TaskInfo, dnspeed));
		fileData = other.fileData;
	}

	QString name;

	int dnspeed;
	int upspeed;

	int64_t totalLength;
	int64_t uploadLength;
	int64_t dnloadLength;

	int 	picNums;
	int 	picLength;

	std::vector<aria2::FileData> fileData;

};

#endif // TASKINFO_H
