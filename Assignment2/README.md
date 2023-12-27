# Operating Systems
3-1, Operating Systems Assignment, We have to simulate an application for a distributed graph database system involving a load
balancer process, a primary server process, two secondary server processes (secondary server 1 and secondary
server 2), a cleanup process and several clients.

## The overall workflow is as follows

- Clients send requests (read or write) to the load balancer.

- Load balancer forwards the write requests to the primary server.

- Load balancer forwards the odd numbered read requests to secondary server 1 and the even
numbered read requests to secondary server 2. Request numbering starts from 1.

- On receiving a request from the load balancer, the server (primary or secondary) creates a new thread
to process the request. Once the processing is complete, this thread (not the server process) sends a
message or an output back to the client.

- For terminating the application, a relevant input (explained later) needs to be given to the cleanup
process. The cleanup process informs the load balancer. The load balancer performs the relevant
cleanup activity, informs the servers to exit and terminates itself. On receiving the intimation from the
load balancer, the servers perform the relevant cleanup activities and terminate.


<div align="center">
<img width="675" alt="OS_Graph_DB" src="https://github.com/pavas23/CS-F372/assets/97559428/09b0adeb-6e1a-4dc4-a8f8-da82d66fac5b">
</div>


## GRAPH DATABASE

- The database system can contain both cyclic and acyclic graphs. The graphs are
unweighted and undirected in nature and may contain self loops. Each graph is represented as a text
(ASCII) file. The naming convention of each file is Gx.txt where x is number (like G1.txt or
G2.txt).
- The contents of each file are in the following format:
  - The first line contains a single integer n denoting the number of nodes present in the graph
  and n ≤ 30.

  - The successive n lines represent the n rows of the adjacency matrix. The consecutive elements
  of a single row are separated by a whitespace character. The graph nodes are labeled as 1,
  2, 3, …, n. Thus, the first of the n lines corresponds to node 1, the second of the n lines
  corresponds to node 2 and so on.


<div align="center">
<img width="482" alt="OS_Graph" src="https://github.com/pavas23/CS-F372/assets/97559428/fb510acc-3cdb-482c-a6eb-0fad1e24ff48">
</div>

- You can assume that the graph database system contains a maximum of 20 files. The database allows
the clients to perform various read and write operations on the graphs. The write operations are
facilitated by the primary server process and the read operations are performed by the secondary
server processes, secondary server 1 and secondary server 2.

## CLIENT PROCESS
- The clients send the requests to a load balancer process via a single message queue
(message queue to be created by the load balancer only). Note that the clients do not send the requests 
to the servers directly. You are not allowed to use more than one message queue in the entire
assignment implementation.

- Each client displays the following menu options.
  1. Add a new graph to the database
  2. Modify an existing graph of the database
  3. Perform DFS on an existing graph of the database
  4. Perform BFS on an existing graph of the database

- Options 1 and 2 are write operations (to be performed by the primary server) and options 3 and 4 are
read operations (to be performed by the secondary servers). For option 2, addition and/or deletion of
nodes and/or edges can be requested by the client. Each client uses the following 3-tuple format for
sending the request to the load balancer via the message queue: <Sequence_Number Operation_Number Graph_File_Name>.

- Sequence_Number will start from 1 and will keep on increasing monotonically till 100 for all the client requests sent across all the servers. This
Sequence_Number corresponds to the request number mentioned earlier. It is guaranteed that the
Sequence_Number associated with each client request will be unique.
- Operation_Number will be specified as per the menu options mentioned above. The client prompts the user to enter the
Sequence_Number, Operation_Number and Graph_File_Name by displaying a prompt as
follows:

  - Enter Sequence Number
  - Enter Operation Number
  - Enter Graph File Name

- Graph_File_Name will be a new file name for option 1 and an existing file name for the remaining
options. You can assume that the client will not deliberately send an existing file name for option 1 and
a non-existent file name for options 2, 3 and 4. Additionally, for a write operation, the client after
sending the 3-tuple request to the load balancer, writes the number of nodes of the graph (new or the
modified one) and the corresponding adjacency matrix to a shared memory segment. The client
prompts the user to enter the information to be written to the shared memory segment by displaying
the following prompt:

  - Enter number of nodes of the graph
  - Enter adjacency matrix, each row on a separate line and elements of a single row separated by whitespace characters

- For a read operation, the client specifies the starting vertex for the BFS/DFS traversal. The client writes
this information in a shared memory segment. The client prompts the user to specify this starting
vertex by displaying the following message: Enter starting vertex. Each client can create a
separate shared memory segment for every new request. Note that only the client should create the 
shared memory segment.

- For every request, the client receives some sort of message or output from the server (actually it will
be a thread created by the server as explained later) once the request has been serviced via the single
message queue. Once this is received, the client deletes the shared memory segment.

