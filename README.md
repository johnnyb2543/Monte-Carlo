This project entailed writing a Monte Carlo simulation from scratch in CUDA. I was able to import the data using the yfinance API in Python and then I imported the data from a csv. 
Simulating a stock price over a given period of time involves generating many random numbers to use as 'seeds' that generate many random stock prices.
I simulated over 100,000 stock prices for close to 1,000 options and achieved approximately 1,000 speedup when transforming the calculation from a CPU-based version to a GPU-oriented one.
I focused the stock price simulation on GOOG for a specific date, expiration date, and combinations of strike prices and expiries.
Please see the .py file for the data generation, the .cpp file for the CPU computation, and the .ipynb file for the GPU computation.
Key challenges
- implementing the random number generation
- organizing the kernels and data access to implement a parallel sum
- debugging
