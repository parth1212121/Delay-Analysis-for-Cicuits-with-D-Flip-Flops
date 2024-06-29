#include <iostream>
#include <fstream>                           // AREA_OPTIMIZATION!!!!!!!!
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

double calc_delayA_2(const string& signal,map<string,double>& timingsA, map<string,pair<string,vector<string>>>& connectionsA,
                   map<string,string>&curr_version,map<string,pair<double,double>>version_char,map<string,double>&threshhold,map<string,vector<string>>&version,map<string,double>&new_delay,map<string,double>&new_area){
    if (timingsA.find(signal) != timingsA.end()) {
        return timingsA[signal];
    } else {
        double delayVal = 0;
        for (const string& ins : connectionsA[signal].second) {
            delayVal = max(delayVal,calc_delayA_2(ins, timingsA, connectionsA,curr_version,version_char,threshhold,version,new_delay,new_area));
        }
        int x=threshhold[signal]-delayVal;

        string gate=connectionsA[signal].first;
        float maxi=0;
        for(auto gate_type :version[gate]){
            if(version_char[gate_type].first<=x && version_char[gate_type].first>=maxi){      // MAIN_CODE!!!!
                maxi=version_char[gate_type].first;
                curr_version[gate]=gate_type;
            }
        }
        delayVal += version_char[curr_version[gate]].first;
        new_delay[signal]=delayVal;
        new_area[signal]=version_char[curr_version[gate]].second;
        timingsA[signal] = delayVal;
        return delayVal;
    }
}

double calc_delayB(const string& signal, map<string, double>& timingsB,map<string, vector<pair<string,string>>>&connectionsB,
                   map<string, double>& delay) {
    if (timingsB.find(signal) != timingsB.end()) {
        return timingsB[signal];
    } else {
        double delayVal = numeric_limits<double>::infinity();
        for (const pair<string, string>& gateIns : connectionsB[signal]) {
            delayVal = min(delayVal, calc_delayB(gateIns.second, timingsB, connectionsB, delay) - delay[gateIns.first]);
        }
        timingsB[signal] = delayVal;
        return delayVal;
    }
}


int main(int argc, char *argv[]) {

    vector<string> content;
    vector<string> input;
    vector<string> output;
    map<string, double> timingsA;
    map<string, pair<string,vector<string>>> connectionsA;    
    map<string, double> timingsB;
    map<string, vector<pair<string,string>>> connectionsB;
    map<string,double>threshold;
    map<string, double> delay;
    map<string,double>new_delay;     // assosiated with signal   local
    map<string,double>new_area;      // assosiated with signal   local
    map<string,vector<string>>version;
    map<string,string>curr_version;
    map<string,pair<double,double>>version_char;            // pair(delay,area).


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

    int constraint_delay=0;                  // MAX DELAY ACCEPTABLE....

    ifstream delay_constraint_file("delay_constraint.txt");
    if(delay_constraint_file.is_open()){

        string line;
        while (getline(delay_constraint_file, line)) {
            if (!line.empty() && line[0] != '/' && line.find_first_not_of(" \t") != string::npos) {
                constraint_delay=stod(line);
            }
        }
        delay_constraint_file.close();       
    } else {
        cerr << "Error opening delay_constraint.txt" << endl;
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
                    timingsB[tokens[i]]=constraint_delay;
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
                new_delay[tokens[2]]=0;
                new_area[tokens[2]]=0;           
                connectionsA[inputs.back()]=(make_pair("DFF",back_connection));     // storage in connectionsA
                timingsA[tokens[2]]=0;    
                for (size_t i = 0; i < inputs.size() - 1; i++) {
                    connectionsB[inputs[i]].push_back(make_pair("DFF", inputs.back()));
                }       
                timingsB[tokens[1]]=constraint_delay; 
            }
            else{

                string gate = tokens[0];
                vector<string> inputs(tokens.begin() + 1, tokens.end());    // Content of gate_delays
                if (tokens[1]=="NAND2" || tokens[1]=="NOR2" || tokens[1]=="INV" || tokens[1]=="XOR2" || tokens[1]=="AND2" || tokens[1]=="OR2" || tokens[1]=="XNOR2" ) {
                    if(delay.find(tokens[1])==delay.end()){
                        delay[tokens[1]] = stod(tokens[2]);
                        version[tokens[1]].push_back(tokens[0]);
                        version_char[tokens[0]]=make_pair(stod(tokens[2]),stod(tokens[3]));
                        curr_version[tokens[1]]=tokens[0];
                    }
                    else{
                        double x=delay[tokens[1]];
                        delay[tokens[1]] = min(delay[tokens[1]],stod(tokens[2]));  
                        if(x!=delay[tokens[1]]){        
                            curr_version[tokens[1]]=tokens[0];              // taking the fastest out of the given.
                        }
                        version[tokens[1]].push_back(tokens[0]);
                        version_char[tokens[0]]=make_pair(stod(tokens[2]),stod(tokens[3]));
                    }
                }                                
                 else {                                                     // Content of circuit....

                    vector<string>back_connection(inputs.begin(),inputs.end()-1);
                    connectionsA[inputs.back()]=(make_pair(gate,back_connection));     // storage in connectionsA
                    for (size_t i = 0; i < inputs.size() - 1; i++) {
                        connectionsB[inputs[i]].push_back(make_pair(gate, inputs.back()));
                    }                    
                }          
            }            
            }
        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    // second_time....

    // SETTING UP THE THRESHHOLD VALUES FOR EACH SIGNAL......

try {
        for (const string& line : content) {
                    
            if (line.find("PRIMARY_INPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {                     
                    threshold[tokens[i]]=calc_delayB(tokens[i],timingsB,connectionsB,delay);
                }

            } 
                      
            else if (line.find("PRIMARY_OUTPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    threshold[tokens[i]]=calc_delayB(tokens[i],timingsB,connectionsB,delay);                    
                }
            } 

             else if (line.find("INTERNAL_SIGNALS") == 0) {        
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                   threshold[tokens[i]]=calc_delayB(tokens[i],timingsB,connectionsB,delay);
                }}

        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }


        // OPTIMIZE THE CIRCUIT..........

try {
        for (const string& line : content) {
                    
            if (line.find("PRIMARY_INPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {                     
                    
                }

            } 

            else if (line.find("PRIMARY_OUTPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {               
                        calc_delayA_2(tokens[i],timingsA,connectionsA,curr_version,version_char,threshold,version,new_delay,new_area);
                }
            } 

             else if (line.find("INTERNAL_SIGNALS") == 0) {        
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                  calc_delayA_2(tokens[i],timingsA,connectionsA,curr_version,version_char,threshold,version,new_delay,new_area);
                }}

        }
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }

    // CALCULATING THE RESULTING AREA!!!!

double minimized_area=0;

try {
        for (const string& line : content) {

            map<string,bool>done;
            if (line.find("PRIMARY_INPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1,end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {                     
                    new_area[tokens[i]]=0;
                    done[tokens[i]]=true;
                }

            } 
                      
            else if (line.find("PRIMARY_OUTPUTS") == 0) {       
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    if(!done[tokens[i]]){
                    minimized_area+=new_area[tokens[i]];
                    done[tokens[i]]=true;  }             
                }
            } 

             else if (line.find("INTERNAL_SIGNALS") == 0) {        
                vector<string> tokens;
                size_t start = line.find_first_of(" \t");
                tokens.push_back(line.substr(0,start));
                while (start != string::npos) {
                    size_t end = line.find_first_of(" \t", start + 1);
                    tokens.push_back(line.substr(start+1, end-start-1));
                    start = end;
                }
                for (size_t i = 1; i < tokens.size(); i++) {
                    if(!done[tokens[i]]){
                   minimized_area+=new_area[tokens[i]]; 
                   done[tokens[i]]=true;  }    
                }}

        }
          
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }


    ofstream ans_file("minimum_area.txt");
    float maxi=0;    
    if(ans_file.is_open()){
        ans_file<<minimized_area<<endl;
    }
    else{
        cerr<<"Error opening longest_delay"<<endl;
    }
    return 0;
}

