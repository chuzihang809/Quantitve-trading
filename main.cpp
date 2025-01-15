#include<iostream>
#include<vector>
#include <numeric>
#include<fstream>
#include<cstring>
#include <thread>
#include <chrono>

using namespace std;

class trading
{
private:

	char name[20];//交易股票名字或代码
	int shortperiod;//经济学中的短期交易周期，经验给出，用户可变更
	int longperiod;//经济学中的长期交易周期，经验给出，用户可变更
	double max_money;//最大交易金额,交易单位为人民币
	double MAX_quantity;//一次性交易数量，用户可变更
	double todayprice;
	double rating=0;//权重，用于设置购买数量
	double quantity;//实际购买数量
	int zhuangtaiji=1;//利用状态机实现在一段时间内只对shortAVG第一次大于longAVG时进行买入
	vector<double> last_prices;//用于存储此后价格的数组
	

public:

	trading();
	void set_period(int shortp,int longp);
	void get_todayPrices();//获取实时价格
	void set_name(const char *);
	double calculateAVG(int);
	void trade();//判断是否交易的函数
	void buy_in_out(bool flag);//执行交易函数
	void log_trading(const char action[]);//将每次交易信息写入文档，便于查询
	bool riskcontrol();//判断是否超出最大交易金额
	double get_sellquantity();//通过权重计算交易数量
	double get_buyquantity();
	void run();//运行程序
};

trading::trading()
{
	strcpy_s(name, "NASDAQ");
	shortperiod = 5;
	longperiod = 20;//基于经验分别将短期长期设置为5天和20天
	max_money = 100000;
	quantity = 10;//将交易数初始化为10，后续根据权重计算

	for (int i = 0; i < longperiod; ++i) //提前给出20天的价格数据
	{
		double price = 100.0 + (rand() % 100);
		last_prices.push_back(price);
	}
}

void trading::set_name(const char *inname)
{
	strcpy_s(name, inname);
}

void trading::set_period(int shortp,int longp)//短期长期设置
{
	shortperiod = shortp;
	longperiod = longp;
}

void trading::get_todayPrices()//直接利用生成随机数的方法获取今日价格，实则应该通过脚本读取
{
	todayprice = 100.0 + (rand() % 100);
	last_prices.push_back(todayprice);
}

double trading::calculateAVG(int period)
{
	double AVG = accumulate(last_prices.end() - period, last_prices.end(), 0.0) / period;//计算均线
	return AVG;
}

void trading::trade()//在一段时间内第一次shortAVG > longAVG执行buy操作，shortAVG < longAVG执行sell操作
{
	double shortAVG = calculateAVG(shortperiod);
	double longAVG = calculateAVG(longperiod);

	cout << "Today's shortAVG:" << shortAVG << ' ' << "Today's longAVG:" << longAVG << endl;
	if (shortAVG > longAVG&&zhuangtaiji==1)
	{
		buy_in_out(true);
		zhuangtaiji = 0;
		log_trading("Buy");
		quantity = get_sellquantity();
	}
	else if (shortAVG < longAVG&&zhuangtaiji==0)
	{
		buy_in_out(false);
		zhuangtaiji = 1;
		log_trading("Sell");
		quantity = get_buyquantity();
	}
}

void trading::buy_in_out(bool flag)
{
	if (flag) cout << "In day:" << last_prices.size() << ' ' 
		<< "buy in" << ' ' <<quantity<<'*'<< todayprice << endl;
	else  cout<< "In day:" << last_prices.size() << ' ' 
		<< "sell in" << ' ' << quantity << '*'<<todayprice << endl;
}

double trading::get_sellquantity()
{
	int count = 0;
	int i;
	for (i = 0; i < last_prices.size(); i++)
	{
		if (todayprice <= last_prices[i]) count++;

	}
	double rating = count * 1.0 / last_prices.size();
	return rating * 10;
}

double trading::get_buyquantity()
{
	int count = 0;
	int i;
	for (i = 0; i < last_prices.size(); i++)
	{
		if (todayprice >= last_prices[i]) count++;

	}
	double rating = count * 1.0 / last_prices.size();
	return rating * 10;
}

void trading::log_trading(const char action[])
{
	if (strcmp(action, "buy") == 0)
	{
		ofstream outputFile;
		outputFile.open("tradinglog.txt");
		if (!outputFile) 
		{
			cout << "Unable to open file tradinglog.txt";
		}
		outputFile<< "In day:" << last_prices.size() << ' ' << "buy in" 
			<< ' ' << quantity << '*' << get_buyquantity() << endl;
	}
	else if ((strcmp(action, "Sell") == 0))
	{
		ofstream outputFile;
		outputFile.open("tradinglog.txt");
		if (!outputFile)
		{
			cout << "Unable to open file tradinglog.txt";
		}
		outputFile << "In day:" << last_prices.size() << ' '
			<< "sell in" << ' ' << get_sellquantity() << '*' << todayprice << endl;
	}
}

bool trading::riskcontrol() {
	return (todayprice * quantity) <= max_money;
}

void trading::run() {
    while (true) {
        get_todayPrices(); // 获取今日价格并存入向量
        trade(); // 判断是否交易
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟每天获取一次价格
        if (cin.get() == 'q') break; // 按键退出系统
    }
}

int main()
{
	cout << "开始运行，请按q退出！" << endl;
	trading trade;
	trade.run(); // 开始运行系统
	return 0;
}
