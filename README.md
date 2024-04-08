Traffic Shape:

Tim Chang @ USC 2023
jentingc@gmail.com

1. Environment:
	
	using C
	execute the emulation by using "$make"

2. Description:
	
	Emulate internet traffic shape.
	Packets and token are incoming continuelly.
	Packets are queued.
	Once the number of token satisfies the need of the packet in the front of queue, the packet start processing.

3. constraint:
	
	The capcity of token bucket: 10
	The requried number of token: 3


3. Implementation:
	
	Using 5 threads to process concurrently<br/>
	pthread receives incoming packets and pushes them into a queue<br/>
	tthread receives incoming tokens and pushes them into another queue<br/>
	s1thread and s2thread hold the packets that satisfy the processing requirements and process them<br/>
	cthread holds the errors that occur during the process.<br/>

4. Result:
	
	Record the time including the time when packets arrive, the time when packets enter the queue, the time when packets start processing, and the time when tokens arrive.

	Print statistical data in the .out file.


