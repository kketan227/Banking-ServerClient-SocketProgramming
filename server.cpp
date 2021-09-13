/*
Assignment 3 Socket Programming
Client-Server Socket Oriented Banking System.


Author: Group 7: Ketan, Hemant, Himanshu, Himanshu
Roll Numbers: 204101030, 204101027, 204101028, 204101029

This is the Server side of the banking system...

This has a Main running Loop where in options are received from the user according to the role the
user has logged in with. This initially opens a listening port taken as an argument and on an oncoming
connection negotiates a port to which the client connects and the operations are carried. Because it 
wasn't mentioned in the problem statement and due to lack of time, concurrency wasn't implemented in 
this version of this project. It is one thing to implement in the future.
This Server runs infinitely listening for new connections whenever one client has disconnected until
we manually close the running program. This is done in a way to replicate the nature of a server 
always running and listening for connections.

Roles:
C: Customer. Can check balance and Mini Statement (Last 10 Transactions). Has logout and exit options.
A: Bank Admin. Can see a list of customers in the particular bank. Can do transactions for one customer 
	like debits and credits by paying and collecting cash. This user can also see the balance and mini 
	statement of the customer whose transactions are to be done. Has logout and exit options.
P: Police. Can check the balance of all the customers in his bank area. Can also request mini statement
	for one particular customer. Has logout and exit options.
	
Has security implemented to prevent clients from changing role variables, etc to impersonate a 
different role.

*/

#include <iostream>
#include <string>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <fstream>
#include <ctime>

using namespace std;
//Server side

bool VERBOSE = false; // Used for debugging.

#define BUFFER_SIZE 1024
char msg[1024];
string UID="noAUTH"; //No one is logged in.
string ROLE="X"; // X is unauth, then C A P
ifstream passFile;
fstream balanceFile;

void sendingMSG (int clientSD, string s){
	int num_packets = (s.length()-1)/BUFFER_SIZE +1;
	memset(&msg, 0, sizeof(msg)); //clear the buffer
    strcpy(msg, to_string(num_packets).c_str());
	int n = send(clientSD, (char*)&msg, sizeof(int), 0);
	
	for(int i =0; i<num_packets; i++){
		string mess=s.substr((i*BUFFER_SIZE), min((i+1)*BUFFER_SIZE,(int)s.length()));
		if (VERBOSE) cout<<"In sendingMSG() mess is "<<mess<<endl;
		memset(&msg, 0, sizeof(msg)); //clear the buffer
    	strcpy(msg, mess.c_str());
    	send(clientSD, (char*)&msg, strlen(msg), 0);

	}	
}
string receiveMSG(int clientSD) {
	int num_packets = 0;
	memset(&msg, 0, sizeof(msg));//clear the buffer
    recv(clientSD, (char*)&msg, sizeof(int), 0);
	num_packets = atoi(msg);
	if (VERBOSE) cout <<"In receiveMSG() num_packets is "<<num_packets<<endl;

	string message = "";
	for(int i = 0; i < num_packets; i++) {
		memset(&msg, 0, sizeof(msg));//clear the buffer
        recv(clientSD, (char*)&msg, sizeof(msg), 0);
        //cout<<"msg is"<<msg<<endl;
		message = message+msg;
	}
	return message;
}
string Login (int clientSD){
	cout << "Awaiting client Login Details..." << endl;
        
	sendingMSG(clientSD,"Please enter Username");
	string user=receiveMSG(clientSD);
	cout<<"Username is : "<< user<<endl;
	sendingMSG(clientSD,"Please enter your password");
	string passwd=receiveMSG(clientSD);
	cout<<"Password is : "<<passwd<<endl;
	passFile.open("Login_file.txt");
	if (!passFile) {
		cout << "Passwords file not created."<<endl;
	}
	else {
		char buf[512];
		memset(buf, 0, sizeof(buf));			// initial clearning of buffer
		string temp;

		while(getline(passFile, temp))
		{	
			for(int i=0; i<temp.length();i++)
				buf[i]=temp.at(i);
			char *token = strtok(buf, "\t");			// get the tokens separated by delimiter 'tab'
			while( token != NULL ) 
			{
				string c_in = token;
				if(c_in.compare(user)==0){
					token = strtok(NULL, "\t");
					c_in = token;
					if(c_in.compare(passwd)==0){
						token = strtok(NULL, "\t");
						c_in = token;
						c_in = c_in.at(0);
						sendingMSG(clientSD,c_in);
	      				ROLE=c_in;
	      				passFile.close();
	      				return user;
	      				break;
					}
					else{
						sendingMSG(clientSD,"X");
						passFile.close();
						return "noAUTH";
					}
				}
				token = strtok(NULL, "\t");			// iterates to the next token
			}
		}
	}
	sendingMSG(clientSD,"NOUSER"); // User only not found
	passFile.close();
	return "noAUTH";
}

void logout(int clientSD){
	UID="noAUTH"; 
	ROLE="X";
}

string getBalance(string CustID){
	string Balance ="";
	balanceFile.open(CustID+"_account.txt");
	if (!balanceFile) {
		cout << "Account file for "<<CustID<<" not found."<<endl;
		return "NO CUSTOMER";
	}
	else {
		string line, buffer[10];
      	const size_t size = sizeof buffer / sizeof *buffer;
		int i = 0;
		while ( getline(balanceFile, line) ){
	    	buffer[i] = line;
	    	if ( ++i >= size ){
	    		i = 0;
	    	}
	   }		
		string temp = buffer [i-1];
		string delimiter = "\t";
		
		size_t pos = 0;
		string token;
		while ((pos = temp.find(delimiter)) != string::npos) {
		    token = temp.substr(0, pos);
		    temp.erase(0, pos + delimiter.length());
		}
		Balance=temp;
	}
	balanceFile.close();
	return Balance;

}
string getMini(string CustID){
	cout<<"Mini Statement for Customer "<<CustID<<" requested."<<endl;
	string statement ="The current balance for the account is "+getBalance(CustID)+"\nAccount Mini Statement in Latest to Old order\nTrnx Date\tCredit/Debit\tClosing Balance\n";
	balanceFile.open(CustID+"_account.txt");
	if (!balanceFile) {
		cout << "Account file for "<<CustID<<" not found."<<endl;
		statement+="*** Account file for "+CustID+" not found. ***";
	}
	else {
		string line, buffer[10];
      	const size_t size = sizeof buffer / sizeof *buffer;
		int i = 0;
		while ( getline(balanceFile, line) ){
	    	buffer[i] = line;
	    	if ( ++i >= size ){
	        	i = 0;
	    	}
		}
		for(int j=0;j<10;j++){
			//cout<<(i-1-j+10)%10<<endl;
			if(buffer[(i-1-j+10)%10].compare("")==0) continue;
			statement+=buffer[(i-1-j+10)%10]+"\n";
		}
		statement+="*** END OF STATEMENT ***";
	}
	balanceFile.close();
	return statement;
}

string createListOfCustomers(){
	ifstream usersfile;
	usersfile.open("Login_file.txt");
	string users="";
	string line;
	while(getline(usersfile,line)){
		string temp = line;
		string delimiter = "\t";
		size_t pos = 0;
		string token;
		pos=temp.find(delimiter);
		string name= temp.substr(0,pos);
		while ((pos = temp.find(delimiter)) != string::npos) {
		    token = temp.substr(0, pos);
		    temp.erase(0, pos + delimiter.length());
		}
		if (VERBOSE) cout <<"In createListOfCustomers Temp val "<< temp << endl;
		string comp=""+temp.substr(0,1);
		//cout<<"Comp val "<<comp<<endl;
		if(comp.compare("C")==0){
			users+=name+",";
		}
	}
	return users;
}

string allBalance(string userslist){
	string Result="";
	string temp = userslist;
	string delimiter = ",";
	size_t pos = 0;
	string token;
	while ((pos = temp.find(delimiter)) != string::npos) {
	    token = temp.substr(0, pos);
	    Result+=token+"\'s\t\t Balance is \tRs."+getBalance(token)+"\n";
	    temp.erase(0, pos + delimiter.length());
	}
	return Result;
}


int main(int argc, char *argv[])
{
    //for the server, we only need to specify a port number
    if(argc != 3-1){
        cerr << "Usage: "<<argv[0]<<" port_number" << endl;
        exit(0);
    }
    //grab the port number
    int port = atoi(argv[1]);
    //buffer to send and receive messages with
    int opted=1;
     
    //setup a socket and connection tools
    sockaddr_in servAddr;
    bzero((char*)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); // To accept connections to all the addresses linked to this machine.
    servAddr.sin_port = htons(port);
 
    //Open a TCP stream socket and keep track of its descriptor
    int serverSd = socket(AF_INET, SOCK_STREAM, 0); //socket(domain, type, protocol) STREAM means TCP
    if(serverSd < 0){
        cerr << "Error establishing the server socket" << endl;
        exit(0);
    }
    //setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &opted, sizeof ( opted )); //Options for the socket.
    
    //bind the socket to its local address
    int bindStatus = bind(serverSd, (struct sockaddr*) &servAddr, 
        sizeof(servAddr));
    if(bindStatus < 0){
        cerr << "Error binding socket to local address" << endl;
        exit(0);
    }
    
    cout << " **** Group 7 Banking CORP. ****"<<"\nThe Server is up and waiting for clients to connect." << endl;
    while(1){
	    //listen for up to 1 requests at a time
	    listen(serverSd, 1);
	    //receive a request from client using accept
	    //we need a new address to connect with the client
	    sockaddr_in newSockAddr;
	    socklen_t newSockAddrSize = sizeof(newSockAddr);
	    //accept, create a new socket descriptor to 
	    //handle the new connection with client
	    int newSd = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
	    if(newSd < 0){
	        cerr << "Error accepting request from client!" << endl;
	        exit(1);
	    }
	    cout << "Connected with a client!" << endl;
	    
	    while(1){
	        //receive a message from the client (listen)
	        while(true && ROLE.compare("X")==0){
	        	string response=Login(newSd);
	        	cout<<"Response here is "<<response<<endl;
	        	if(response.compare("noAUTH")!=0){
	        		UID=response;
	        		cout<<"UID IS "<<UID<<'\t';
	        		cout<<"ROLE IS "<<ROLE<<endl;
					break;
				} 
				else{
					cout<<"Wrong creds..."<<endl;
					
				}
			}
	        
	        /*
	        CUSTOMER ROLE
	        */
	        if(ROLE.compare("C")==0){
	        	cout<<"Customer logged in so sending balance."<<endl;
	        	string Balance = getBalance(UID);
	        	sendingMSG(newSd,"Your account balance is Rs."+Balance+"/-");
	        	bool exithua=false;
	        	
	        	int loopcheck=0;
	        	while(1){
	        		loopcheck++;
	        		//TRANSACTION MENU
					string message=receiveMSG(newSd);
					if(message.compare("exit")==0){
	       	 			exithua=true;
			            cout << "Client has quit the session\nWaiting for a new client to connect." << endl;
			            UID="noAUTH"; //No one is logged in.
						ROLE="X"; // X is unauth, then C A P
			            close(newSd);
			            loopcheck--;
			            break;
			        }
			        if(message.compare("L")==0){
			        	cout<<"Client has logged out."<<endl;
			        	logout(newSd);
			        	loopcheck--;
						break;
					}
					if(message.compare("M")==0){
						string statement=getMini(UID);
						sendingMSG(newSd,statement);
						loopcheck--;
					}
					if(message.compare("B")==0){
						string Balance = getBalance(UID);
	        			sendingMSG(newSd,UID+", Your account balance is Rs."+Balance+"/-");
	        			loopcheck--;
					}
					if(loopcheck>2){
						cout<<"breaking a loop because of infinite running and calling an exit on client.\nWaiting for a new client now..."<<endl;
						exithua=true;
						UID="noAUTH"; //No one is logged in.
						ROLE="X"; // X is unauth, then C A P
			            close(newSd);
						break;	
					}
					
				}
				if(exithua){
					exithua=false;
					break;
				}
			}
	        	
	        /*
			BANK ADMIN ROLE
			*/
			if(ROLE.compare("A")==0){
				cout<<"Bank Admin logged in... No special welcome message. List of all the customers."<<endl;
				string custList=createListOfCustomers();
				cout<<custList<<endl;
				sendingMSG(newSd,"The list of all the customers in your bank:\n"+custList.substr(0,custList.length()-1));
				bool exithua=false;
	        	
	        	while(1){
	        		//TRANSACTION MENU
					string message=receiveMSG(newSd);
					if(message.compare("exit")==0){
	       	 			exithua=true;
			            cout << "Client(Admin) has quit the session\nWaiting for a new client to connect." << endl;
			            UID="noAUTH"; //No one is logged in.
						ROLE="X"; // X is unauth, then C A P
			            close(newSd);
			            break;
			        }
			        if(message.compare("L")==0){
			        	cout<<"Client has logged out."<<endl;
			        	logout(newSd);
						break;
					}
					if(message.compare("C")==0){
						cout<<"Cust List requested."<<endl;
						sendingMSG(newSd,"The list of all the customers in your bank:\n"+custList.substr(0,custList.length()-1));
					}
					if(message.compare("T")==0){
						string UserID = receiveMSG(newSd);
						cout<<"Doing transactions for "<<UserID<<endl;
						string Balance = getBalance(UserID);
						if(Balance=="NO CUSTOMER"){
							sendingMSG(newSd,"NO CUSTOMER");
							continue;
						}
	        			sendingMSG(newSd,"Available balance in "+UserID+"\'s account is Rs."+Balance+"/-");
	        			int loopcheck=0;
	        			while(1){
	        				loopcheck++;
	        				string command=receiveMSG(newSd);
	        				command=command.substr(0,1);
	        				cout<<"the command sent out is:"<<command<<"<- Just this"<<endl;
	        				if(command.compare("M")==0){
	        					string blah = receiveMSG(newSd);
								string statement=getMini(UserID);
								cout<<statement<<endl;
								cout<<"Sending Mini Statement for "<<UserID<<endl; 
								sendingMSG(newSd,statement);
								loopcheck--;
							}
							if(command.compare("S")==0){
								cout<<"Coming out of "<<UserID<<"\'s account."<<endl;
								loopcheck--;
								break;
							}
							if(command.compare("C")==0){
								string amount=receiveMSG(newSd);
								cout<<"The amount for credit is "<<amount<<" and the int version of it is... "<<stoi(amount)<<endl;
								string writingmsg="";
								time_t now = time(0);
								tm *ltm = localtime(&now);
								int bal=stoi(getBalance(UserID))+stoi(amount);
								writingmsg+=bal;
								ofstream outfile;
								outfile.open(UserID+"_account.txt", ios_base::app);
								outfile<<"\n";
								outfile<<ltm->tm_mday;
								outfile<<'/';
								outfile<<(ltm->tm_mon)+1;
								outfile<<'/';
								outfile<<(ltm->tm_year)+1900;
								outfile<<"\tCredit\t+";
								outfile<<amount;
								outfile<<"\t";
								if (VERBOSE) cout<<"In Credit Switch Case Writing this -> "<<writingmsg<<endl;
								cout<<"Balance now is "<<bal<<endl;
								outfile<<bal;
								outfile.close();
								sendingMSG(newSd,to_string(bal));
								loopcheck--;
								
							}
							if(command.compare("D")==0){
								while(1){
									string amount=receiveMSG(newSd);
									cout<<"The amount for debit is "<<amount<<" and the int version of it is... "<<stoi(amount)<<endl;
									if(stoi(amount)<=stoi(getBalance(UserID))){
										string writingmsg="";
										time_t now = time(0);
										tm *ltm = localtime(&now);
										int bal=stoi(getBalance(UserID))-stoi(amount);
										writingmsg+=bal;
										ofstream outfile;
										outfile.open(UserID+"_account.txt", ios_base::app);
										outfile<<"\n";
										outfile<<ltm->tm_mday;
										outfile<<'/';
										outfile<<(ltm->tm_mon)+1;
										outfile<<'/';
										outfile<<(ltm->tm_year)+1900;
										outfile<<"\tDebit\t-";
										outfile<<amount;
										outfile<<"\t";
										if (VERBOSE) cout<<"In Debit Switch Case Writing this -> "<<writingmsg<<endl;
										cout<<"Balance now is "<<bal<<endl;
										outfile<<bal;
										outfile.close();
										sendingMSG(newSd,to_string(bal));
										break;
									}
									if(stoi(amount)>stoi(getBalance(UserID))){
										sendingMSG(newSd,"NOFUNDS");
										continue;
									}
									loopcheck--;
								}		
							}
							if(loopcheck>2){
								cout<<"breaking a loop because of infinite running and calling an exit on client.\nWaiting for a new client now..."<<endl;
								exithua=true;
								UID="noAUTH"; //No one is logged in.
								ROLE="X"; // X is unauth, then C A P
					            close(newSd);
								break;	
							}
						}

					}
					if(message.compare("B")==0){
						sendingMSG(newSd,allBalance(createListOfCustomers()));
					}
					if(exithua){
					break;
					}
					
				}
				if(exithua){
					exithua=false;
					break;
				}
			}
				
			/*
			POLICE ROLE
			*/	
			if(ROLE.compare("P")==0){
				cout<<"Police logged in so sending all accounts balance."<<endl;
				string custList=createListOfCustomers();
				cout<<custList<<endl;
				string MessageToSend=allBalance(custList);
				sendingMSG(newSd,MessageToSend);
				bool exithua=false;
	        	
	        	int loopcheck=0;
	        	while(1){
	        		loopcheck++;
	        		//TRANSACTION MENU
					string message=receiveMSG(newSd);
					if(message.compare("exit")==0){
						exithua=true;
			            cout << "Client(Police) has quit the session\nWaiting for a new client to connect." << endl;
			            UID="noAUTH"; //No one is logged in.
						ROLE="X"; // X is unauth, then C A P
			            close(newSd);
			            loopcheck--;
			            break;
			        }
			        if(message.compare("L")==0){
			        	cout<<"Client has logged out."<<endl;
			        	logout(newSd);
			        	loopcheck--;
						break;
					}
					if(message.compare("M")==0){
						string UserID = receiveMSG(newSd);
						if(getBalance(UserID)=="NO CUSTOMER"){
							sendingMSG(newSd,"NO CUSTOMER");
						}
						else{
							string statement=getMini(UserID);
							sendingMSG(newSd,statement);
						}
						loopcheck--;
					}
					if(message.compare("B")==0){
						sendingMSG(newSd,allBalance(createListOfCustomers()));
						loopcheck--;
					}
					if(loopcheck>2){
						cout<<"breaking a loop because of infinite running and calling an exit on client.\nWaiting for a new client now..."<<endl;
						exithua=true;
						UID="noAUTH"; //No one is logged in.
						ROLE="X"; // X is unauth, then C A P
			            close(newSd);
						break;	
					}
					
				}
				if(exithua){
					exithua=false;
					break;
				}
			}			
		}
	    close(newSd);
    }

    //we need to close the socket descriptors after we're all done
    close(serverSd);
    cout << "Connection closed..." << endl;
    return 0;   
}
