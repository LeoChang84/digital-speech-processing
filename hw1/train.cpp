#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <cstdio>
#include <iostream>
#include "hmm.h"

using namespace std;

HMM hmm;
char seq[MAX_SEQ];  // input sequnce
double sample_num = 0;
double initial_state[MAX_STATE];
double transition_prob_numerator[MAX_STATE][MAX_STATE];
double transition_prob_denominator[MAX_STATE];
double observ_prob_numerator[MAX_OBSERV][MAX_STATE];
double observ_prob_denominator[MAX_STATE];
double alpha[MAX_SEQ][MAX_STATE];
double beta[MAX_SEQ][MAX_STATE];
double gama[MAX_SEQ][MAX_STATE];
double epsilon[MAX_SEQ][MAX_STATE][MAX_STATE];

void init_each_iteration() {
  sample_num = 0;
  memset(seq, 0, sizeof(char) * MAX_SEQ);
  memset(initial_state, 0, sizeof(double) * MAX_STATE);
  memset(transition_prob_numerator, 0, sizeof(double) * MAX_STATE * MAX_STATE);
  memset(transition_prob_denominator, 0, sizeof(double) * MAX_STATE);
  memset(observ_prob_numerator, 0, sizeof(double) * MAX_OBSERV * MAX_STATE);
  memset(observ_prob_denominator, 0, sizeof(double) * MAX_STATE);
}

void init_each_sequence() {
  memset(alpha, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(beta, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(gama, 0, sizeof(double) * MAX_SEQ * MAX_STATE);
  memset(epsilon, 0, sizeof(double) * MAX_SEQ * MAX_STATE * MAX_STATE);
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

void calculate_beta() {
  for (int t = strlen(seq) - 1; t >= 0; t--) {
    for (int i = 0; i < hmm.state_num; i++) {
      if (t == strlen(seq) - 1) {
        beta[t][i] = 1;
      } else {
        for (int j = 0; j < hmm.state_num; j++) {
          beta[t][i] += hmm.transition[i][j] *
                        hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j];
        }
      }
    }
  }
}

void calculate_gama() {
  double sum[MAX_SEQ] = {0};
  for (int t = 0; t < strlen(seq); t++) {
    for (int i = 0; i < hmm.state_num; i++) {
      sum[t] += alpha[t][i] * beta[t][i];
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
        sum[t] += alpha[t][i] * hmm.transition[i][j] *
                  hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j];
      }
    }
    for (int i = 0; i < hmm.state_num; i++) {
      for (int j = 0; j < hmm.state_num; j++) {
        epsilon[t][i][j] =
            (alpha[t][i] * hmm.transition[i][j] *
             hmm.observation[seq[t + 1] - 'A'][j] * beta[t + 1][j]) /
            sum[t];
      }
    }
  }
}

void accumulate_initial() {
  for (int i = 0; i < hmm.state_num; i++) {
    initial_state[i] += gama[0][i];
  }
}

void accumulate_transition() {
  double epsilon_sum[MAX_STATE][MAX_STATE] = {{0}};
  for (int i = 0; i < hmm.state_num; i++) {
    for (int j = 0; j < hmm.state_num; j++) {
      for (int t = 0; t < strlen(seq) - 1; t++) {
        transition_prob_numerator[i][j] += epsilon[t][i][j];
      }
    }
  }
  for (int i = 0; i < hmm.state_num; i++) {
    for (int t = 0; t < strlen(seq) - 1; t++) {
      transition_prob_denominator[i] += gama[t][i];
    }
  }
}

void accumulate_observ() {
  for (int j = 0; j < hmm.state_num; j++) {
    for (int t = 0; t < strlen(seq); t++) {
      observ_prob_numerator[seq[t] - 'A'][j] += gama[t][j];
      observ_prob_denominator[j] += gama[t][j];
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
      hmm.transition[i][j] =
          transition_prob_numerator[i][j] / transition_prob_denominator[i];
    }
  }
}

void update_observ() {
  for (int k = 0; k < strlen(seq); k++) {
    for (int j = 0; j < hmm.state_num; j++) {
      hmm.observation[k][j] =
          observ_prob_numerator[k][j] / observ_prob_denominator[j];
    }
  }
}

int main(int argc, char* argv[]) {
  // input arguments
  if (argc != 5) {
    cout << "Input arguments should be:\n ./train [iterations] [init_model] "
            "[tranining data] [output filename]"
         << endl;
    return 0;
  }
  loadHMM(&hmm, argv[2]);
  int iteration = atoi(argv[1]);
  while (iteration--) {
    init_each_iteration();  // initialize all variables
    FILE* fp = fopen(argv[3], "r");
    while (fgets(seq, sizeof(seq), fp)) {
      init_each_sequence();
      if (seq[strlen(seq) - 1] == '\n') seq[strlen(seq) - 1] = '\0';
      sample_num += 1;
      calculate_alpha();
      calculate_beta();
      calculate_gama();
      calculate_epsilon();
      accumulate_initial();
      accumulate_transition();
      accumulate_observ();
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
