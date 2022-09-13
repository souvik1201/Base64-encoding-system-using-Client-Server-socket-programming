#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <unistd.h>            
#include <arpa/inet.h>
#include <string.h>  
#define MSG_LEN 256

char *ind="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

struct packet{                      // Message structure as define in question
	int msg_type;
	char msg[MSG_LEN];
};

int power(int a,int b){					//Use this function for our convince 
	int temp=1;
	while(b--){
		temp*=a;
	}
	return temp;
}

char *decode(char *input,char *encode){	
	int n=strlen(input);
	int i,k,p=0,l,c,temp,j=0;
	int binary[24];
	int val;
	int g = n/3;             // total number of groups are g+1

	for(k=0;k<g;k++){
		for(i=0;i<3;i++){ 
			c=input[3*k+i];
			j=0;
			while(j<=7){            // converting dec to bin
				temp=c%2;
				binary[8*i+7-j]=temp;
				c=c/2;
				j++;
			}
		}
		//Dividing string into 6 bit group and then represent the number by ind table character
		j=0;
		for(l=0;l<4;l++){
			val=0;
			for(j=0;j<6;j++){
				val+=binary[6*l+5-j]*power(2,j);
			}
			encode[p] = ind[val];
			p++;		
		}
	}
	//If size of our message string is in from of 3n+1 or 3n+2
	int rem1[12]={0};	//We have to add 4 zeros
	int rem2[18]={0};	//We have to add 2 zeros

	if(n%3==1){
		for(i=0;i<1;i++){	                                // When number of characters are 3n+1
			c=input[3*g+i];
			j=0;
			while(j<=7){            // converting dec to bin
				temp=c%2;
				rem1[8*i+7-j]=temp;
				c=c/2;
				j++;
			}
		}
		//Dividing string into 6 bit group and then represent the number by ind table character
		j=0;
		for(l=0;l<2;l++){
			val=0;
			for(j=0;j<6;j++){
				val+=rem1[6*l+5-j]*power(2,j);
			}
			encode[p] = ind[val];
			p++;		
		}
		encode[p]='=';
		p++;
		encode[p]='='; // Remaining 12 bits in the last group are filled with ==
		p++;
	}

	else if(n%3==2){                            // 3n+2
		for(i=0;i<2;i++){
			c=input[3*g+i];
			j=0;
			while(j<=7){            // converting dec to bin
				temp=c%2;
				rem2[8*i+7-j]=temp;
				c=c/2;
				j++;
			}
		}
		//Dividing string into 6 bit group and then represent the number by ind table character
		j=0;
		for(l=0;l<3;l++){
			val=0;
			for(j=0;j<6;j++){
				val+=rem2[6*l+5-j]*power(2,j);
			}
			
			encode[p] = ind[val];
			p++;		
		}	
		encode[p]='=';      // Last 6bits are filed with =
		p++;
	}
	encode[p] = '\0';
}



void error(char *msg){ // Helper error function
	perror(msg);
	exit(0);
}


int main(int argc, char *argv[]){
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char* buffer=(char *)malloc(MSG_LEN*sizeof(char));		//allocating buffer

	if(argc<3){						//If enter port number is not available
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portno=atoi(argv[2]);		//Extracting port number
	sockfd=socket(AF_INET, SOCK_STREAM, 0); //AF_INET for TCP connection
	
	if (sockfd < 0){			//Checking socket created or not
		error("Fail to opening socket");
	}
	
	server = gethostbyname(argv[1]);		//Get server address
	
	if (server == NULL) {				//User input server address or not
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));	//Filling in various fields of serv_addr
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);			//Extracting host port number
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){	//Initiate connection to server
		error("Fail to connect");
	}
	struct packet msg;
	char temp[MSG_LEN-1];
	bzero(buffer,MSG_LEN);
	n = read(sockfd,buffer,MSG_LEN);
	if (n < 0){					//If reading is not done then it return -1
		error("ERROR reading from socket");
	}

	printf("%s\n",buffer);

	while(1){
		bzero(buffer,MSG_LEN);
		printf("Please enter the message : \n");  
		fgets(buffer,MSG_LEN,stdin);
		buffer[strlen(buffer)-1]='\0';
		msg.msg_type=buffer[0] - '0'; //First char is msg type
		strcpy(msg.msg,buffer+1);

		if(msg.msg_type==1){
			char *to_send=(char*)malloc((sizeof(char)*MSG_LEN*4+6)/3);
			to_send[0]='1';
			decode(buffer+1,to_send+1);
			n = write(sockfd,to_send,strlen(to_send)); 
			free(to_send);
		
			if(n<0){			//If writing in socket is not done then it return -1
				error("ERROR writing to socket");  
			}	
		
			bzero(buffer,MSG_LEN);    
			n = read(sockfd,buffer,MSG_LEN);	
			printf("%s\n",buffer);
		}
		
		else if(msg.msg_type==3){       //Close connection
			n = write(sockfd,buffer,strlen(buffer));

			if (n < 0){
				error("Fail to writing to socket");
			}

			break;	
		}
		
		else{
			printf("Put 1 at begining og massage for massage send, if you want to close the connection then start with 3\n");
			continue;
		}
	}
	close(sockfd);				// Close socket
	free(buffer);				// Deallocating buffer space
	return 0;
}

