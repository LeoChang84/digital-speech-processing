#include "hmm.h"
#include <iostream>
#include <cstdio>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

using namespace std;

HMM hmm;
char seq[MAX_SEQ];  //input sequnce
double sample_num = 0;
double initial_state[MAX_STATE];
double transition_prob[MAX_STATE][MAX_STATE];
double observ_prob[MAX_OBSERV][MAX_STATE];
double alpha[MAX_SEQ][MAX_STATE];
double beta[MAX_SEQ][MAX_STATE];
double gama[MAX_SEQ][MAX_STATE];
double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];

void init() {
  sample_num = 0;
  memset(seq, 0, sizeof(char) * MAX_SEQ);
  memset(initial_state, 0, sizeof(double) * MAX_STATE);
  memset(transition_prob, 0, sizeof(double) * MAX_STATE * MAX_STATE);
  memset(observ_prob, 0,sizeof(double) * MAX_OBSERV * MAX_STATE);
  memset(alpha, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(beta, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(gama, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(epsilon, 0, sizeof(double) * MAX_SEQ * MAX_STATE * MAX_STATE);
}

void print_alpha() {
  for (int i = 0; i < strlen(seq); i++) {
    for (int j = 0; j < hmm.state_num; j++) {
      printf("%lf ", alpha[i][j]);
    }
    cout << endl;
  }
}

void calculate_alpha() {
  for (int t = 0; t < strlen(seq); t++) {
    for (int j = 0; j < hmm.state_num; j++) { 
      if (t == 0) {
          alpha[t][j] = hmm.initial[j] * hmm.observation[seq[t] - 'A'][j];
      } else {
        for (int i = 0; i < hmm.state_num; i++) {
          alpha[t][j] += alpha[t - 1][i] * hmm.transition[i][j];
        }
        alpha[t][j] *= hmm.observation[seq[t] - 'A'][j];
      }
    }
  }
}

void print_beta() {
  for (int t = 0; t < strlen(seq); t++) {
    for (int i = 0; i < hmm.state_num; i++) {
      printf("%lf ", beta[t][i]);
    }
    printf("\n");
  }
}

void calculate_beta() {
  for (int t = strlen(seq) - 1; t >= 0; t--) {
    for (int i = 0; i < hmm.state_num; i++) {
      if (t == strlen(seq) - 1) {
        beta[t][i] = 1;
      } else {
        for (int j = 0; j < hmm.state_num; j++) {
          beta[t][i] += hmm.transition[i][j] * hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j];
        }
      }
    }
  }
}

void calculate_gama() {
  double sum[MAX_SEQ] = {0};
  for (int t = 0; t < strlen(seq); t++) {
    for (int i = 0; i < hmm.state_num; i++) {
      sum[t] += alpha[t][i];
    }
    for (int i = 0; i < hmm.state_num; i++) {
      gama[t][i] = (alpha[t][i] * beta[t][i]) / sum[t];
    }
  }
}

void calculate_epsilon() {
  double sum[MAX_SEQ] = {0};
  for (int t = 0; t < strlen(seq) - 1; t++) {
    for (int i = 0; i < hmm.state_num; i++) {
      for (int j = 0; j < hmm.state_num; j++) {
        sum[t] += alpha[t][i] * hmm.transition[i][j] * hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j];
      }
    }
    for (int i = 0; i < hmm.state_num; i++) {
      for (int j = 0; j < hmm.state_num; j++) { 
        epsilon[t][i][j] = (alpha[t][i] * hmm.transition[i][j] * hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j]) / sum[t];
      }
    }
  }
}

void calculate_initial() {
  for (int i = 0; i < hmm.state_num; i++) {
    initial_state[i] = gama[0][i];
  }
}

void calculate_transition() {
  double epsilon_sum[MAX_STATE][MAX_STATE] = {{0}};
  for (int i = 0; i < hmm.state_num; i++) {
    for (int j = 0; j < hmm.state_num; j++) {
      for (int t = 0; t < strlen(seq); t++) {
        epsilon_sum[i][j] += epsilon[t][i][j];
      }
    }
  }
  double gama_sum[MAX_STATE] = {0};
  for (int i = 0; i < hmm.state_num; i++) {
    for (int t = 0; t < strlen(seq); t++) {
      gama_sum[i] += gama[t][i];
    }
  }
  for (int i = 0; i < hmm.state_num; i++) {
    for (int j = 0; j < hmm.state_num; j++) {
      transition_prob[i][j] = epsilon_sum[i][j] / gama_sum[i];
    }
  }
}

void calculate_observ() {
  double gama_sum[MAX_SEQ][MAX_STATE] = {{0}};
  double gama_observ[MAX_SEQ];
  for (int j = 0; j < hmm.state_num; j++) {
    for (int t = 0; t < strlen(seq); t++) {
      gama_sum[seq[t] - 'A'][j] += gama[t][j];
      gama_observ[j] += gama[t][j];
    }
  }
  for (int k = 0; k < strlen(seq); k++) {
    for (int j = 0; j < hmm.state_num; j++) {
      observ_prob[k][j] = gama_sum[k][j] / gama_observ[k];
    }
  }
}

void update_initial() {
  for (int i = 0; i < hmm.state_num; i++) {
    hmm.initial[i] = initial_state[i] / sample_num;
  }
}

void update_transition() {
  for (int i = 0; i < hmm.state_num; i++) {
    for (int j = 0; j < hmm.state_num; j++) {
      hmm.transition[i][j] = transition_prob[i][j];
    }
  }
}

void update_observ() {
  for (int k = 0; k < hmm.state_num; k++) {
    for (int j = 0; j < strlen(seq); j++) {
      hmm.observation[k][j] = observ_prob[k][j];
    }
  }
}

int main(int argc, char *argv[]) { 
  //input arguments
  if (argc != 5) {
    cout << "Input arguments should be:\n ./train [iterations] [init_model] [tranining data] [output filename]" << endl;
    return 0;
  }
  loadHMM(&hmm, argv[2]);
  int iteration = atoi(argv[1]);
  while (iteration--) {
    init(); //initialize all variables
    FILE* fp = fopen(argv[3], "r");
    while (fgets(seq, sizeof(seq), fp)) {
      //cout << seq << endl;
      //cout << "size is : " << strlen(seq) << endl;
      if (seq[strlen(seq)-1] == '\n') seq[strlen(seq)-1] = '\0'; 
      sample_num += 1;
      calculate_alpha();
      cout << "alpha" << endl;
      print_alpha();
      calculate_beta();
      cout << "beta" << endl;
      calculate_gama();
      cout << "gamma" << endl;
      calculate_epsilon();
      cout << "epsilon" << endl;
      calculate_initial();
      calculate_transition();
      calculate_observ();
//      break;
    }
    update_initial();
    update_transition();
    update_observ();
    fclose(fp);
  }
  FILE* fp = fopen(argv[4], "w");
  dumpHMM(fp, &hmm);
  fclose(fp);
  return 0;
}
