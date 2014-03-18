#ifndef _CONNSLAVE_
#define _CONNSLAVE_

#include <iostream>
#include <mapreduce/definitions.hh>
#include "master_task.hh"

class connslave // connection to the slave
{
private:
	int fd;
	int maxtask;
	string address;
	vector<master_task*> running_tasks;

public:
	connslave(int fd);
	connslave(int maxtask, int fd);
	connslave(int maxtask, int fd, string anaddress);
	~connslave();
	int getfd();
	int getmaxtask();
	void setmaxtask(int num);
	int getnumrunningtasks();
	master_task* getrunningtask(int index);
	void add_runningtask(master_task* atask);
	void remove_runningtask(master_task* atask);
	void set_address(string anaddress);
	string get_address();
};

connslave::connslave(int fd)
{
	this->maxtask = 0;
	this->fd = fd;
}

connslave::connslave(int maxtask, int fd)
{
	this->maxtask = maxtask;
	this->fd = fd;
}

connslave::connslave(int maxtask, int fd, string anaddress)
{
	this->maxtask = maxtask;
	this->fd = fd;
	this->address = anaddress;
}

connslave::~connslave()
{
	close(fd);
}

int connslave::getfd()
{
	return this->fd;
}

int connslave::getmaxtask()
{
	return this->maxtask;
}

int connslave::getnumrunningtasks()
{
	return this->running_tasks.size();
}

void connslave::setmaxtask(int num)
{
	this->maxtask = num;
}

master_task* connslave::getrunningtask(int index)
{
	if((unsigned)index>=this->running_tasks.size())
	{
		cout<<"Index out of bound in the connslave::getrunningtask() function."<<endl;
		return NULL;
	}
	else
		return this->running_tasks[index];
}

void connslave::add_runningtask(master_task* atask)
{
	if(atask != NULL)
		running_tasks.push_back(atask);
	else
		cout<<"A NULL task is assigned to the running task vector in the connslave::add_runningtask() function."<<endl;
}

void connslave::remove_runningtask(master_task* atask)
{
	for(int i=0;(unsigned)i<running_tasks.size();i++)
	{
		if(running_tasks[i] == atask)
		{
			running_tasks.erase(running_tasks.begin()+i);
			break;
		}
	}
}

void connslave::set_address(string anaddress)
{
	this->address = anaddress;
}

string connslave::get_address()
{
	return this->address;
}

#endif
