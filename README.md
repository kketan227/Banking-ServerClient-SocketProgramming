# Banking Server Client using Socket Programming

## Files explained  
- **server.cpp**
This is the Server side of the banking system...

This has a Main running Loop where in options are received from the user according to the role the
user has logged in with. This initially opens a listening port taken as an argument and on an oncoming
connection negotiates a port to which the client connects and the operations are carried. Because it 
wasn't mentioned in the problem statement and due to lack of time, concurrency wasn't implemented in 
this version of this project. It is one thing to implement in the future.
This Server runs infinitely listening for new connections whenever one client has disconnected until
we manually close the running program. This is done in a way to replicate the nature of a server 
always running and listening for connections.

*Roles:*
- C: Customer. Can check balance and Mini Statement (Last 10 Transactions). Has logout and exit options.
- A: Bank Admin. Can see a list of customers in the particular bank. Can do transactions for one customer 
	like debits and credits by paying and collecting cash. This user can also see the balance and mini 
	statement of the customer whose transactions are to be done. Has logout and exit options.
- P: Police. Can check the balance of all the customers in his bank area. Can also request mini statement
	for one particular customer. Has logout and exit options.
	
Has security implemented to prevent clients from changing role variables, etc to impersonate a 
different role.  

- **client.cpp**
This is the Client side of the banking system...

This has a Main running Loop where in options are given to the user according to the role the
user has logged in with. This initially connects to the server on the listening port and negotiates
a port and connects to the server after which the operations are carried. Because it wasn't mentioned 
in the problem statement and due to lack of time, concurrency wasn't implemented in this version of 
this project. It is one thing to implement in the future.

*Roles:*
- C: Customer. Can check balance and Mini Statement (Last 10 Transactions). Has logout and exit options.
- A: Bank Admin. Can see a list of customers in the particular bank. Can do transactions for one customer 
	like debits and credits by paying and collecting cash. This user can also see the balance and mini 
	statement of the customer whose transactions are to be done. Has logout and exit options.
- P: Police. Can check the balance of all the customers in his bank area. Can also request mini statement
	for one particular customer. Has logout and exit options.
	
Has security implemented to prevent clients from changing role variables, etc to impersonate a 
different role.  


_**Other detailed comments are given inline where necessary in each of the files**_

## To run the application
First run the server side and give a required port number for the server application to bind to and listen for incoming clients from the client side application. The txt files remain in the server side and should not be given access to the clients.   
The client then runs the application and given the required ip address of the server and the open listening port of the server to connect to from this client application. After which the application itself describes and gives all the required options and continues to work.  

## Predifined users
There are a few predefined users for demo purposes already in the login file. If required new users need to be added in that 
[login file](Login_file.txt)  

~~~
ketan	12345	A
cust	cust	C
police	police	P
cust2	cust2	C
cust3	cust3	C
cust4	cust4	C
cust5	cust5	C
cust7	cust7	C
cust8	cust8	C
cust9	cust9	C
cust10	cust10	C
admin	admin	A
~~~
