#ifndef TASKINFO_H
#define TASKINFO_H

#include <QString>

#include "aria2/aria2.h"

struct TaskInfo{
	TaskInfo()
	{
		dnspeed = 0;
		upspeed = 0;
		totalLength = 0;
		uploadLength = 0;
		dnloadLength = 0;
	}

	void operator=(const TaskInfo &other)
	{
		if(!other.name.isEmpty())
			name = other.name;

		memcpy(&dnspeed, &other.dnspeed, offsetof(TaskInfo, xx) - offsetof(TaskInfo, dnspeed));
	}

	QString name;

	int dnspeed;
	int upspeed;

	int64_t totalLength;
	int64_t uploadLength;
	int64_t dnloadLength;

	int 	state;
	int 	picNums;
	int 	picLength;
	int 	xx;
};

struct TaskInfoEx : public TaskInfo{
	std::vector<aria2::FileData> fileData;
	aria2::KeyVals opts;
};

struct FinishTaskInfo{
	uint64_t id;
	QString name;
	int64_t size;
	QString datetime;
	QString localPath;
};

struct Task{
	int type; //1-url
	int state;
	uint64_t rid;
	std::string name;
	std::string dir;

	aria2::KeyVals opts;

	virtual std::string getUri() = 0;
	virtual std::string getLocal() = 0;
};

struct UriTask : public Task{

	std::string url;
	std::string getUri();
	std::string getLocal();
};

struct BtTask : public Task{
	std::string torrent;
	std::string getUri();
	std::string getLocal();
};

#endif // TASKINFO_H
