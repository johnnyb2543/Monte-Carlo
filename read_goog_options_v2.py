import yfinance as yf
import pandas as pd

ticker = yf.Ticker('GOOG')
expiration_dates = ticker.options

# Build the surface
option_data = pd.DataFrame()

for exp_date in expiration_dates:
    chain = ticker.option_chain(exp_date)
    calls = chain.calls.set_index('strike')['lastPrice']
    option_data[exp_date] = calls

# Transpose so expirations are rows, strikes are columns
option_data = option_data.T

# Drop columns with >25% missing
option_data = option_data.dropna()

# Drop rows with >25% missing
option_data = option_data.dropna(axis=1)
    
print(f"\nFinal shape: {option_data.shape[0]} expirations × {option_data.shape[1]} strikes")
print(option_data)
option_data.to_csv('option_data_v2.csv')

