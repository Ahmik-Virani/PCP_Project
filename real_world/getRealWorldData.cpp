#include<iostream>
#include<vector>
#include<string>
#include<fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

using namespace std;

struct Tick{
    string symbol_from;
    string symbol_to;
    string timestamp;
    double bid;
    double ask;
};


class getRealData {
private:

    vector<string> fileNames;   // the data csv files we will read

    // symbol timestamp bid ask
    vector<Tick> data;

    bool splitSymbolPair(const string& rawSymbol, string& from, string& to) {
        string s;
        s.reserve(rawSymbol.size());
        for (char ch : rawSymbol) {
            if (!isspace(static_cast<unsigned char>(ch))) s.push_back(ch);
        }
        size_t slashPos = s.find('/');
        if (slashPos != string::npos) {
            from = s.substr(0, slashPos);
            to = s.substr(slashPos + 1);
            return !from.empty() && !to.empty();
        }

        if (s.size() == 6) {
            from = s.substr(0, 3);
            to = s.substr(3, 3);
            return true;
        }

        return false;
    }

public:
    getRealData() {
        // store the files names
        fileNames.push_back("CHFJPY-2025-12.csv");
        fileNames.push_back("USDZAR-2025-12.csv");
        fileNames.push_back("EURUSD-2025-12.csv");
        fileNames.push_back("EURCHF-2025-12.csv");
        fileNames.push_back("GBPUSD-2025-12.csv");
        fileNames.push_back("CADJPY-2025-12.csv");
        fileNames.push_back("EURGBP-2025-12.csv");
        fileNames.push_back("GBPJPY-2025-12.csv");
        fileNames.push_back("EURJPY-2025-12.csv");
        fileNames.push_back("AUDNZD-2025-12.csv");
        fileNames.push_back("EURPLN-2025-12.csv");
        fileNames.push_back("NZDUSD-2025-12.csv");
        fileNames.push_back("USDMXN-2025-12.csv");
        fileNames.push_back("USDJPY-2025-12.csv");
        fileNames.push_back("AUDUSD-2025-12.csv");
        fileNames.push_back("USDCAD-2025-12.csv");
        fileNames.push_back("AUDJPY-2025-12.csv");
        fileNames.push_back("USDCHF-2025-12.csv");
        fileNames.push_back("USDTRY-2025-12.csv");

        for (const string& fileName : fileNames) {
            ifstream input_data("December_Data/" + fileName);  // folder + file name
            if (!input_data.is_open()) {
                cerr << "Could not open: " << fileName << '\n';
                continue;
            }

            string line;
            while (getline(input_data, line)) {
                if (line.empty()) continue;

                string sym, ts, bidStr, askStr;
                stringstream ss(line);

                if (!getline(ss, sym, ',')) continue;
                if (!getline(ss, ts, ',')) continue;
                if (!getline(ss, bidStr, ',')) continue;
                if (!getline(ss, askStr, ',')) continue;

                try {
                    double bid = stod(bidStr);
                    double ask = stod(askStr);
                    string from, to;
                    if (!splitSymbolPair(sym, from, to)) continue;

                    data.push_back({from, to, ts, bid, ask});
                } catch (...) {
                    continue;   // malformed number
                }
            }
        
            // sort the data is timestamp manner
            sort(data.begin(), data.end(),
                [](const Tick& a, const Tick& b) {
                    return a.timestamp < b.timestamp;
                });
        }
    }

    vector<Tick> getData(){
        return data;
    }

    // get at a particluar index
    Tick getData(int i){
        return data[i];
    }

    long long getSizeOfData(){
        return data.size();
    }
};