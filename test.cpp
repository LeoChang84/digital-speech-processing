#include "hmm.h"
#include <iostream>

#define MAX_NUM 10

using namespace std;

char seq[MAX_SEQ];
HMM hmms[MAX_NUM];
double delta[MAX_SEQ][MAX_STATE];
double prob[MAX_NUM];

void init() {
	memset(seq, 0, sizeof(char) * MAX_SEQ);
	memset(&hmms, 0, sizeof(HMM) * MAX_NUM);
	memset(delta, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
}

void calculate_delta(int num_models, int* model_index) {
	for (int n = 0; n < num_models; n++) {
		memset(delta, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
		for (int t = 0; t < strlen(seq); t++) {
			for (int j = 0; j < hmms[n].state_num; j++) {
				if (t == 0) {
					delta[t][j] = hmms[n].initial[j] * hmms[n].observation[seq[t] - 'A'][j];
				} else {
					double max_prob = 0;
					for (int i = 0; i < hmms[n].state_num; i++) {
						double tmp = delta[t - 1][i] * hmms[n].transition[i][j];
						if (max_prob < tmp) max_prob = tmp;
					}
					delta[t][j] = max_prob * hmms[n].observation[seq[t] - 'A'][j];
				}
			}
		}
		for (int i = 0; i < hmms[n].state_num; i++) {
			double tmp = delta[strlen(seq) - 1][i];
			if (tmp > prob[n]) prob[n] = tmp;
		}
	}
	for (int n = 0; n < num_models; n++) {
		if (prob[n] > prob[*model_index]) *model_index = n;
	}
}

int main(int argc, char *argv[]) {
	if (argc != 4) {
		cout << "Input arguments should be:\n ./test [modellist] [testing data] [output filename]" << endl;
    	exit(1);
	}
	FILE* fin = fopen(argv[2], "r");
	FILE* fout = fopen(argv[3], "w");
	if (!fin || !fout) {
		cout << "error occur when open file" << endl;
		exit(1);
	}
	init();
	int num_models = load_models(argv[1], hmms, MAX_NUM);
	while (fgets(seq, sizeof(seq), fin)) {
		memset(prob, 0, sizeof(double) * MAX_NUM);
		int model_index = 0;
		if (seq[strlen(seq)-1] == '\n') seq[strlen(seq)-1] = '\0';
		calculate_delta(num_models, &model_index);
		fprintf(fout, "%s %e\n", hmms[model_index].model_name, prob[model_index]);
	}
	fclose(fin);
	fclose(fout);
	return 0;
}
