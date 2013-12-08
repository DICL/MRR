#ifndef _CONNCLIENT_
#define _CONNCLIENT_

#include <iostream>
#include <mapreduce/job.hh>
#include <master/dec_connclient.hh>

connclient::connclient(int fd)
{
	this->fd = fd;
	this->runningjob=NULL;
}

connclient::~connclient()
{
	close(fd);
	clearjob();
}

int connclient::getfd()
{
	return this->fd;
}

void connclient::setfd(int num)
{
	this->fd = num;
}

job* connclient::getrunningjob()
{
	return this->runningjob;
}

void connclient::setrunningjob(job* ajob)
{
	this->runningjob = ajob;
}

void connclient::clearjob()
{
	if(this->runningjob!=NULL)
	{
		delete runningjob;
		this->runningjob=NULL;
	}
}

#endif