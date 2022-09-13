#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>    		
#include <arpa/inet.h>     
#include <netinet/in.h>  
#include <sys/time.h>  
#include <string.h>
#include <stdlib.h>	
#define MSG_LEN 256


/**
 * Structure defining the msg 
 * to be used for communication <br>
 * 
 **/
struct packet{
	int msg_type;
	char msg[MSG_LEN];   
};


/**
 * Convert the encoded string 
 * to binaryary form <br>
 * 
 **/
void string_to_binary(char * ip,int *binary,int n) { 
	for(int i=0;i<4;i++){
		int temp;
		int j=i+n;
		if(ip[j]>='a' && ip[j]<='z'){   
			temp=ip[j]-'a'+26;
		}
		
		else if(ip[j]>='A' && ip[j]<='Z'){
			temp=ip[j]-'A';
		}
		
		else if(ip[j]>='0' && ip[j]<='9'){
			temp=ip[j]-'0'+52;
		}
		
		else if(ip[j]=='/'){
			temp=63;
		}
		
		else if(ip[j]=='+'){
			temp=62;
		}
		
		else{
			temp=0;
		}
		
		for(int j=5;j>=0;j--){ // Convert to binary
			binary[j+(i*6)]=temp%2;
			temp=temp>>1;
		}
	}
}


/**
 * Take 24 bits and group as (8+8+8)
 * Every 8 bits represents a character
 * 24 bits converted to 3 characters
 **/
void convert_to_string(char *op,int* binary,int s,int type){  
	for(int i=0;i<3;i++){
		int temp=0;
		int val=1;
		int j=7;
		while(j>=0){
			temp+=val*binary[j+i*8];
			val=val<<1;
			j--;
		}
		
		temp=temp-97;
		op[s+i]='a'+temp;
	}
}



/**
 * Decode the string 
 * (Converting group of 24 bits to binaryary then string)
 * 
 **/
void decode(char *ip,int n,char *op){
	int binary[24];
	if(n%4!=0){
		printf("Wrong input\n");
		return;
	}

	int temp=(3*n)/4;

	/**
 	 * Checks the padding of "="
 	 **/
	if(ip[n-1]=='='){	
		temp=temp-1;
	}

	else if(ip[n-2]=='='){ 
		temp=temp-2;
	}
	
	else{
		temp=3*n/4;
	}


	for(int i=0;i<n;i=i+4){	
		string_to_binary(ip,binary,i);
		convert_to_string(op,binary,3*i/4,0);
	}

	op[temp]='\0';
}



/**
 * Prints the Error msg
 * 
 **/
void error(char *msg){
		perror(msg);
		exit(1);
}


 /**
  * main function
  * 
  **/
int main(int argc, char *argv[]) {

	if (argc <= 1){
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	int max_sd , max_clients = 30 ;  
	int client_socket[max_clients] , pbg, i , sd;    
	int sockfd, newsockfd, portno, clilen;
	int n;

	char* buffer=(char *)malloc(MSG_LEN*sizeof(char)); // Server buffer
	struct sockaddr_in serv_addr, cli_addr;
	fd_set readfds; 	// List of socket fds


	for (i = 0; i < max_clients; i++){     // client_socket is a bitmap denoting occupied/non-occupied sockets
		client_socket[i] = 0;   
	}

	
	/**
	 *  CREATION OF TCP SOCKET
	 */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	
	/*
	 *  Fields of serv_addr filled before binaryding
	 */
	bzero((char *) &serv_addr, sizeof(serv_addr)); 
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	

	/**
	 *  binaryD associates and reserves a port for use by the socket
	 */
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("ERROR on binaryding");
	}
	

	/**
	 * Listens the connections from clients
	 */
	listen(sockfd,5); 
	

	struct packet msg;

	while(1){
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;
		
		for ( i = 0 ; i < max_clients ; i++){
			  
			sd = client_socket[i];   
			  
			// Only sd which are in use/ value =1 are added
			if(sd > 0){   	
				FD_SET( sd , &readfds);
			}   
			  

			// Max_sd needed for select later
			if(sd > max_sd){  
				max_sd = sd;
			}   
		}
			 
		// Select gets activated if any sockfd in readfs is active
		pbg = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  

		if ((pbg < 0)){   
			error("select error");   
		}   	
			
		/**
		 * Checks if there is activity on main socket, indicating new connection
		 * 
		 */		 
		if (FD_ISSET(sockfd, &readfds)){ 

			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // Accept the incoming connection

			if (newsockfd < 0){
				error("ERROR on accept");
			}
			
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , newsockfd , inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port)); 
			n = write(newsockfd,"Welcome to team",15);
			if (n < 0){
				error("ERROR writing to socket");
			}
					
		  
			
			for (i = 0; i < max_clients; i++){
				/**
				 *  Add new connection to first non-zero entry of client_socket  
				 */
				if( client_socket[i] == 0 ){
					client_socket[i] = newsockfd;   
					printf("Adding to list of sockets as %d\n" , i);   
					break;   
				}   
			}   
		}
		 

		// Else its some IO operation on some other socket 

		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];  
			if (FD_ISSET( sd , &readfds)){ 
				bzero(buffer,MSG_LEN);
				n = read(sd,buffer,MSG_LEN);
				
				if (n < 0){
					error("ERROR reading from socket");
				}

				/**
				 * Close socket and clears bitmap
				 * */
				if (n == 0){   
                    getpeername(sd , (struct sockaddr*)&cli_addr ,(socklen_t*)&cli_addr);
                    printf("Host disconnected , ip %s , port %d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));            
                    close( sd );  
                    client_socket[i] = 0;
                }    
     			

                /**
                 * get the type of msg from the first byte
                 * */
				msg.msg_type=buffer[0]-'0'; 
				strcpy(msg.msg,buffer+1);
			
				if(msg.msg_type==1) {

					printf("Here is the msg : %s\n",msg.msg);
					char op[3*strlen(msg.msg)/4];
					decode(msg.msg,strlen(msg.msg),op); 

					// print the decode msg
					printf("Decoded op: %s\n",op);
					bzero(buffer,MSG_LEN);
					buffer[0]='2';

					// sending back the acknowledgement of the msg
					strcpy(buffer+1," Here is the Acknowledgement for your previous msg");
					n = write(sd,buffer,MSG_LEN); 
					
					if (n < 0){ 
						error("ERROR writing to socket");
					}

				}
				
				/**
				 * Type 3 connection is for closing the connection
				 * Finally close the socket and clear bitmap
				 * */
				else if(msg.msg_type==3){ 
					getpeername(sd , (struct sockaddr*)&cli_addr ,(socklen_t*)&cli_addr);
					printf("Host disconnected , ip %s , port %d \n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));                        
					close( sd );   
					client_socket[i] = 0;   
				}
			}    
		}
	} 


	/**
	 * free the memory of server buffer
	 */   
	free(buffer);
	return 0; 
}
