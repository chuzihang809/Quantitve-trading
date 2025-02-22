#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
enum TradingState { BUY = 0, SELL = 1 };
class StockData {
private:
  string filename;

public:
  StockData(const string &file) : filename(file) {}

  vector<double> getStockPrices() {
    vector<double> prices;
    ifstream file(filename);

    if (!file.is_open()) {
      cout << "Unable to open file " << filename << endl;
      return prices;
    }

    string line;
    getline(file, line); // 跳过CSV文件的标题行

    while (getline(file, line)) {
      stringstream ss(line);
      string date, open, high, low, close, adj_close, volume;

      // 解析CSV每行
      getline(ss, date, ',');
      getline(ss, open, ',');
      getline(ss, high, ',');
      getline(ss, low, ',');
      getline(ss, close, ',');
      getline(ss, adj_close, ',');
      getline(ss, volume, ',');

      prices.push_back(stod(close)); // 将收盘价转为 double 类型并加入到价格数组
    }
    file.close();
    return prices;
  }
};
class Logger {
private:
  ofstream output;
  mutex logMutex; // 互斥锁，用于保护线程安全
public:
  Logger(const string &filename) {
    output.open(filename, ios::app);
    if (!output)
      cout << "Unable to open log file" << endl;
  }
  void logTrade(const string &action, const string &name, double quantity,
                double price) {
    lock_guard<mutex> lock(logMutex); // 加锁，确保线程安全
    output << "Action: " << action << ", Stock: " << name
           << ", Quantity: " << quantity << ", 	Price: " << price << endl;
  }
};
class Trading {
private:
  string name;        // 交易股票名字或代码
  int shortPeriod;    // 经济学中的短期交易周期，经验给出，用户可变更
  int longPeriod;     // 经济学中的长期交易周期，经验给出，用户可变更
  double maxMoney;    // 最大交易金额,交易单位为人民币
  double maxQuantity; // 最大交易数量，用户可变更
  double todayPrice; // 用于检测数据是否更新
  double quantity; // 实际购买数量
  TradingState state = SELL; // 利用状态机实现在一段时间内只对shortAVG第一次大于longAVG时进行买入
  vector<double> lastPrices; // 用于存储此后价格的数组
  StockData &stockData;
  Logger &logger;
  double calculateRating(bool isBuy) {
        int count = 0;
        for (double price : lastPrices) {
            if (isBuy ? (todayPrice >= price) : (todayPrice <= price)) {
                count++;
            }
        }
        return (count*1.0) / lastPrices.size();
    }

public:
  Trading(StockData &sd, Logger &lg)
      : name("AAPL"), shortPeriod(5), longPeriod(20), maxMoney(100000),
        maxQuantity(100), quantity(50), stockData(sd), logger(lg) {}
  void set_period(int shortp, int longp) {
    shortPeriod = shortp;
    longPeriod = longp;
  }
  
  void updatePrices() {
    lastPrices = stockData.getStockPrices();
    if (!lastPrices.empty()) {
      todayPrice = lastPrices.back();
    }
  } // 获取实时价格
  void set_name(const string &inname) { name = inname; }
  double calculateAVG(int period) {
    return accumulate(lastPrices.end() - period, lastPrices.end(), 0.0) / period; // 计算均线
  }
  void executeTrade(bool isBuy) {
    const string action = isBuy ? "BUY" : "SELL";
    cout << "Day:" << lastPrices.size() << ' ' << action << ' ' << name << ' '
         << quantity << " * " << todayPrice << endl;
    logger.logTrade(action, name, quantity, todayPrice);
  } // 判断是否交易的函数
  void whetherTrade() {
        if (lastPrices.size() < static_cast<size_t>(longPeriod)) {
            cerr << "Not enough data! " << endl;
            return;
        }
        double shortAVG = calculateAVG(shortPeriod);
        double longAVG = calculateAVG(longPeriod);

        cout << name << " ShortAVG: " << shortAVG 
             << ", LongAVG: " << longAVG << endl;

        if (shortAVG > longAVG && state == SELL) {
            quantity = maxQuantity * calculateRating(true);
            if (quantity * todayPrice <= maxMoney) {
                executeTrade(true);
                state = BUY;
            }
        } 
		else if (shortAVG < longAVG && state == BUY) {
            quantity = maxQuantity * calculateRating(false);
            executeTrade(false);
            state = SELL;
        }
    } // 实际执行交易的函数

  void run() {
  while (true) {
    updatePrices();                          // 获取今日价格并存入向量
    whetherTrade();                                    // 判断是否交易
    this_thread::sleep_for(chrono::hours(24)); // 模拟每天获取一次价格
    if (cin.get() == 'q')
      break; // 按键退出系统
  }
}; // 运行程序
};

int main() {
  	cout << "Starting trading system (press 'q' to exit)..." << endl;
  	StockData stockData("读取文件/AAPL_historical_data.csv");
    Logger logger("trading_log.txt");
    Trading tradingSystem(stockData, logger);
    tradingSystem.run();
  	return 0;
}
