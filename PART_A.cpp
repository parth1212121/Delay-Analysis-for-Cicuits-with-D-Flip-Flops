#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include<limits>

using namespace std;

double calc_delayA(const string& signal,map<string,double>& timingsA, map<string,pair<string,vector<string>>>& connectionsA,
                   map<string,double> delay, vector<int>& comb_delays ){
    if (timingsA.find(signal) != timingsA.end()) {
        return timingsA[signal];
    } else {
        double delayVal = 0;
        for (const string& ins : connectionsA[signal].second) {
            delayVal = max(delayVal, calc_delayA(ins, timingsA, connectionsA, delay,comb_delays));
        }
        delayVal += delay[connectionsA[signal].first];
        timingsA[signal] = delayVal;
        return delayVal;
    }
}

int main(int argc, char *argv[]) {

    vector<string> content;
    vector<string> input;
    vector<string> output;
    map<string, double> timingsA;
    map<string, pair<string,vector<string>>> connectionsA;
    map<string, double> delay;

    // Extraction of data from the files....

    ifstream file("circuit.txt");

    if (file.is_open()) {               // writing circuit.txt into content......
        string line;
        while (getline(file, line)) {
            if (!line.empty() && line[0] != '/' && line.find_first_not_of(" \t") != string::npos) {
                content.push_back(line);
            }
        }
        file.close();
    } else {
        cerr << "Error opening circuit.txt" << endl;
        return 1;
    }

    ifstream gateDelaysFile("gate_delays.txt");

    if (gateDelaysFile.is_open()) {      // writing gat_delays.txt into content......
        string line;
        while (getline(gateDelaysFile, line)) {
            if (!line.empty() && line[0] != '/' && line.find_first_not_of(" \t") != string::npos) {
                content.push_back(line);
            }
        }
        gateDelaysFile.close();
    } else {
        cerr << "Error opening gate_delays.txt" << endl;
        return 1;
    }

    // Filling the data dictionary

    vector<int>comb_delays;
    vector<float>results;
    try {
        for (const string& line : content) {
                    
            if (line.find("PRIMARY_INPUTS") == 0) {        // filling the input vector from content....
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {    // timing A represents delay of that particular node.......
                    timingsA[tokens[i]] = 0;
                    input.push_back(tokens[i]);
                }
            } 
                      
            else if (line.find("PRIMARY_OUTPUTS") == 0) {        // filling the output vector from content....
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    output.push_back(tokens[i]);
                }
            } 
                          
             // Entering into the gate_delay phase of content....

            else if (line.find("NAND2") == 0 || line.find("NOR2") == 0 || line.find("INV") == 0 ||
                       line.find("XOR2") == 0 || line.find("AND2") == 0 || line.find("OR2") == 0 ||
                       line.find("XNOR2") == 0 || line.find("DFF")==0 || line.find("NAND2") == 1 || line.find("NOR2") == 1 || line.find("INV") == 1 ||
                       line.find("XOR2") == 1 || line.find("AND2") == 1 || line.find("OR2") == 1 ||
                       line.find("XNOR2") == 1  ) {

                vector<string> tokens;                          // tokenisation begins....
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }                                                // tokenisation ends....

            if(line.find("DFF")==0){

                delay["DFF"]=0;
                vector<string> inputs(tokens.begin() + 1, tokens.end());
                vector<string>back_connection(inputs.begin(),inputs.end()-1);
                connectionsA[inputs.back()]=(make_pair("DFF",back_connection));     // storage in connectionsA
                timingsA[tokens[2]]=0;    
               
            }
            else{

                string gate = tokens[0];
                vector<string> inputs(tokens.begin() + 1, tokens.end());
                if (tokens[1]=="NAND2" || tokens[1]=="NOR2" || tokens[1]=="INV" || tokens[1]=="XOR2" || tokens[1]=="AND2" || tokens[1]=="OR2" || tokens[1]=="XNOR2" ) {
                    if(delay.find(tokens[1])==delay.end()){

                        delay[tokens[1]] = stod(tokens[2]);

                    }
                    else{
                        delay[tokens[1]] = min(delay[tokens[1]],stod(tokens[2]));  // taking the fastest out of the given....
                    }
                }                                
                 else {

                    vector<string>back_connection(inputs.begin(),inputs.end()-1);
                    connectionsA[inputs.back()]=(make_pair(gate,back_connection));     // storage in connectionsA
                    
                }          
            }
            
            }
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    // second_time....

    ///FINDING THE MAXIMUM LENGTH COMBINATORICAL PATH....


     try {
        for (const string& line : content) {
                    
            if (line.find("PRIMARY_INPUTS") == 0) {        // filling the input vector from content....
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {    // timing A represents delay of that particular node.......                  
                    results.push_back(calc_delayA(tokens[i],timingsA,connectionsA,delay,comb_delays));
                }

            } 
                      
            else if (line.find("PRIMARY_OUTPUTS") == 0) {        // filling the output vector from content....
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    results.push_back(calc_delayA(tokens[i],timingsA,connectionsA,delay,comb_delays));
                }
            } 

             else if (line.find("INTERNAL_SIGNALS") == 0) {        // filling the output vector from content....
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    results.push_back(calc_delayA(tokens[i],timingsA,connectionsA,delay,comb_delays));
                }}

        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    ofstream ans_file("longest_delay.txt");
    float maxi=0;    
    for(int i=0;i<results.size();i++){
        if(results[i]>maxi){            
            maxi=results[i];
        }
    }
    if(ans_file.is_open()){
        ans_file<<maxi<<endl;
    }
    else{
        cerr<<"Error opening longest_delay"<<endl;
    }

    return 0;
}

