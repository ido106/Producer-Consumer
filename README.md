# General Description

The purpose of this assignment is to gain experience with concurrent programming and synchronization mechanisms.  
The scenario is also intended to give a feel for the performance impact of synchronization constructs under different loads.  
The overall scenario we are simulating is that of news broadcasting. Different types of stories are produced and the system sorts them and displays them to the public.  
In this assignment, the ‘new stories’ are simulated by simple strings which should be displayed to the screen in the order they arrive.  
In the scenario that you should implement, there are 4 types of active actors: Producer, Dispatcher, Co-Editors and Screen Manager.  
<img src="https://user-images.githubusercontent.com/92651125/186007234-86c36a37-732a-4618-bd4e-86351ccbf87f.png" width="800">

## Producer

Each producer creates a number of strings in the following format (for example):  `Producer 2 SPORTS 0`   
Where ‘2’ is the producers ID, ‘SPORTS' is a random type it chooses, and ‘0’ is the number of articles of that type this producer has already produced.  
Each of the producers passes its information to the Dispatcher (introduced below) via its own private queue.  
Each of the Producers private queue is shared between the Producer and the Dispatcher.  

## Dispatcher

The Dispatcher continuously accepts messages from the Producers queues. It scans the Producers queue using a **Round Robin algorithm**. The Dispatcher does not block when the queues are empty. Each message is "sorted" by the Dispatcher and inserted to a one of the Dispatcher queues which includes strings of a single type. 

## Co-Editors

For each type of possible messages there is a Co-Editor that receives the message through the Dispatchers queue, "edits" it, and passes it to the screen manager via a single shared queue. The editing process will be simulated by the Co-Editors by blocking for one tenth (0.1) of a second.

## Screen Manager

The Screen-manager displays the strings it receives via the Co-Editors  queue to the screen (std-output). After printing all messages to the screen, the Screen manager displays a ‘DONE’ statement.

## Configuration File

The Configuration file has the following format:

    PRODUCER 1
    [number of products]
    queue size = [size]
    …
    …
    …
    PRODUCER n
    [number of products]
    queue size = [size]
    
    Co-Editor queue size = [size]
