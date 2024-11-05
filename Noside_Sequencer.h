#pragma once
#define MAX_STEPS 16
#define CHANNELS 4
class Noside_Sequencer
{

public:
	Noside_Sequencer();

	void setStep(int ind);

	void incrementStep();

	int getCurStep();

	void setChannel(int ind);

	int getCurChannel();

	void setStepActive(int channel, int stepInd, bool b);

	bool getStepActive(int channel, int stepInd);

	bool getCurStepActive(int channel);

	void printStepActive();

	void setStepCV(int channel, int stepInd, int val);

	int getStepCV(int channel, int stepInd);

	int getCurStepCV(int channel);
	
	void printStepCV();

	void printSequencer();

private:
	int curStep;
	int curChannel;
	bool stepActive[CHANNELS][MAX_STEPS];
	int stepCV[CHANNELS][MAX_STEPS];
};


