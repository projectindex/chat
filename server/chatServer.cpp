#include<string>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<iostream>
#include<map>
#include<vector>
#include<signal.h>
#include<pthread.h>
using namespace std;
map<string*,int> fdmap;
int ans=0;
void recvClient(int client_fd)
{
	map<string*,int>::iterator it;
	char clientName[200];
	int n=recv(client_fd,clientName,200,0);
	clientName[n]=0;
	char tmpName[1024];
	for(it=fdmap.begin();it!=fdmap.end();it++)
	{
		strcpy(tmpName,"add:");
		strcat(tmpName,clientName);
		strcat(tmpName,";");
		send(it->second,tmpName,strlen(tmpName),0);
	}
	fdmap.insert(pair<string*,int>(new string(clientName),client_fd));
	cout<<"size:"<<fdmap.size()<<endl;
}
void *str_parse(void *arg)
{
	map<string*,int>::iterator it;
	char recvs[2048];
	int n=0;
	int fd=*((int*)arg);
	recvClient(fd);
	char chat[2048];
	char *rmName;
	for(it=fdmap.begin();it!=fdmap.end();it++)
	{
		strcpy(recvs,"add:");
		strcat(recvs,(*it).first->c_str());
		strcat(recvs,";");
		send(fd,recvs,strlen(recvs),0);
		cout<<"sendto: "<<fd<<" "<<recvs<<endl;
		//for check
		//recv(fd,recvs,2048,0);
	}
	while(true)
	{
		if((n=recv(fd,recvs,2048,0))==-1)
			break;
		recvs[n]=0;
		for(it=fdmap.begin();it!=fdmap.end();it++)
			send(it->second,recvs,strlen(recvs),0);

		if(recvs[0]=='r'&&recvs[1]=='m')
		{
			rmName=&recvs[3];
			for(it=fdmap.begin();it!=fdmap.end();it++)
				if(strcmp(it->first->c_str(),rmName)==0) break;
			fdmap.erase(it);
			break;//loop is end
		}
	}
	cout<<"break"<<endl;
	close(fd);
}

int main()
{
	int sockfd,new_fd,ret;
	char tmpname[100];
	struct sockaddr_in my_addr,their_addr;
	socklen_t sin_size;
	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd<0)
	{
		cout<<"error in socket"<<endl;
		exit(1);
	}
	signal(SIGPIPE,SIG_IGN);
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(3333);
	my_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(my_addr.sin_zero),8);
	ret=bind(sockfd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr));
	if(ret<0)
	{
		cout<<"error in bind"<<endl;
		exit(1);
	}
	ret=listen(sockfd,10);
	if(ret<0)
	{
		cout<<"error in listen"<<endl;
		exit(1);
	}
	
	pthread_t pid;
	while(true)
	{
		sin_size=sizeof(struct sockaddr_in);
		new_fd=accept(sockfd,(struct sockaddr*)&their_addr,&sin_size);
		if(new_fd<0)
		{
			cout<<"error in accept"<<endl;
			exit(1);
		}
		//int nameLen=recv(new_fd,tmpname,100,0);
		//if(nameLen<0) continue;
		//tmpname[nameLen]=0;
		
		//fdmap.insert(pair<string*,int>(new string(tmpname),new_fd));	
		pthread_create(&pid,NULL,str_parse,&new_fd);
	}

	close(sockfd);
	return 0;
}

