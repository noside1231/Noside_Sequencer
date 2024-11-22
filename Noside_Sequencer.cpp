#include "Noside_Sequencer.h"

// #include <iostream>

#define MAX_STEPS 16
#define CHANNELS 4

#define DEBUG

//struct stepParam {
//	bool active;
//	int value;
//};
//
//stepParam stepParams[CHANNELS][MAX_STEPS];

int curStep;
int curChannel;
bool stepActive[CHANNELS][MAX_STEPS];
int stepCV[CHANNELS][MAX_STEPS];
int CVRange[CHANNELS];



Noside_Sequencer::Noside_Sequencer() {
#ifdef DEBUG
	// Serial.print("initializing sequencer...\n");
#endif // DEBUG

	curStep = 1;
	curChannel = 1;

	//initialize step active and CV
	for (int i = 0; i < CHANNELS; i++) {
		for (int j = 0; j < MAX_STEPS; j++) {
			stepActive[i][j] = false;
			stepCV[i][j] = 2048;
			channelCVRange[i] = 16;
		}
	}
}

void Noside_Sequencer::setStep(int ind) {
	if (ind > MAX_STEPS) {
		curStep = MAX_STEPS;
	}
	else if (ind < 1) {
		curStep = 1;
	}
	else {
		curStep = ind;
	}
#ifdef DEBUG
	// Serial.print("setting step to: %i\n", curStep);
#endif // DEBUG


}

void Noside_Sequencer::incrementStep() {
	curStep++;
	if (curStep > MAX_STEPS) {
		curStep = 1;
	}
#ifdef DEBUG
	// Serial.print("incrementing step to: %i\n", curStep);
#endif // DEBUG


}

int Noside_Sequencer::getCurStep() {
	return curStep;
}

void Noside_Sequencer::setChannel(int ind) {
	if (ind > CHANNELS) {
		curChannel = CHANNELS;
	}
	else if (ind < 1) {
		curChannel = 1;
	}
	else {
		curChannel = ind;
	}
#ifdef DEBUG
	// Serial.print("setting channel to: %i\n", curChannel);
#endif // DEBUG

}

int Noside_Sequencer::getCurChannel() {
	return curChannel;
}

void Noside_Sequencer::setStepActive(int channel, int stepInd, bool b) {
	if (channel > CHANNELS) {
		return;
	}
	if (channel < 1) {
		return;
	}
	if (stepInd > MAX_STEPS) {
		return;
	}
	if (stepInd < 1) {
		return;
	}
	stepActive[channel - 1][stepInd - 1] = b;
}

bool Noside_Sequencer::getStepActive(int channel, int stepInd) {
	return stepActive[channel - 1][stepInd - 1];
}

bool Noside_Sequencer::getCurStepActive(int channel) {
	return stepActive[channel - 1][curStep-1];

}

// void Sequencer::printStepActive() {
//
// 	Serial.print("\nSteps Active:\n");
// 	for (int i = 0; i < CHANNELS; i++) {
// 		for (int j = 0; j < MAX_STEPS; j++) {
// 			if (stepActive[i][j]) {
// 				Serial.print("XXX ");
// 			}
// 			else {
// 				Serial.print("--- ");
// 			}
// 		}
// 		Serial.print("\n");
// 	}
// 	Serial.print("\n");
// }

void Noside_Sequencer::setStepCV(int channel, int stepInd, int val) {
	if (channel > CHANNELS) {
		return;
	}
	if (channel < 1) {
		return;
	}
	if (stepInd > MAX_STEPS) {
		return;
	}
	if (stepInd < 1) {
		return;
	}
	stepCV[channel - 1][stepInd - 1] = val;
}

int Noside_Sequencer::getStepCV(int channel, int stepInd) {
	return stepCV[channel - 1][stepInd - 1];
}

int Noside_Sequencer::getCurStepCV(int channel) {
	return stepCV[channel - 1][curStep-1];
}

void Noside_Sequencer::setCVRange(int channel, int range) {
	if (channel > CHANNELS) {
		return;
	}
	if (channel < 1) {
		return;
	}

	if (range > 16) {
		return;
	}	
	if (range < 0) {
		return;
	}
	CVRange[channel] = range;
}

int Noside_Sequencer::getCVRange(int channel) {
	return CVRange[channel];
}

// void Sequencer::printStepCV() {
// 	Serial.print("\nSteps CV:\n");
//
// 	for (int i = 0; i < CHANNELS; i++) {
// 		for (int j = 0; j < MAX_STEPS; j++) {
// 			Serial.print("%03i ", stepCV[i][j]);
// 		}
// 		Serial.print("\n");
// 	}
// 	Serial.print("\n");
// }

// void Sequencer::printSequencer() {
// 	Serial.print("\nActive CV:\n");
//
// 	for (int i = 0; i < CHANNELS; i++) {
// 		for (int j = 0; j < MAX_STEPS; j++) {
// 			if (stepActive[i][j]) {
// 				Serial.print("%03i ", stepCV[i][j]);
// 			}
// 			else {
// 				Serial.print("--- ");
// 			}
// 		}
// 		Serial.print("\n");
// 	}
// 	Serial.print("\n");
// }
