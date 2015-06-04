/*********************
*Program: ftp_server.cpp 
*Author: Tracy Stinghen
*Date Created: 5/26/15
*Description: FTP Server Program for Project #2 CS 372 Spring 2015


*Resources: Beej's Guide to Network Programming 
http://www.dreamincode.net/forums/topic/59943-accessing-directories-in-cc-part-i/
http://www.linuxhowtos.org/C_C++/socket.htm
http://codewiki.wikidot.com/c:system-calls:fstat
http://www.cplusplus.com/doc/tutorial/files/
http://stackoverflow.com/questions/2409504/using-c-filestreams-fstream-how-can-you-determine-the-size-of-a-file
http://runnable.com/UpSpP5ymsslIAAFo/reading-a-file-into-an-array-for-c%2B%2B-and-fstream


Using code from Project #1, which used the following resources: 
TCP/IP Programming in C by Prof. Vinod Pillai (on youtube.com) 
http://www.binarytides.com/code-chat-application-server-client-sockets-python/
http://www.tenouk.com/Module41.html
http://pymotw.com/2/socket/tcp.html#echo-client
***************************/

#include <iostream> 
#include <string>
#include <cstring>
#include <cstdlib>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <dirent.h> 
#include <iostream>
#include <fstream>
#include <stdio.h>



	/****** These functions are copied straight from Project #1 *******/ 
void error(std::string msg); 	//prints an error message and exits the program

int port_num(int argc, char *argv[]); //gets the port number from the command line

int start_server(int sockfd, int portnum, struct sockaddr_in &server_addr); //initializes the server 

int send_message(char message[], int rec_fd);

	/********* End section of functions from Project #1 **********/ 

void now_listen(int sockfd);

void get_commands(std::string (&commands)[3], char buffer[]);

void get_dir(char (&list)[1024]);

int data_connect(int portnum); 

int get_fsize( std::string fname, char (&buffer)[1024]);

void send_file(int fsize, std::string fname, int sendfd); 

	
int main(int argc, char *argv[]) 
{
	
	int sockfd; 
	int portnum; 
	struct sockaddr_in server_addr;
	
	 
	
	
	/***************** Setting Up Server ***********************/ 
	
	portnum = port_num(argc, argv); 
	
	sockfd = start_server(sockfd, portnum, server_addr); 
	

	
	/****************** Talking to Clients ***********************/
	
	now_listen(sockfd); 

	close(sockfd);

	return 0; 
}
	
	/***************** LISTEN ************************/


void now_listen(int sockfd)
{
	struct sockaddr_in client_addr; //client address info 
	socklen_t addr_size; 
	char buffer[1024]; 
	
	int clientfd; 
	int datafd; 
	int buff_size = 1024; 
	int data_port; 
	
	int file_size; 
	

	
	std::string commands[] = {"none", "none", "none"}; 
	
	
	
	listen(sockfd, 2); 			//second argument declares how many clients can access
	
	addr_size = sizeof client_addr;
	clientfd  = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
	
	
	//this section below should repeat continuously
	while(1)
	{
	
	
		bzero(buffer, buff_size);	
		
		recv(clientfd, buffer, buff_size, 0);
		
		get_commands(commands, buffer); 
		
		if (commands[0] != "none")
		{
			
			data_port = atoi(commands[2].c_str()); 
			
			if (commands[0] == "-l") 
			{
				get_dir(buffer); 
				
				
				
				datafd = data_connect(data_port);  
				
				send_message(buffer, datafd); 
				
				close(datafd);
			
			}
			
			if (commands[0] == "-g") 
			{
				file_size = get_fsize(commands[1], buffer);   //also sets the buffer to contain file size 
				
				if(file_size < 0)
				{
					strcpy(buffer, "Error opening file\nPlease check file name "); 
					
				}
				
				//get_dir(buffer); 
				
				datafd = data_connect(data_port);
					
				send_message(buffer, datafd);
				
				send_file(file_size, commands[1], datafd); 
				
				close(datafd); 
				
			}
	
		}
	}
	


		
}

/*************** SEND FILE ******************/ 
void send_file(int fsize, std::string fname, int sendfd) 
{
	int buff_size = 1024; 
	
	char buffer[buff_size]; 
	int bytes_sent; 
	int length = 0; 
	
	std::ifstream afile (fname.c_str());
	
	bzero(buffer, buff_size);
	
	if(afile.is_open())
	{
	
		while (!afile.eof())
		{
			afile.get(buffer[length]); 
			length++; 
			
			if(length >= buff_size)
			{
				
				send_message(buffer, sendfd); 
				bzero(buffer, buff_size);
				length = 0; 
				
				std::cout<<"Buffer:"<<buffer<<std::endl;
				
			}
		}
		
		if(length > 0) 
		{
			send_message(buffer, sendfd); 
			std::cout<<"Buffer:"<<buffer<<std::endl; 
		}
		
	
	
	}



}

/************ GET FILE SIZE *****************/ 
int get_fsize( std::string fname, char (&buffer)[1024])
{
	 
	 int filesize;
	 std::ifstream afile (fname.c_str(), std::ifstream::binary);
	 

	 //afile.open(fname.c_str()); 
	 
	 if(afile.is_open()) 
	 {
	 	
		std::cout<<"Success opening file: "<<fname<<"\n"; 
		 
	 	
	 
	 
	afile.seekg(0, afile.end); 

	filesize = afile.tellg(); 
	
	afile.close(); 
	
	std::cout<<"This is the filesize:" << filesize<<std::endl; 
	
	sprintf(buffer, "%d", filesize); 

	
	return filesize; 
	
	} 
	
	else 
	{
		std::cout<<"Error opening file\n"; 
		return -1; 
	}

}

/************ MAKE DATA CONNECTION ************/ 
int data_connect(int portnum) 
{
	
	int sockfd; 
	struct sockaddr_in client_addr; 
	struct hostent *server; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	
	server = gethostbyname("localhost");
	
	bzero((char*) &client_addr, sizeof(client_addr)); 
	client_addr.sin_family = AF_INET; 
	
	bcopy((char *)server->h_addr, (char *)&client_addr.sin_addr.s_addr, server->h_length);

	client_addr.sin_port = htons(portnum); 
	
	if(connect(sockfd, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0)
	{
		error("Error making Data Connection"); 
	}	 	 
	
	return sockfd; 

}

/************** SEND MESSAGE *****************/
	
int send_message(char message[], int rec_fd)
{

	int data_sent; 
	

	std::cout<<"Message being sent:"<<message<<std::endl; 
	data_sent = send(rec_fd, message, strlen(message), 0);
	
	if(data_sent == -1) 	//attempts to send message to the reciever's file descriptor
		{
			error("Sending failed"); 
		}
		
		else 
		{
			std::cout<<"Success on sending\n"; 
		}
		
		std::cout<<"Size of message is: "<<strlen(message)<<"\n";
		
		 

}


/***************** PARSE COMMANDS ****************/

void get_commands(std::string (&commands)[3], char buffer[])
{
	
	char * tokenizer;


	tokenizer = strtok(buffer, " "); 
		
	for(int i = 0; i < 3; i++) 
	{
		if(tokenizer != NULL) 
		{
			commands[i] = tokenizer; 
			
			tokenizer = strtok(NULL, " "); 
		}
		
		std::cout<<commands[i]<<std::endl; 
	}

}
	
	
/********* GET CURRENT DIRECTORY FILES *********/

void get_dir(char (&list)[1024])
{
	DIR *currdir = NULL; 
	struct dirent *currdirent = NULL; 
	
	currdir = opendir("."); 
	
	strcpy(list, "\nFiles For Transmit: "); 
	
	if (currdir == NULL) 
	{
		error("Error opening current directory"); 
	}
	
	while(currdirent = readdir(currdir))
	{
		if(currdirent == NULL) 
		{
			error("Error reading current directory"); 
			
		}
		
		if(strstr(currdirent->d_name, ".txt") != NULL)
		{
			strcat(list, "\n"); 
			strcat(list, currdirent->d_name); 
		}
		
	
	}


		
}




	/****************** ERROR ***********************/

	
void error(std::string msg)
{
	std::cout << msg << std::endl; 
	exit(1); 
	
}



	/****************** PORT NUMBER ***********************/
	
	
int port_num(int argc, char *argv[])
{
	int portnum; 
	
	if (argc < 2) 
	{
		std::cout<<"No Port Provided. Exiting."; 
		exit(1); 
	}
	
	portnum = atoi(argv[1]); 
	
	return portnum; 

}

	/****************** START SERVER ***********************/

int start_server(int sockfd, int portnum, struct sockaddr_in &server_addr)
{

	int yes = 1; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 	//opens socket
	
	if(sockfd < 0) 
	{
		error("Error when opening socket"); 
	}
	
	std::cout<<"Success opening socket \n"; 
	
	bzero((char *) &server_addr, sizeof(server_addr));	//clears server address
	
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)  //allows multiple clients to connect
	{
		error("Error when declaring reusable socket"); 
	}

	
	server_addr.sin_family = AF_INET; 			//going to use IP protocol 
	server_addr.sin_addr.s_addr = INADDR_ANY; 
	server_addr.sin_port = htons(portnum); 
	
	if(bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) <0)  //binds to the socket
	{
		error("Error when binding to socket"); 
	}
	
	std::cout<<"Success binding socket \n"; 



	return sockfd; 
}

