
# *Program:ftp_client.py
# *Author: Tracy Stinghen
# *Date Created: 5/27/15
# *Description: FTP Server Program for Project #2 CS 372 Spring 2015

# Resources: Beej's Guide to Network Programming 
# TCP/IP Programming in C by Prof. Vinod Pillai (on youtube.com) 
# http://www.binarytides.com/code-chat-application-server-client-sockets-python/
# http://www.tenouk.com/Module41.html
# http://pymotw.com/2/socket/tcp.html#echo-client



import socket 
import sys 
import os.path


def make_control_connect(): 
	control_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) #creates a socket

	port_num = input('Enter Port Number: ')			#enter same port number as server 
	server_address = ('localhost', port_num)

	print >>sys.stderr, 'connecting to %s port %s ' % server_address 

	control_sock.connect(server_address)
	
	return control_sock 
	
def make_data_connect(message, control_sock): 
	data_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	data_sock.bind(('', 0))
	data_sock.listen(1)
	data_port = data_sock.getsockname()[1]

	message = message + ' ' + str(data_port)
	
	control_sock.sendall(message)
	
	data_sock, maddr = data_sock.accept()
	print >> sys.stderr, 'Data connection made with ', maddr 
	
	return data_sock
	
def close_exit(control_sock): 
	
	print >>sys.stderr, 'closing socket, exiting' 
	control_sock.close()  	#closes socket before exiting
	exit() 
	
def write_file(data, data_sock, contron_sock, file_name): 
		if "Error" in data: 
			print >> sys.stderr, '%s' % data  
			command_cycle(control_sock)
		
		else: 
			file_size = int(data) 
			print >> sys.stderr, 'Writing %d bytes of data' % file_size
			
			#if_exist = open(file_name,'r+')

			if os.path.isfile(file_name) : 
				rewrite = raw_input('File exists, want to re-write? (type yes to rewrite): ')
				
				if "yes" not in rewrite: 
					print >> sys.stderr, 'File will not be over-written'
					command_cycle(control_sock)
					
				new_file = open(file_name, 'w') 
				
			else: 
			
				new_file = open(file_name, 'w') 
				
			
			
			recieved = 0 
			
			while file_size > recieved: 
			
				data = data_sock.recv(1064)
				recieved = recieved + sys.getsizeof(data)  
				#less size of header data per packet
				recieved = recieved - 40 
				
				new_file.write(data)
				
				#print >> sys.stderr, 'Data Recieved: %d' % recieved 
					
			
			print >> sys.stderr, 'Data written: %d bytes' % recieved 
			new_file.close()

			
			
def command_cycle(control_sock): 
	while 1: 
		message = ""; 
		command = raw_input('Enter command(-l or -g or -q): ')
		if command == '-g': 
			file_name = raw_input('Enter file name: ') 
			message = command + ' ' + file_name 
			
			

		elif command == '-l': 
			message = command + ' none'  
			# file_name = raw_input('Enter file name: ') 
			# message = command + ' ' + file_name 

			
		elif command == '-q': 
			close_exit(control_sock) 

		else:
			print >>sys.stderr, 'Error - Invalid Command' 
			command_cycle(control_sock) 
			#Make this go back to the beginning of the loop... 
			
		
		data_sock = make_data_connect(message, control_sock)
		
		data = data_sock.recv(1024) 
		
		if command == '-l': 
			
			print >> sys.stderr, 'From server: %s ' % data 
			
		if command == '-g': 
		
			write_file(data, data_sock, control_sock, file_name) 
			
			print >> sys.stderr, 'Success: wrote %s ' % file_name 

		
control_sock = make_control_connect() 

command_cycle(control_sock) 
		
		

