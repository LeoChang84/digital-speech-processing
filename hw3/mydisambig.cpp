#include <stdio.h>
#include <iostream>
#include <map>
#include <set>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <limits>
#include "Ngram.h"

using namespace std;

class NodeProb { 
    public: 
        double probability;
        vector<string> wordVector; 
};

double getBigramProb(Vocab &voc, Ngram &lm, const char *w1, const char *w2)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}

void GetMappingFromZhuYinBig5(const char* map_filename, map<string, set<string> > &mapping) {
    ifstream fp_map(map_filename);
    string inputStr;
    while (getline(fp_map, inputStr)) {
        stringstream ss(inputStr);
        string zuyin, word;
        ss >> zuyin;
        while (ss >> word) { mapping[zuyin].insert(word); }
    }
    fp_map.close();
}

vector<set<string> > NodeInsertion(vector<string> wordVector, map<string, set<string> > mapping) {
    vector<set<string> > pathGrpah;
    map<string, set<string> >::iterator iter;
    set<string> nodeSet;
    for (int i = 0; i < wordVector.size(); i++) {
        set<string> nodeSet;
        iter = mapping.find(wordVector.at(i));
        if (iter != mapping.end()) {
            nodeSet = (iter->second);
        } else {
            nodeSet.insert(wordVector.at(i));
        }
        pathGrpah.push_back(nodeSet);
    }
    return pathGrpah;
}

vector<set<string> > BuildGraph(string inputStr, map<string, set<string> > mapping) {
    stringstream ss(inputStr);
    stringstream ssConcatenat;
    string token;
    while (ss >> token) ssConcatenat << token;
    vector<string> wordVector;
    for (int i = 0; i < ssConcatenat.str().size(); i += 2) {
        string buf = ssConcatenat.str().substr(i, 2);
        wordVector.push_back(buf);
    }
    return NodeInsertion(wordVector, mapping);
}

string BestPathofLastNode(vector<NodeProb> columnNodes) {
    double maxProb = - numeric_limits<double>::infinity();
    int Index = 0, i;
    for (i = 0; i < columnNodes.size(); i++) {
        if (columnNodes.at(i).probability > maxProb) {
            Index = i;
            maxProb = columnNodes.at(i).probability;
        }
    }
    stringstream ssConcatenat;
    ssConcatenat << "<s> ";
    for (int i = 0; i < columnNodes.at(Index).wordVector.size(); i++) {
        ssConcatenat << columnNodes.at(Index).wordVector.at(i);
        ssConcatenat << " ";
    }
    ssConcatenat << "</s>";
    return ssConcatenat.str();    
}


string Viterbi(vector<set<string> > pathGraph, Ngram &lm, Vocab &vocab) {
    vector<NodeProb> columnNodes;
    NodeProb tmpNodeProb;
    // cout << pathGraph.at(0).size() << endl;
    for (set<string>::iterator node_iter = pathGraph.begin()->begin();
             node_iter != pathGraph.begin()->end(); node_iter++) {

        vector<string> startWordVector;
        startWordVector.push_back((*node_iter));
        tmpNodeProb.wordVector = startWordVector, tmpNodeProb.probability = 0.0;
        // cout << "wordVector" << tmpNodeProb.probability << " " << columnNodes.size() << endl; 
        columnNodes.push_back(tmpNodeProb);
    }
    // cout << "111" << " " << pathGraph.size() << " " << columnNodes.size() << endl;
    for (int i = 1; i < pathGraph.size(); i++) {
        // cout << "222" << endl;
        vector<NodeProb> newColumnNodes;
        
        for (set<string>::iterator node_iter = pathGraph.at(i).begin();
                node_iter != pathGraph.at(i).end(); node_iter++) {
            // cout << "333" << endl;
            string current = (*node_iter);
            vector<string>::iterator iter;
            int maxIndex = 0;
            double Prob, maxProb = - numeric_limits<double>::infinity();
            for (int i = 0; i < columnNodes.size(); i++) {
                // cout << "444" << " " << *(columnNodes.at(i).wordVector.end() - 1) << endl;
                string preWord = *(columnNodes.at(i).wordVector.end() - 1);
                double oldProb = columnNodes.at(i).probability;
                double newProb = getBigramProb(vocab, lm, preWord.c_str(), current.c_str());
                if ((Prob = oldProb + newProb) > maxProb) {
                    maxProb = Prob;
                    maxIndex = i;
                }
            }
            // columnNodes.at(maxIndex).wordVector.push_back(current);
            NodeProb tmpNodeProb = columnNodes.at(maxIndex);
            tmpNodeProb.wordVector.push_back(current), tmpNodeProb.probability = maxProb;
            newColumnNodes.push_back(tmpNodeProb);
            // for (vector<string>::iterator iter = tmpNodeProb.wordVector.begin(); iter != tmpNodeProb.wordVector.end(); iter++)
            //     cout << *iter << " ";
            // cout << endl;
        }
        // cout << "5555" << endl;
        columnNodes.clear();
        columnNodes = newColumnNodes;
    }
    return BestPathofLastNode(columnNodes);
}

int main(int argc, char const *argv[])
{   
    if (argc != 9) {
        cout << "./mydisambig -text [file] -map [map] -lm [LM] -order [orderNum]" << endl;
        return 0;
    }
    const char* text_filename = argv[2];
    const char* map_filename = argv[4];
    const char* lm_filename = argv[6];
    int order = atoi(argv[8]);
    Vocab vocab;
    Ngram lm(vocab, order);
    File lmFile(lm_filename, "r" );
    lm.read(lmFile);
    lmFile.close();
    map<string, set<string> > mapping;
    GetMappingFromZhuYinBig5(map_filename, mapping);    // read mapping from ZhuYiBig5
    ifstream fp_text(text_filename);
    string inputStr;
    while(getline(fp_text, inputStr)) {
        vector<set<string> > pathGraph = BuildGraph(inputStr, mapping);     // build path graph with node set
        string result = Viterbi(pathGraph, lm, vocab);
        cout << result << endl;
    }
    return 0;
}
