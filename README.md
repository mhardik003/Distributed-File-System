# DISTRIBUTED FILE SYSTEM

![](./images/output.gif)


Distributed File System by creating a nameserver which stores the mapping of the file path to the storage server and the storage server which stores the file and the client which can access the file from the storage server.
Welcome to our Distributed File System (DFS) project, a modern solution to data storage challenges. This project is an implementation of a DFS in C, designed to efficiently manage data across multiple storage servers. Despite the underlying complexity, it offers a seamless and unified view of the data, akin to interacting with a single server. This is achieved through an intelligent naming server that abstracts the distribution mechanics, providing a user-friendly interface.


## Key Features

- **Data Distribution** : Efficiently spreads data across various servers, enhancing storage capabilities and reliability.
- **Abstraction Layer** : The naming server masks the distributed nature, offering a simple and cohesive user experience.
- **Scalability** : Easily scales to accommodate growing data and user demands.
- 
- **Fault Tolerance** : Ensures data availability and consistency, even in the event of server failures. 

`(NOTE : The redundancy/backups part of the Project PDF isn't fully functional yet and might have some bugs, so please fix it/take care of it before using)`


## For more info

- To know more about the project refer to the [Project PDF](./Project.pdf) </a>

- To understand about how we have implemented the API calls and the server specifications refer to the [Working PDF](./Working.pdf) </a>

## Running the code

To run the code, run the following commands in different terminals followed by the instructions for Storage Server and the client

```
make naming_server
make storage_server
make client
```

You can find out the `IP address` of the naming server by running the following command (required for the Client and the Storage Server)

```
hostname -I
```

## File Structure

- ### Naming Server

  - **main.c** : Main goto file while compiling which calls the other files
  - **server_setup.h** : Has functions related to setting up the server and other server functions like sending and receiving messages etc.
  - **hashmap.h** : Contains functions for implementing the hashing algorithm for storing the file paths and the storage server details
  - **LRUCaching.h** : Contains functions for implementing the LRU Caching algorithm
  - **client_handler.h** : Contains functions for actively listening for connection requests from the client and sending them to the operation handler
  - **SS_handler.h** : Contains functions for actively listening for connection requests from the storage server and sending them to the operation handler
  - **operation_handler.h** : Contains functions which have all the APIs for all the operations from both the client and the storage server
  - **utils.h** : For logging, keeping the MACROS, global variables and other utility functions

- ### Storage Server

  - **main.c** : Main goto file while compiling which calls the other files
  - **server_setup.h** : Has functions related to setting up the server and other server functions like sending and receiving messages etc.
  - **get_accessible_paths.h** : Contains functions for getting the accessible paths from the user and sending them to the naming server
  - **client_handler.h** : Contains functions for actively listening for connection requests from the client and the naming server sending them to the operation handler
  - **operation_handler.h** : Contains functions which have all the APIs and in-server operations for all the operations from the storage server
  - **utils.h** : For keeping the MACROS, global variables

- ### Client
  - **main.c** : Main goto file while compiling which calls the other files
  - **server_setup.h** : Has functions related to setting up the server and other server functions like sending and receiving messages etc.
  - **operation_handler.h** : Contains functions which have all the APIs for all the operations from the client
  - **utils.h** : For keeping the MACROS, global variables

## Developers

- [Hardik Mittal](https://github.com/mhardik003)
- [Arjun Dosajh](https://github.com/ArjunDosajh)
- [Mihika Sanghi](https://github.com/mihikasanghi)

<br>
<br>
Feel free to raise issue contribute to the project.
