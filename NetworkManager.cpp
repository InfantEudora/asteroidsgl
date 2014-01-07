#include "NetworkManager.h"



udpserver data_server;
udpclient data_client;


void NetMan::add_player(uint32_t id, uint16_t port){
	bool have = false;
	//Do we have it already?
	for (int i=0;i<10;i++){
		if (playerinfo[i].unique_id == id){
			have = true;		
			printf("Already have player with uid: %08X:%u\n", id,port);
			return;
		}
	}

	//Add it.
	for (int i=0;i<10;i++){
		if (playerinfo[i].taken == false){
			playerinfo[i].taken = true;
			playerinfo[i].unique_id = id;
			playerinfo[i].port = port;
			printf("New player added with uid: %08X:%u\n", id,port);

			//Set its ship
			if (i!=0){
				players[i].ship.alive = true;
			}

			return;
		}
	}
}

//Returns the index of a player by its ID.
int NetMan::get_playerbyid(uint32_t id){	
	for (int i=0;i<10;i++){
		if (playerinfo[i].unique_id == id){
			return i;
		}
	}
	return -1;
}

int NetMan::num_players;
Player* NetMan::players;

int NetMan::num_asteroids;
Asteroid* NetMan::asteroids;

uint8_t NetMan::unique_id[8];
uint32_t NetMan::unique_id32;

NetMan::NetMan(){
	for(int i =0;i<10;i++){
		playerinfo[i].taken = false;
		playerinfo[i].unique_id = 0;
	}

	port_in = 5000;
	port_out = 5000;

	last_ast = 0;
}


NetMan::~NetMan(){
	

}
void NetMan::init(Player* _players, int nump, Asteroid* _asteroids, int numa){
	players = _players;
	num_players = nump;

	asteroids = _asteroids;
	num_asteroids = numa;

	//Generate unique id;
	for(uint8_t i=0;i<8;i++){
		unique_id[i] = rand();
	}

	
	memcpy(&unique_id32,unique_id,4);


}

void NetMan::send(){		
	//Prepare header
	data_client.data[0] = PACK_HEAD;
	data_client.data[1] = PACK_GAME;

	memcpy(&data_client.data[2],&unique_id32,sizeof(unique_id32));

	//Add data.
	int len = 6;
	len += players[0].SendData((uint8_t*)&data_client.data[len]);

	for(int i =0;i<10;i++){
		//Send data to each alive player except self.
		if ((playerinfo[i].taken)&& (playerinfo[i].unique_id != unique_id32)){
			//printf("Sending data to player %08X\n",playerinfo[i].unique_id);
			data_client.open(playerinfo[i].port);
			data_client.send((char*)data_client.data,len);
		}
	}

	//And that ...
	sendAsteroids();
}


void NetMan::sendAsteroids(){		
	//Prepare header
	data_client.data[0] = PACK_HEAD;
	data_client.data[1] = PACK_AST;

	//Number of asteroids in this data packet.
	int num_ast_inpack = 20;
	if ((num_asteroids-last_ast) == 0){
		//Start at the beginning.
		last_ast = 0;		
	}
	if ((last_ast + num_ast_inpack)> num_asteroids){
		num_ast_inpack = num_asteroids - last_ast;
	}

	


	int len = 2;
	memcpy(&data_client.data[len],&unique_id32,sizeof(unique_id32));
	len += sizeof(unique_id32);

	//Send number of asteroids, and the start index of this packet.
	memcpy(&data_client.data[len],&num_asteroids,sizeof(num_asteroids));
	len += sizeof(num_asteroids);

	//Starting asteroid in this packet.
	memcpy(&data_client.data[len],&last_ast,sizeof(last_ast));
	len += sizeof(last_ast);

	

	memcpy(&data_client.data[len],&num_ast_inpack,sizeof(num_ast_inpack));
	len += sizeof(num_ast_inpack);

	//Add data.
	for (int i=last_ast;i<(last_ast+num_ast_inpack);i++){	
		len += asteroids[i].SendData((uint8_t*)&data_client.data[len]);
	}

	last_ast += num_ast_inpack;

	for(int i =0;i<10;i++){
		//Send data to each alive player except self.
		if ((playerinfo[i].taken)&& (playerinfo[i].unique_id != unique_id32)){
			//printf("Sending asteroid data to player %08X\n",playerinfo[i].unique_id);
			data_client.open(playerinfo[i].port);
			data_client.send((char*)data_client.data,len);
		}
	}	
}

void NetMan::receive(){
	int len;
	while(1){
		len = data_server.receive(1024,10);
		if (len ==0){
			break;
		}
		
		handle_data((uint8_t*)data_server.data,len);		
	}
}

void NetMan::startup(){
	//UDP Servers
	for(int i=0;i<10;i++){
		if (!data_server.listen(port_in)){
			port_in++;
		}else{
			break;
		}			
	}
	
	data_client.open(port_out);

	//Add self.
	add_player(unique_id32,port_in);
}

//Send an alive message over the network.
void NetMan::send_alive(void){
	for (int i =0;i<10;i++){
		data_client.open(port_out+i);
		//We are allowed to send random stuff.
		//data_client.send("Alive!\n",7);
		//Send alive packet.
		uint8_t alive_packet[8];
		alive_packet[0] = PACK_HEAD;
		alive_packet[1] = PACK_ID;

		memcpy(&alive_packet[2],&unique_id,4);
		memcpy(&alive_packet[6],&port_in,2);
		
		data_client.send((char*)alive_packet,8);
	}	
	printf("Sending alive packets.\n");
}


void NetMan::handle_data(uint8_t* data, int len){
	printf("Received stuff: len = %i\n",len);

	if (data[0]==PACK_HEAD){
		if (data[1]==PACK_ID){
			//Receive player ID.
			uint32_t pid;
			uint16_t pport;
			memcpy(&pid,&data[2],4);
			memcpy(&pport,&data[6],2);

			add_player(pid,pport);
		}else if (data[1]==PACK_GAME){
			//Make sure it's not my id:
			uint32_t recid;
			memcpy(&recid,&data[2],4);
			if (recid != unique_id32){				
				int i = get_playerbyid(recid);
				if (i == -1){
					return;
				}
				//printf("Updating player[%i] ID=%04X\n",i,recid);
				players[i].ParseData(&data[6]);	
			}
		}else if (data[1]==PACK_AST){
			//Make sure it's not my id:
			uint32_t recid;
			memcpy(&recid,&data[2],4);
			if ((recid != unique_id32)&&(recid>unique_id32)){				
				int i = get_playerbyid(recid);
				if (i == -1){
					return;
				}
				printf("Updating asteroids from [%i] ID=%04X\n",i,recid);
				//Set asteroids.

				int rec_n;
				int rec_start;
				int rec_cnt;

				int len = 6;
				//Get number of asteroids, and the start index of this packet.
				memcpy(&rec_n,&data[len],sizeof(rec_n));
				len += sizeof(rec_n);

				//Starting asteroid in this packet.
				memcpy(&rec_start,&data[len],sizeof(rec_start));
				len += sizeof(rec_start);

				//Number of asteroids in this data packet.				
				memcpy(&rec_cnt,&data[len],sizeof(rec_cnt));
				len += sizeof(rec_cnt);

				//Get data and parse it.
				for (int i=rec_start;i<(rec_start+rec_cnt);i++){	
					len += asteroids[i].ParseData(&data[len]);
				}
			}
		}
	}

}