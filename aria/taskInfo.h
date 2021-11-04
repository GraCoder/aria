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

		memcpy(&dnspeed, &other.dnspeed, offsetof(TaskInfo, totalLength) - offsetof(TaskInfo, dnspeed));

		if(other.totalLength > 0)
			memcpy(&totalLength, &other.totalLength, sizeof(int64_t) * 3);
	}

	QString name;

	int dnspeed;
	int upspeed;

	int 	state;
	int 	picNums;
	int 	picLength;

	int64_t totalLength;
	int64_t dnloadLength;
	int64_t uploadLength;
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
	uint64_t id;
	int type; //1-url
	int state;
	uint64_t to_size;
	uint64_t dn_size;
	uint64_t up_size;
	std::string name;
	std::string dir;

	aria2::KeyVals opts;

	virtual ~Task(){}
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
