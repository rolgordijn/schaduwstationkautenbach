/*
#pragma once
#include <MCP23017.h>
#include "Track.h"
#include "Wissel.h"
#include "IO.h"


extern MCP23017 mcp2;
extern MCP23017 mcp1;
extern MCP23017 mcp0; 


extern Wissel  wissel1;
extern Wissel  wissel2;
extern Wissel  wissel3 ;
extern Wissel  wissel4 ;
extern Wissel  wissel5 ;
extern Wissel  wissel6 ;


extern MCP23017IO led8;
extern MCP23017IO led7;
extern MCP23017IO led6;
extern MCP23017IO led5;

extern MCP23017IO led9;
extern MCP23017IO led10;
extern MCP23017IO led11;
extern MCP23017IO led12;
extern MCP23017IO led13;
extern MCP23017IO led14;
extern MCP23017IO led15;
extern MCP23017IO led16;

extern MCP23017IO knop1;
extern MCP23017IO knop2;
extern MCP23017IO knop3;
extern MCP23017IO knop4;
extern MCP23017IO knop5;
extern MCP23017IO knop6;
extern MCP23017IO knop7;
extern MCP23017IO knop8;

extern MCP23017IO knop9;
extern MCP23017IO knop10;
extern MCP23017IO knop11;
extern MCP23017IO knop12;
extern MCP23017IO knop13;
extern MCP23017IO knop14;
extern MCP23017IO knop15;
extern MCP23017IO knop16;


//extern MCP23017IO bezetmelder1;
extern IOLogDecorator<MCP23017IO> bezetmelder1;
extern IOLogDecorator<MCP23017IO> bezetmelder2;
extern IOLogDecorator<MCP23017IO> bezetmelder3;
extern IOLogDecorator<MCP23017IO> bezetmelder4;
extern IOLogDecorator<MCP23017IO> bezetmelder5;
extern IOLogDecorator<MCP23017IO> bezetmelder6;
extern IOLogDecorator<MCP23017IO> bezetmelder7;
extern IOLogDecorator<MCP23017IO> bezetmelder8;

extern IO * bezetmelders[8];


extern BasicIO led4;
extern BasicIO led3;
extern BasicIO led2;
extern BasicIO led1;

extern BasicIO relay1;
extern BasicIO relay2;
extern BasicIO relay3;
extern BasicIO relay4;
//extern BasicIO relay5;
extern BasicIO relay6;
extern BasicIO relay7;
extern BasicIO relay8;

extern IOLogDecorator<BasicIO> relay5;


extern Track track1;
extern Track track2;
extern Track track3;
extern Track track4;
extern Track track5;
extern Track track6;

extern BasicIO intBezetmelders;
extern BasicIO intDrukknop;

extern Wissel * wissels[6]; 
extern IO * relays[8] ;
extern IO * knoppen[8];
extern IO * leds[8];
extern Track * sporen[6] ; 


void initIO(void);
*/
