#include "types.h"
#include "net_parse.h"
#include "lib_parse.h"

parse file f {
	while (!'\eof') {

		fetch(line) from file f;
		parse line n;
		first token t;

		switch case 't':

			case "input" {
				create node from next token;							//name of next token in line
				enqueue v1 in PI_list;									//v1 is a primary input
			}

			case "output" {
				create node from next token;							//name of next token in line
				enqueue v2 in PO_list;									//v2 is a primary output
			}

			case "instance" {
				create cell from next token;							//name of next token in line
				c1.parameters = fopen(lib);								//fetch cell parameters from lib file

				inpins of c1 = i;
				outpins of c1 = j;

				int k,l;
				for (k = 0; k < j; k++) {
					if(next_token != PI_list) {
						create node from next token;					//parse tokens until reaching the proper number as instructed in library specs
					}
				}
				for (l = 0; l < j; l++) {
					if(next_token != PO_list) {
						create node from next token;					//parse remaining tokens and verify that they are exactly as many as in lib specs
					}
				}
			}

			case "wire" {
				if(next_token != existing_nodes) {
					create node v3 from next token;
				}
				else {
					next token = v3;									//already existing node from parsing "input", "outpout" or "instance" fields
				}

				while (there are still i tokens) {
					if(next_token != existing_nodes) {
						create node v[i] from next token;
						create edge e[i];								//create edges for all midpoints of wire
						v3 = e[i].start;								//with v3 as your starting point
						v[i] = e[i].end;								//and v[i] new node as endpoint of edge
					}
					else {
						create edge e[i];								//create edges for all midpoints of wire
						v3 = e[i].start;								//with v3 as your starting point
						next token = v[i];								//v[i] existing node
						v[i] = e[i].end;								// used as endpoint of edge
					}
				}
			}

			case "cap" {
				if (next_token != existing_nodes) {
					create AuxNode from next token;
					AuxNode.cap = next_token;
				}
				else {
					v4.cap = next_token;								//v4 an already existing node
				}

			case "res" {
				if ((next_token != existing_nodes) || (next_token++ != existing_nodes)) {
					create AuxEdge f;
					create AuxNode v5 from next token;
					create AuxNode v6 from token after;
					f.start = v5;
					f.end = v6;
					f.res = next_token;
				}
				else {
					e[i].res = next_token;								//edge e[i] with starting point st and endpoint en has its resistor determined
				}
			}

			case "at" {
				next token one of PI_list = pi;
				pi.erlFall = next_token;
				pi.ltFall = next_token;
				pi.erlRise = next_token;
				pi.ltRise = next_token;
			}

			case "slew" {
				next token one of PI_list = pi;
				pi.erlFall = next_token;
				pi.erlRise = next_token;
				pi.ltFall = next_token;
				pi.ltRise = next_token;
			}

			case "rat" {
				next token one of PO_list = po;
				next token tok;

				switch case tok:

					case "early" {
						po.early = next_token;
					}

					case "late" {
						po.late = next_token;
					}
			}

			case "clock" {
				if (next_token != existing_nodes) {
					create node v7 from next token;
				}
				v7.period = next_token;
			}
