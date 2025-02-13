import csv
import pandas as pd
from alpha_vantage.timeseries import TimeSeries

# 使用您的 API 密钥
api_key = "0BG3MGW5K758HJ2C"

# 创建 Alpha Vantage API 实例
ts = TimeSeries(key=api_key, output_format="pandas")

# 创建所需要的股票列表
stocks = ["AAPL", "GOOG", "NVDA", "TSLA"]

# 用于存储所有股票的历史数据
stock_data = {}

# 循环遍历股票列表，获取每个股票的数据
for stock in stocks:
    try:
        # 获取股票的日常时间序列数据（历史数据）
        data, meta_data = ts.get_daily(
            symbol=stock, outputsize="full"
        )  # 'full' 获取完整历史数据
        # 将数据保存到 CSV 文件
        data.to_csv(f"{stock}_historical_data.csv")
        print(f"成功获取 {stock} 的数据，并保存到 {stock}_historical_data.csv")
    except Exception as e:
        print(f"获取 {stock} 数据失败：{e}")
