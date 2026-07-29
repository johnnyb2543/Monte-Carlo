#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>
#include <iostream>
#include <iomanip>

struct OptionGrid {
    std::vector<float> strikes;
    std::vector<std::string> expiries;
    std::vector<std::vector<float>> prices;
    size_t num_rows = 0;
    size_t num_cols = 0;
    std::vector<std::vector<float>> option_prices;
};

OptionGrid load_csv(const std::string& filename) {
    OptionGrid grid;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return grid;
    }

    std::string line;

    // 1. Parse Header Row (Strikes)
    if (std::getline(file, line)) {
        int i = line.find(',') + 1; 
        
        while (i < line.length()) {
            if (line[i] != ',' && line[i] != '\r' && line[i] != '\n') {
                std::string s = "";
                while (i < line.length() && line[i] != ',') {
                    s += line[i];
                    i++;
                }
                if (!s.empty()) grid.strikes.push_back(std::stof(s));
            }
            i++;
        }
    }

    int price_row = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;

        int pos = line.find(',');
        grid.expiries.push_back(line.substr(0, pos));
        
        int i = pos + 1;
        grid.prices.emplace_back();

        while (i < line.length()) {
            if (line[i] == ',') {
                grid.prices[price_row].push_back(0.0f); // Treat as 0 or NaN
                i++;
                continue;
            }

            std::string s = "";
            while (i < line.length() && line[i] != ',') {
                s += line[i];
                i++;
            }

            try {
                if (s == "NaN" || s == "") grid.prices[price_row].push_back(0.0f);
                else grid.prices[price_row].push_back(std::stof(s));
            } catch (...) {
                grid.prices[price_row].push_back(0.0f);
            }
            i++;
        }
        price_row++;
    }
    
    grid.num_rows = grid.expiries.size();
    grid.num_cols = grid.strikes.size();
    return grid;
}

void print_option_grid(const OptionGrid& grid) {
    const int col_width = 12;
    // Only print the first 8 columns to keep it neat
    size_t display_cols = std::min(grid.strikes.size(), (size_t)8);

    // 1. Header
    std::cout << std::setw(col_width) << "Date |";
    for (size_t j = 0; j < display_cols; ++j) {
        std::cout << std::setw(col_width) << grid.strikes[j];
    }
    std::cout << "\n" << std::string(col_width * (display_cols + 1), '-') << "\n";

    // 2. Data
    for (size_t i = 0; i < grid.num_rows; ++i) {
        std::cout << std::setw(col_width - 2) << grid.expiries[i] << " |";
        for (size_t j = 0; j < display_cols; ++j) {
            std::cout << std::setw(col_width) << std::fixed << std::setprecision(2) << grid.prices[i][j];
        }
        std::cout << "\n";
    }
}
#include <cmath>
#include <ctime>
double normal_cdf(double z) {
    return 0.5 * (1.0 + std::erf(z/ std::sqrt(2.0)));
}
long days_between(std::string s1, std::string s2) {
        struct std::tm tm1 = {0}, tm2 = {0};
        std::istringstream ss1(s1), ss2(s2);

        // Parse the strings
        ss1 >> std::get_time(&tm1, "%Y-%m-%d");
        ss2 >> std::get_time(&tm2, "%Y-%m-%d");

        std::time_t time1 = std::mktime(&tm1);
        std::time_t time2 = std::mktime(&tm2);

        // Calculate difference in seconds and convert to days
        const long seconds_per_day = 60 * 60 * 24;
        return (time2 - time1) / seconds_per_day;
}
double bs_call_value(double stock_price, double strike_price, double time_delta, double int_rate, double vol) {
    double d1 = (std::log(stock_price/strike_price) + (int_rate + 0.5*vol*vol)*time_delta)/(vol*std::sqrt(time_delta));
    double d2 = (std::log(stock_price/strike_price) + (int_rate - 0.5*vol*vol)*time_delta)/(vol*std::sqrt(time_delta));
    double call_price = stock_price*normal_cdf(d1) - strike_price*std::exp(int_rate*time_delta)*normal_cdf(d2);
    return call_price;
}
double bs_vega(double stock_price, double strike_price, double time_delta, double int_rate, double vol) {
    double d1 = (std::log(stock_price/strike_price) + (int_rate + 0.5*vol*vol)*time_delta)/(vol*std::sqrt(time_delta));
    double vega = stock_price*normal_cdf(d1)*std::sqrt(time_delta);
    return vega;
}
int main() {
    std::string path = "/Users/johnboncore/Desktop/finproj/implied_vols_v1.csv";
    std::cout << "Loading: " << path << "..." << std::endl;
    OptionGrid option_grid = load_csv(path);
    print_option_grid(option_grid);
    option_grid.option_prices.resize(option_grid.num_rows, std::vector<float>(option_grid.num_cols, 0.0f));
    auto start = std::chrono::high_resolution_clock::now();
    
    /*Need datetime to calculate volatility, time
    Im going to run Newton_Rapson to find the implied volatiltiy of these Google options
    I need volatility, time, and interest rate
    time can be found using std::chrono library
    interest rate can be found on fred using the current t-bill rate, scaled to the time to expiry
    The Newton Raphson method requires an innital guess for volatility
    I will use the 90-day historic volatility from May 12, 2026, which I found to be about 0.2
    */

    for (int i = 0; i < option_grid.num_rows; i++) {
        //std::cout << "Got here.\n";
        double time_delta = days_between("2026-05-12",option_grid.expiries[i])/365.0;
        double annual_rate = 0.07;
        double annual_vol = 0.2;
        double interest_rate = annual_rate;
        double volatility = annual_vol;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::normal_distribution<double> d(0.0, 1.0);

        for (int j = 0; j < option_grid.num_cols; j++) {
            //std::cout << "Got here.\n";

            double stock_price = 382.09;
            double strike_price = option_grid.strikes[j];

            constexpr int iterations = 100000;
            
            int option_count = (i * option_grid.num_cols) + j;
            std::array<double,iterations> random_array{};
            double total_payoff {};
            for (int k = 0; k < iterations; k++) {
                std::cout << "Option #" << option_count << " | NR Iteration: " << k << std::endl;
                double sample = d(gen);
                double random_price = stock_price * std::exp((interest_rate - 0.5 * volatility * volatility) * time_delta + volatility * std::sqrt(time_delta) * sample);
                double option_payoff = std::max(random_price - strike_price, 0.0);
                total_payoff += option_payoff;
            }
            double mean_payoff = total_payoff / iterations;
            double call_price = std::exp(-interest_rate * time_delta) * mean_payoff;
            option_grid.option_prices[i][j] = call_price;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Load time: " << elapsed.count() << " seconds." << std::endl;
    for (int i = 0; i < option_grid.option_prices.size(); i++) {
        std::cout << "Option " << i << " price: " << option_grid.option_prices[i] << '\n';
    }
    return 0;
}


