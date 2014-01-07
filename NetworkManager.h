#ifndef NWMAN_H 
#define NWMAN_H 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "udpserver.h"
#include "Player.h"

typedef struct{
	bool taken;
	uint32_t unique_id;
	uint16_t port;
}player_info;

#define PACK_HEAD	0x35

#define PACK_GAME	0x77
#define PACK_AST	0x78

#define PACK_ID		0x88



class NetMan{
public:
	NetMan();
	~NetMan();

	static void init(Player* players, int nump, Asteroid* asteroids, int numa);

	void handle_data(uint8_t* data, int len);
	void add_player(uint32_t,uint16_t);
	int get_playerbyid(uint32_t id);
	void send_alive(void);

	void send();
	void NetMan::sendAsteroids();
	void startup();
	void receive();

	static uint32_t unique_id32;
private:
	static int num_players; 
	static Player* players;

	static int num_asteroids; 
	static Asteroid* asteroids;

	int port_out;
	int port_in;

	int last_ast;


	static uint8_t unique_id[8];

	player_info playerinfo[10];

};


#endif