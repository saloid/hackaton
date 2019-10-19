#include <cstdio>
#include <cstdlib>
#include <string>
#include <windows.h>
extern "C"
{
#include "MQTTClient.h"
}

//#define SIMULATION

#ifdef SIMULATION
#define TOPIC_PREFICS "redbutton/test/u14"
#else
#define TOPIC_PREFICS "redbutton"
#endif

#define ADDRESS "tcp://192.168.1.208:1883"
#define CLIENTID "ExampleClientPubfewgfeg"

#define COMMAND_SUFICS "/command"
#define TELEMETRY_SUFICS "/telemetry/position"

#define QOS 1
#define TIMEOUT 10000L
volatile MQTTClient_deliveryToken token;
MQTTClient client;
MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
MQTTClient_message pubmsg = MQTTClient_message_initializer;

const float speedMmMsec = 0.485;
const float rotationSpeedDegMsec = 2.33;

enum Rotation
{
	LEFT,
	RIGHT
};
/*
enum Direction
{
	UP, RIGHT, DOWN, LEFT
};
*/

struct structPath
{
	uint16_t distance;
	Rotation rotate;
};

struct CordsStruct
{
	uint16_t x;
	uint16_t y;
	uint16_t angle;
}lastCords;

const structPath path[] =
	{{777, Rotation::RIGHT},
	 {321, Rotation::RIGHT},
	 {777, Rotation::LEFT},
	 {317, Rotation::LEFT},
	 {777, Rotation::RIGHT},
	 {521, Rotation::RIGHT},
	 {378, Rotation::RIGHT},
	 {204, Rotation::LEFT},
	 {316, Rotation::LEFT},
	 {3640, Rotation::LEFT}};

/*
control_pos = {
    {157,280},
    {157,934},
    {478,934},
    {478,157},
    {795,157},
    {795,934},
    {1316,934},
    {1316,556},
    {1112,556},
    {1112,556},
    {1476,240}}
*/
void sendCommand(char *stringToSend)
{
	pubmsg.payload = stringToSend;
	pubmsg.payloadlen = strlen(stringToSend);
	pubmsg.qos = QOS;
	pubmsg.retained = 0;

	printf("Sent: %s\n", stringToSend);

	MQTTClient_publishMessage(client, TOPIC_PREFICS COMMAND_SUFICS, &pubmsg, const_cast<MQTTClient_deliveryToken *>(&token));
	MQTTClient_waitForCompletion(client, token, 10000);
}

void executeScenario ()
{
	sendCommand("forward:1165");
	Sleep(2000);
	sendCommand("right:210");
	Sleep(1000);
	sendCommand("forward:661");
	Sleep(1000);
	sendCommand("right:210");
	Sleep(1000);
	sendCommand("forward:1165");
	Sleep(2000);
	sendCommand("left:210");
	Sleep(1000);
	sendCommand("forward:553");
	Sleep(1000);
	sendCommand("left:210");
	Sleep(1000);
}

void cancelCommand()
{
	sendCommand("");
}

void ride(uint16_t lengthToRide)
{
	uint16_t timeToExecute;
	char stringToSend[50] = "forward:";
	
	
	
	timeToExecute = lengthToRide / speedMmMsec;
	itoa(timeToExecute, &stringToSend[strlen(stringToSend)], 10);
	sendCommand(stringToSend);
	Sleep(timeToExecute * 1.3);
}

void rotate(Rotation direction, uint16_t angle)
{
	uint16_t timeToExecute;
	char stringToSend[50];

	if (direction == LEFT)
	{
		strncat(stringToSend, "left:", 6);
	}
	else
	{
		strncat(stringToSend, "right:", 7);
	}
	timeToExecute = angle / rotationSpeedDegMsec;
	itoa(timeToExecute, &stringToSend[strlen(stringToSend)], 10);
	sendCommand(stringToSend);
	Sleep(timeToExecute * 1.3);
}
/*
void safeRide(structPath path, Direction direction)
{
	ride(path.distance / 2);
	
	while ()
	{
		ride(10);
	}
	
}
*/

void safeRotate(structPath path)
{
	rotate(path.rotate, 90 / 2);
	/*
	while (getCurrentAngle() <)
	{
		rotate(path.rotate, 5);
	}
	*/
}

void mainProg(void)
{
	for (uint8_t i = 0; i < (sizeof(path) / sizeof(structPath)); i++)
	{
		//safeRide(path[i]);
		safeRotate(path[i]);
	}
	printf("Completed!!!!\n");
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	char *payloadptr;

	printf("Input msg. topic %s  message: %s\n", topicName, message->payload);

	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

void connlost(void *context, char *cause)
{
	(void) context;
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
	(void) context;
	printf("Message with token value %d delivery confirmed\n", dt);
	token = dt;
}

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	int rc;

	MQTTClient_create(&client, ADDRESS, CLIENTID,
					  MQTTCLIENT_PERSISTENCE_NONE, NULL);
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = "saloid";
	conn_opts.password = "abrakadabra";

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(-1);
	}

	executeScenario();

	MQTTClient_subscribe(client, "mqtt_test/output", QOS);

	//mainProg();

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);

	return rc;
}
