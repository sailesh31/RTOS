Simplex Socket Communication allows the client(on a local system) to communicate information to server(on a cloud). 

It is a one way connection/communication. 

Port used for connection is 4000. This can be changed at both the client and server side as per requirement(PORT_NO variable).

The current max size of inputs/transmissions is 1024 chars. This can also be toggled as per requirement by changing the MAX_BUF_SIZE variable in the code.

In order to run the code, download both the codes, put the client-side code in your local system and put the cloud-side code on your linux cloud vm/system which has gcc in it. Change the serverIP in client-side code to that of the deployed server and run both the programs.(Server to be started first).

