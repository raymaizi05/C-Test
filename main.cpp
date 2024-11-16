#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <iomanip>
#include <limits>
#include <map>

using namespace std;

struct TradeStats {
    long long maxTimeGap = 0;
    long long totalVolume = 0;
    long long totalPriceQuantity = 0; // Sum of price * quantity for weighted average price
    long long totalQuantity = 0;      // Total quantity for weighted average price
    int maxPrice = 0;
    long long lastTimestamp = -1;     // To calculate time gap
};

void processTrade(const std::string& line, map<std::string, TradeStats>& symbolData) {
    std::stringstream ss(line);
    long long timestamp;
    string symbol;
    long long quantity;
    int price;
    string temp;


    std::getline(ss, temp, ',');
    timestamp = std::stoll(temp);

    std::getline(ss, symbol, ',');

    std::getline(ss, temp, ',');
    quantity = std::stoll(temp);

    std::getline(ss, temp, ',');
    price = std::stoll(temp);

    auto& stats = symbolData[symbol];

    // Update max time gap
    if (stats.lastTimestamp != -1) {
        stats.maxTimeGap = std::max(stats.maxTimeGap, timestamp - stats.lastTimestamp);
    }
    stats.lastTimestamp = timestamp;

    // Update total volume
    stats.totalVolume += quantity;

    // Update weighted average price
    stats.totalPriceQuantity += static_cast<long long>(price) * quantity;
    stats.totalQuantity += quantity;

    // Update max price
    stats.maxPrice = std::max(stats.maxPrice, price);
}

void writeOutput(const map<std::string, TradeStats>& symbolData, const std::string& outputFile) {
    std::ofstream out(outputFile);

    // Header
    out << "symbol,MaxTimeGap,Volume,WeightedAveragePrice,MaxPrice\n";

    // Collect symbols in sorted order
    std::vector<std::string> symbols;
    for (const auto& entry : symbolData) {
        symbols.push_back(entry.first);
    }
    std::sort(symbols.begin(), symbols.end());

    for (const auto& symbol : symbols) {
        const auto& stats = symbolData.at(symbol);
        int weightedAveragePrice = stats.totalQuantity > 0
                                   ? static_cast<int>(std::floor(stats.totalPriceQuantity / stats.totalQuantity))
                                   : 0;

        out << symbol << ',' << stats.maxTimeGap << ',' << stats.totalVolume << ',' << weightedAveragePrice << ','
            << stats.maxPrice << '\n';
    }

    out.close();
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.csv> <output.csv>\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];

    std::ifstream in(inputFile);
    if (!in.is_open()) {
        std::cerr << "Error: Could not open input file " << inputFile << "\n";
        return 1;
    }

    map<std::string, TradeStats> symbolData;
    std::string line;

    // Read and process each line
    while (std::getline(in, line)) {
        processTrade(line, symbolData);
    }
    in.close();

    // Write results to output file
    writeOutput(symbolData, outputFile);

    return 0;
}