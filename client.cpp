/*
Assignment 3 Socket Programming
Client Server Socket Oriented Banking System.


Author: Group 7: Ketan, Hemant, Himanshu, Himanshu
Roll Numbers: 204101030, 204101027, 204101028, 204101029

This is the Client side of the banking system...

This has a Main running Loop where in options are given to the user according to the role the
user has logged in with. This initially connects to the server on the listening port and negotiates
a port and connects to the server after which the operations are carried. Because it wasn't mentioned 
in the problem statement and due to lack of time, concurrency wasn't implemented in this version of 
this project. It is one thing to implement in the future.

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
#include <iterator> 
#include <map> 
using namespace std;
//Client side

bool VERBOSE = false;

#define BUFFER_SIZE 1024
char msg[1024];
string user = "noAuth";
string ROLE = "X"; //X is unauth rest is C A P SAved on both client and server so no altering these...
map <string,string> rolemap={{"A", "Bank Admin"}, {"C", "Customer"}, {"P", "Police"}};


void sendingMSG (int clientSD, string s){
	if(VERBOSE) cout<<"In sendingMSG () the messge read is "<<s<<endl;
	int num_packets = (s.length()-1)/BUFFER_SIZE +1;
	memset(&msg, 0, sizeof(msg)); //clear the buffer
    strcpy(msg, to_string(num_packets).c_str());
    if(VERBOSE) cout<< "In sendingMSG() num_packets: "<<num_packets<<endl;
	int n = send(clientSD, (char*)&msg, sizeof(int), 0);
	
	for(int i =0; i<num_packets; i++){
		//cout<< "Min of" <<(i+1)*BUFFER_SIZE+1<<" and "<<(int)s.length()<<" is "<<min((i+1)*BUFFER_SIZE+1,(int)s.length())<<endl;
		string mess=s.substr((i*BUFFER_SIZE), min((i+1)*BUFFER_SIZE,(int)s.length()));
		if(VERBOSE) cout<<"In sendingMSG() loop mess is "<<mess<<endl;
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
	
	string message = "";
	int i;
	for(i = 0; i < num_packets; i++) {
		memset(&msg, 0, sizeof(msg));//clear the buffer
        recv(clientSD, (char*)&msg, sizeof(msg), 0);
		message = message+msg;
	}
	return message;
}
string login(int clientSD){
	string resp = receiveMSG(clientSD);
	cout<<"Server: "<<resp<<endl<<">";
	string data;
    getline(cin, data);
    sendingMSG(clientSD,data);
    string tempUser= data;
    resp = receiveMSG(clientSD);
	cout<<"Server: "<<resp<<endl<<">";
	getline(cin, data);
    sendingMSG(clientSD,data);
    resp = receiveMSG(clientSD);
	if(resp.compare("X")==0){
		cout<<"*** Wrong password. Please try again... ***"<<endl;
		user="noAuth";
		ROLE="X";
	}
	else if(resp.compare("NOUSER")==0){
		cout<<"*** The User \""<<tempUser<<"\" not found. ***"<<endl;
		user="noAuth";
		ROLE="X";
	}
	else{
		user=tempUser;
	}
    return resp;
	
}

int main(int argc, char *argv[])
{
    //we need 2 things: ip address and port number, in that order
    if(argc != 4-1)
    {
        cerr << "Usage: "<<argv[0]<<" ip_address port" << endl; exit(0); 
    } //grab the IP address and port number 
    char *serverIp = argv[1]; int port = atoi(argv[2]); 
    //setup a socket and connection tools 
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr;   
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
    inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    sendSockAddr.sin_port = htons(port);
    int clientSd = socket(AF_INET, SOCK_STREAM, 0);
    //try to connect...
    int status = connect(clientSd,
                         (sockaddr*) &sendSockAddr, sizeof(sendSockAddr));
    if(status < 0)
    {
        cout<<"Error connecting to socket!"<<endl;
        exit(2);
    }
    cout << "Connected to the server!" << endl;
    
    //Once connected client side operations
	while(1)
    {
    	//Login loop when there is no user logged in
    	while(true && ROLE.compare("X")==0){
    		string reply=login(clientSd);
    		if (reply.compare("X")!=0 && reply.compare("NOUSER")!=0){
    			//If some ROLE is set, break the login loop.
				ROLE=reply;
    			break;
			}
		}
		// For additional security this role is stored even in the server side so this variable isn't changed to 
		// appear like another role.
		
		auto itr=rolemap.find(ROLE);
		if(VERBOSE) cout<<"After login loop. Role vairable is "<<ROLE<<"ok?"<<endl;
		cout<<"Hello "<<user<<", Welcome back. Your role is "<<itr->second<<endl;
		string welcomeMSG=receiveMSG(clientSd);
		cout<<welcomeMSG<<endl;
		// By now the role is understood and now we will run the client side according to the role.
		
		
		// CUSTOMER's Role options
		if(ROLE.compare("C")==0){
			cout<<"Options:\nM: Mini Statement.\nB: Account Balance\n"
				<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
			while(1){
				//transactions loop
				cout<<"Enter an option.\n>";
				string data;
	    		getline(cin, data);
	    		if(data=="exit" || data =="x" || data== "X"){
	    			sendingMSG(clientSd,"exit");
	    			cout<<"Goodbye. Visit Again."<<endl;
	    			exit(0);//Because I am in multiple loops
				}
				else if(data=="l" || data=="L"){
					sendingMSG(clientSd,"L");
					cout<<"You have been logged out. Please login again."<<endl;
					user = "noAuth";
					ROLE = "X";
					break; // Transactions loop is broken and we go into the login loop because ROLE is reset.	
				}
				else if(data=="m" || data=="M"){
					sendingMSG(clientSd,"M");
					cout<<"Server:\n"<<receiveMSG(clientSd)<<endl;
				}
				else if(data=="b" || data=="B"){
					sendingMSG(clientSd,"B");
					cout<<"Server:\n"<<receiveMSG(clientSd)<<endl;
				}
				else{
					cout<<"Invalid Option... Please look at this\nOptions:\nM: Mini Statement.\nB: Account Balance\n"
						<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
				}
				
			}
		}
		
		// Bank Admin's ROLE options
		if(ROLE.compare("A")==0){
			cout<<"Options:\nC: List all the customers in your bank.\nT: To make a transaction for a particular customer.\n"
				<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
			while(1){
				//transactions loop
				cout<<"Enter an option.\n>";
				string data;
	    		getline(cin, data);
	    		if(data=="exit" || data =="x" || data== "X"){
	    			sendingMSG(clientSd,"exit");
	    			cout<<"Goodbye. Visit Again."<<endl;
	    			close(clientSd);
	    			exit(0);//Because I am in multiple loops
				}
				else if(data=="l" || data=="L"){
					sendingMSG(clientSd,"L");
					cout<<"You have been logged out. Please login again."<<endl;
					user = "noAuth";
					ROLE = "X";
					break;	// Out of transactions loop into login loop
				}
				else if(data=="T" || data=="t"){
					sendingMSG(clientSd,"T");
					cout<<"Type the userID of the user whose transaction you want to do."<<endl;
					string data; 
					getline(cin,data);
					sendingMSG(clientSd,data);
					string currentUser = data;
					string reply = receiveMSG(clientSd);
					if(reply=="NO CUSTOMER"){
						cout<<"No such customer "<<data<<" found... Taking you back to main menu."<<endl;
						cout<<"Options:\nC: List all the customers in your bank.\nT: To make a transaction for a particular customer.\n"
						<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
						continue;
					}
					cout<<"Server:\n"<<reply<<endl;
					while(1){
						cout<<"M: Get the mini statement for this account.\nC: Credit this account\nD: Debit this account\nS: Switch out of this customer and go back to main menu\n";
						cout<<"Input option\n>";
						string data; 
						getline(cin,data);
						if(data=="m" || data == "M"){
							sendingMSG(clientSd,"M");
							sendingMSG(clientSd,currentUser);
							cout<<"Server:\n"<<receiveMSG(clientSd)<<endl;
							continue;
						}
						if(data=="c" || data == "C"){
							sendingMSG(clientSd,"C");
							cout<<"Enter how much money to credit to "<<currentUser<<"\'s account. \nYou can use commas also.\n>";
							string data;
							getline(cin,data);
							// Cleaning for comma's just in case
								string ResultAmt="";
								string temp = data;
								string delimiter = ",";
								size_t pos = 0;
								string token;
								while ((pos = temp.find(delimiter)) != string::npos) {
								    token = temp.substr(0, pos);
								    ResultAmt+=token;
								    //cout << token << endl;
								    temp.erase(0, pos + delimiter.length());
								}
								//cout << temp << endl;
								ResultAmt+=temp;
								if(VERBOSE) cout<<"In Credit switch case... result amount is "<<ResultAmt<<endl;
							//sendingMSG(clientSd,data);
							sendingMSG(clientSd,ResultAmt);
							string currentBal=receiveMSG(clientSd);
							cout<<"*** The current Balance is "<<currentBal<<". Please remember to collect the amount "<<data<<" from the customer "<<currentUser<<" ***"<<endl;
							continue;
						}
						if(data == "s" || data == "S"){
							sendingMSG(clientSd,"S");
							cout<<"Coming out of "<<currentUser<<"\'s account."<<endl;
							currentUser="";
							cout<<"Options:\nC: List all the customers in your bank.\nT: To make a transaction for a particular customer.\n"
								<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
							break;
						}
						if(data=="d" || data == "D"){
							sendingMSG(clientSd,"D");
							while(1){
								cout<<"Enter how much money to Debit from "<<currentUser<<"\'s account. \nYou can use COMMAS for convenience.\n>";
								string data;
								getline(cin,data);
								// Cleaning for comma's just in case
									string ResultAmt="";
									string temp = data;
									string delimiter = ",";
									size_t pos = 0;
									string token;
									while ((pos = temp.find(delimiter)) != string::npos) {
									    token = temp.substr(0, pos);
									    ResultAmt+=token;
									    //cout << token << endl;
									    temp.erase(0, pos + delimiter.length());
									}
									//cout << temp << endl;
									ResultAmt+=temp;
									if(VERBOSE) cout<<"In debit function... result amount is "<<ResultAmt<<endl;
								//sendingMSG(clientSd,data);
								sendingMSG(clientSd,ResultAmt);
							
								string currentBal=receiveMSG(clientSd);
								if(currentBal=="NOFUNDS"){
									cout<<"*** The account balance is not sufficient to withdraw so much amount. Please check again. ***"<<endl;
									continue;
								}
								else{
									cout<<"*** The current Balance is "<<currentBal<<". Please remember to give the amount "<<data<<" to the customer "<<currentUser<<" ***"<<endl;
									break;
								}
							}
							continue;
						}
						cout<<"*** Invalid Option ***"<<endl;
					}
					
				}
				else if(data=="c" || data=="C"){
					sendingMSG(clientSd,"C");
					cout<<"Server:\n"<<receiveMSG(clientSd)<<endl;
				}
				else{
					cout<<"Invalid Option... Please look at this\nOptions:\nC: List all the customers in your bank.\nT: To make a transaction for a particular customer.\n"
						<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
				}	
			}	
		}
		
		
		// Police's role options
		if(ROLE.compare("P")==0){
			cout<<"Options:\nM: Mini Statement of a particular user.\nB: List all Accounts Balance\n"
				<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
			while(1){
				//transactions loop
				cout<<"Enter an option.\n>";
				string data;
	    		getline(cin, data);
	    		if(data=="exit" || data =="x" || data== "X"){
	    			sendingMSG(clientSd,"exit");
	    			cout<<"Goodbye. Visit Again."<<endl;
	    			close(clientSd);
	    			exit(0);//Because I am in multiple loops
				}
				else if(data=="l" || data=="L"){
					sendingMSG(clientSd,"L");
					cout<<"You have been logged out. Please login again."<<endl;
					user = "noAuth";
					ROLE = "X";
					break;	
				}
				else if(data=="m" || data=="M"){
					sendingMSG(clientSd,"M");
					cout<<"Type the userID of the user whose statement you want to see"<<endl;
					string data; 
					getline(cin,data);
					sendingMSG(clientSd,data);
					string reply = receiveMSG(clientSd);
					if(reply=="NO CUSTOMER"){
						cout<<"No such customer "<<data<<" found... Taking you back to main menu."<<endl;
						cout<<"Options:\nM: Mini Statement of a particular user.\nB: List all Accounts Balance\n"
							<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
						continue;
					}
					cout<<"Server:\n"<<reply<<endl;
				}
				else if(data=="b" || data=="B"){
					sendingMSG(clientSd,"B");
					cout<<"Server:\n"<<receiveMSG(clientSd)<<endl;
				}
				else{
					cout<<"Invalid Option... Please look at this\nOptions:\nM: Mini Statement of a particular user.\nB: List all Accounts Balance\n"
						<<"L:Logout to reuse connection to login as another role.\nX:Exit the connection and close the client.\n";
				}
				
			}
		}
    }
    
    // Gracefully close the connection in case the execution reaches here....
    // The flow doesn't reach here in 99% times.
    close(clientSd);
    cout << "Connection closed" << endl;
    return 0;    
}
