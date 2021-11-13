#ifndef TASKINFO_H
#define TASKINFO_H

#include <QString>

#include "aria2.h"

enum TaskType{
	enTaskType_Uri = 1,
	enTaskType_Bt
};

struct TaskUpdateInfo{
	TaskUpdateInfo()
	{
		dnspeed = 0;
		upspeed = 0;
		totalLength = 0;
		uploadLength = 0;
		dnloadLength = 0;
	}

	void operator=(const TaskUpdateInfo &other)
	{
		memcpy(&dnspeed, &other.dnspeed, offsetof(TaskUpdateInfo, totalLength) - offsetof(TaskUpdateInfo, dnspeed));

		if(other.totalLength > 0)
			memcpy(&totalLength, &other.totalLength, sizeof(int64_t) * 3);
	}

	static int surfixToInt(std::string &suf)
	{
		int ret; suf.resize(4, 0);
		memcpy(&ret, suf.c_str(), 4);
		return ret;
	}

	static std::string intToSurfix(int iconType)
	{
		std::string ret(4, 0);
		memcpy(ret.data(), &iconType, 4);
		return ret;
	}

	int 	dnspeed;
	int 	upspeed;

	int 	picNums;
	int 	picLength;

	int64_t totalLength;
	int64_t dnloadLength;
	int64_t uploadLength;

	int 	state;
};

struct TaskInfo{
	uint64_t 	id;
	QString 	name;
	int			type;
	int 		iconType;
};

struct TaskInfoEx : public TaskInfo, public TaskUpdateInfo{

	aria2::BtMetaInfoData metaInfo;
	std::vector<aria2::FileData> fileData;
	aria2::KeyVals opts;

	void operator=(const TaskUpdateInfo &other)
	{
		TaskUpdateInfo::operator=(other);
	}
};

struct FinishTaskInfo : public TaskInfo{
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
	std::string agent;
	std::string cookie;
	std::string mime;

	std::string getUri();
	std::string getLocal();
};

struct BtTask : public Task{
	std::string torrent;
	std::string getUri();
	std::string getLocal();
};

#endif // TASKINFO_H
