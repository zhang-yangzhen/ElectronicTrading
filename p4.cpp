#include<string>
#include<iostream>
#include<map>
#include<set>
#include<queue>
#include<fstream>
#include<cstdlib>
#include<getopt.h>
using namespace std;

typedef struct compare_buy {
    int price, id;
    friend bool operator<(struct compare_buy a, struct compare_buy b) {
        return (a.price > b.price || (a.price == b.price && a.id < b.id));
    }
}Comp_buy;

typedef struct compare_sell {
    int price, id;
    friend bool operator<(struct compare_sell a, struct compare_sell b) {
        return (a.price < b.price || (a.price == b.price && a.id < b.id));
    }
}Comp_sell;

typedef struct order_struct {
    int id, price, quantity, due, timestamp;
    bool sell;
    string client, symbol;
}Order;

typedef struct compare_to_buy {
    bool operator()(const Order &a, const Order &b) {
        return (a.price < b.price || (a.price == b.price && a.id > b.id));
    }
}Comp_to_buy;

typedef struct compare_to_sell {
    bool operator()(const Order &a, const Order &b) {
        return (a.price > b.price || (a.price == b.price && a.id > b.id));
    }
}Comp_to_sell;

typedef struct client_struct {
    string name;
    int bought, sold, value;
}Merchant;

typedef struct traveler_struct {
    string symbol;
    int time_buy, time_sell;
    int benefit;
}Traveler;

//structure for fast getting median
class median_heap {
private:
    struct min_comp {
        bool operator()(const int &a, const int &b) {
            return (a > b);
        }
    };
    struct max_comp {
        bool operator()(const int &a, const int &b) {
            return (a < b);
        }
    };
    priority_queue<int, vector<int>, min_comp> *min_heap;
    priority_queue<int, vector<int>, max_comp> *max_heap;
public:
    median_heap() {
        min_heap = new priority_queue<int,vector<int>, min_comp>;
        max_heap = new priority_queue<int,vector<int>, max_comp>;
    }
    void insert(int value) {
        if (max_heap->empty()) {
            max_heap->push(value);
            return;
        }
        if (min_heap->empty()) {
            if (value > max_heap->top())
                min_heap->push(value);
            else {
                min_heap->push(max_heap->top());
                max_heap->pop();
                max_heap->push(value);
            }
            return;
        }
        if (value < max_heap->top()) {
            if (max_heap->size() <= min_heap->size()) {
                max_heap->push(value);
            }
            else {
                min_heap->push(max_heap->top());
                max_heap->pop();
                max_heap->push(value);
            }
        }
        else if (min_heap->top() < value) {
            if (min_heap->size() <= max_heap->size()) {
                min_heap->push(value);
            }
            else {
                max_heap->push(min_heap->top());
                min_heap->pop();
                min_heap->push(value);
            }
        }
        else {
            if (max_heap->size() < min_heap->size()) max_heap->push(value);
            else min_heap->push(value);
        }
        return;
    }
    int get_median() {
        if (max_heap->size() < min_heap->size()) {
            return min_heap->top();
        }
        else if (max_heap->size() > min_heap->size()) {
            return max_heap->top();
        }
        else {
            return (min_heap->top()+max_heap->top())/2;
        }
    }
};

int main(int argc, char **argv)
{
    ifstream fin;
    fin.open("test.in");

    bool verbose = false;
    bool f_mid = false;
    bool f_median = false;
    bool f_transfer = false;
    Traveler t;
    vector<Traveler>* TTT = new vector<Traveler>;
    t.benefit = 0;

    static struct option long_options[] = {
        {"verbose", no_argument, 0, 'v'},
		{"median", no_argument, 0, 'm'},
		{"midpoint", no_argument, 0, 'p'},
		{"transfers", no_argument, 0, 't'},
		{"ttt", required_argument, 0, 'g'},
		{0, 0, 0, 0}
    };
    int f;
    string i_type;
    while ((f = getopt_long(argc,argv,"g:mptv",long_options,NULL)) != -1) {
        switch (f) {
        case 'v':
            verbose = true;
            break;
        case 'm':
            f_median = true;
            break;
        case 'p':
            f_mid = true;
            break;
        case 't':
            f_transfer = true;
            break;
        case 'g':
            i_type = optarg;
            t.symbol = i_type;
            TTT->push_back(t);
            break;
        }
    }

    typedef map<string,map<Comp_sell, Order>*> sell_map;
    typedef map<string,map<Comp_buy, Order>*> buy_map;
    typedef map<string,priority_queue<Order, vector<Order>, Comp_to_buy>*> to_buy_map;
    typedef map<string,priority_queue<Order, vector<Order>, Comp_to_sell>*> to_sell_map;

    to_sell_map* tosell = new to_sell_map;
    to_buy_map* tobuy = new to_buy_map;

    sell_map* sell = new sell_map;
    buy_map* buy = new buy_map;

    map<string,Merchant>* transfer = new map<string, Merchant>;
    set<string> midpoint;
    map<string, median_heap*> median;
    median_heap* prices;
    int id = 0;
    int timestramp, duration;
    string buy_sell, client, symbol, price, quantity;
    Order o, p;
    Comp_sell c_s;
    Comp_buy c_b;
    Merchant m;
    int earn = 0;
    int money_tran = 0;
    int trades = 0;
    int shares = 0;
    int curr = 0;
    int tradeprice;
    bool traded = false;

    while (fin >> timestramp >> client >> buy_sell >> symbol >> price >> quantity >> duration) {
        if (timestramp>curr) {
            if (f_median) {
                for (map<string,median_heap*>::iterator iter = median.begin(); iter != median.end(); iter++) {
                    cout << "Median match price of " << iter->first << " at time " << curr << " is $" << iter->second->get_median() << endl;
                }
            }
            if (f_mid) {
                for (set<string>::iterator iter = midpoint.begin(); iter != midpoint.end(); iter++) {
                    int maxbuy = -1;
                    int minsell = -1;
                    if (tobuy->find(*iter) != tobuy->end()) {
                        p.due = -2;
                        priority_queue<Order, vector<Order>, Comp_to_buy>* newbuy = (*tobuy)[*iter];
                        if (!newbuy->empty()) {
                            p = newbuy->top();
                        }
                        while (p.due <= curr && p.due != -1 && !newbuy->empty()) {
                            newbuy->pop();
                            if (!newbuy->empty()) {
                                p = newbuy->top();
                            }
                        }
                        if (p.due > curr || p.due == -1) {
                            maxbuy = p.price;
                        }
                    }
                    if (tosell->find(*iter) != tosell->end()) {
                        p.due = -2;
                        priority_queue<Order, vector<Order>, Comp_to_sell>* newsell = (*tosell)[*iter];
                        if (!newsell->empty()) {
                            p = newsell->top();
                        }
                        while (p.due<=curr && p.due!=-1 && !newsell->empty()) {
                            newsell->pop();
                            if (!newsell->empty()) {
                                p = newsell->top();
                            }
                        }
                        if (p.due > curr || p.due == -1) {
                            minsell = p.price;
                        }
                    }
                    if (minsell != -1 && maxbuy != -1) {
                        cout << "Midpoint of " << *iter << " at time " << curr << " is $" << (maxbuy+minsell)/2 << endl;
                    }
                    else {
                        cout << "Midpoint of " << *iter << " at time " << curr << " is undefined" << endl;
                    }
                }
            }
            curr = timestramp;
        }

        o.id = id;
        id++;
        o.timestamp = timestramp;
        o.client = client;
        if (duration != -1) {
            o.due = timestramp + duration;
        }
        else {
            o.due = -1;
        }
        o.price = atoi(price.substr(1, price.length()-1).c_str());
        o.quantity = atoi(quantity.substr(1, quantity.length()-1).c_str());
        if (buy_sell == "SELL") {
            o.sell = true;
        }
        else {
            o.sell = false;
        }
        o.symbol = symbol;
        if (midpoint.find(o.symbol) == midpoint.end()) {
            midpoint.insert(symbol);
        }
        if (o.sell) {
            c_s.price = o.price;
            c_s.id = o.id;
        }
        else {
            c_b.price = o.price;
            c_b.id = o.id;
        }
        for (vector<Traveler>::iterator ter = TTT->begin(); ter!=TTT->end(); ter++) {
            if (o.symbol == ter->symbol) {
                if (!o.sell && sell->find(o.symbol) != sell->end()) {
                    map<Comp_sell,Order>* tempsell = (*sell)[o.symbol];
                    for (map<Comp_sell, Order>::iterator jter = tempsell->begin(); jter != tempsell->end(); jter++) {
                        if (o.price-jter->second.price > ter->benefit) {
                            ter->time_buy = jter->second.timestamp;
                            ter->time_sell = o.timestamp;
                            ter->benefit = o.price-jter->second.price;
                        }
                    }
                }
            }
        }
        map<string, Merchant>::iterator cter = transfer->find(o.client);
        if (cter == transfer->end()) {
            m.name = o.client;
            m.bought = 0;
            m.sold = 0;
            m.value = 0;
            transfer->insert(pair<string, Merchant>(o.client, m));
        }
        cter = transfer->find(o.client);

        if (o.sell) {
            if (tobuy->find(o.symbol) != tobuy->end()) {
                priority_queue<Order, vector<Order>, Comp_to_buy>* newbuy = (*tobuy)[o.symbol];
                if (!newbuy->empty()) {
                    p=newbuy->top();
                    while (p.due != -1 && p.due <= timestramp && !newbuy->empty()) {
                        newbuy->pop();
                        if (!newbuy->empty()) {
                            p = newbuy->top();
                        }
                    }
                    while (p.price >= o.price && o.quantity > 0 && !newbuy->empty()) {
                        traded = false;
                        if (p.quantity >= o.quantity && o.quantity > 0) {
                            if (verbose) {
                                cout << p.client << " purchased " << o.quantity << " shares of " << o.symbol << " from " << o.client << " for $" << p.price << "/share" << endl;
                            }
                            tradeprice = p.price;
                            cter->second.sold = cter->second.sold + o.quantity;
                            cter->second.value = cter->second.value + o.quantity * p.price;
                            map<string, Merchant>::iterator cter2 = transfer->find(p.client);
                            cter2->second.bought = cter2->second.bought + o.quantity;
                            cter2->second.value = cter2->second.value - o.quantity * p.price;
                            trades++;
                            shares = shares + o.quantity;
                            earn = earn + (o.quantity * p.price / 100);
                            earn = earn + (o.quantity * p.price / 100);
                            money_tran = money_tran + o.quantity * p.price;
                            p.quantity = p.quantity - o.quantity;
                            o.quantity = 0;
                            if (p.quantity > 0) {
                                newbuy->pop();
                                newbuy->push(p);
                            }
                            else {
                                newbuy->pop();
                            }
                            traded = true;
                        }
                        else if (p.quantity < o.quantity && p.quantity > 0) {
                            if (verbose) {
                                cout << p.client << " purchased " << p.quantity << " shares of " << o.symbol << " from " << o.client << " for $" << p.price << "/share" << endl;
                            }
                            tradeprice = p.price;
                            cter->second.sold = cter->second.sold + p.quantity;
                            cter->second.value = cter->second.value + p.quantity * p.price;
                            map<string, Merchant>::iterator cter2 = transfer->find(p.client);
                            cter2->second.bought = cter2->second.bought + p.quantity;
                            cter2->second.value = cter2->second.value - p.quantity * p.price;

                            trades++;
                            shares = shares + p.quantity;
                            earn = earn + (p.quantity * p.price / 100);
                            earn = earn + (p.quantity * p.price / 100);
                            money_tran = money_tran + p.quantity * p.price;

                            o.quantity = o.quantity - p.quantity;
                            newbuy->pop();
                            if (!newbuy->empty()) {
                                p = newbuy->top();
                            }
                            while (p.due <= timestramp && p.due != -1 && !newbuy->empty()) {
                                newbuy->pop();
                                if (!newbuy->empty()) {
                                    p = newbuy->top();
                                }
                            }
                            traded = true;
                        }

                        if (f_median) {
                            if (median.find(o.symbol) != median.end() && traded) {
                                median[o.symbol]->insert(tradeprice);
                            }
                            else if (traded) {
                                prices = new median_heap;
                                prices->insert(tradeprice);
                                median[o.symbol] = prices;
                            }
                        }
                    }
                }
            }

            if (sell->find(o.symbol) == sell->end()) {
                map<Comp_sell, Order>* new_sell = new map<Comp_sell, Order>;
                new_sell->insert(pair<Comp_sell, Order> (c_s, o));
                (*sell)[o.symbol] = new_sell;
            }
            else {
                (*sell)[o.symbol]->insert(pair<Comp_sell, Order>(c_s, o));
            }
            if (o.quantity > 0 && o.due != timestramp) {
                if (tosell->find(o.symbol) == tosell->end()) {
                    priority_queue<Order, vector<Order>, Comp_to_sell>* new_to_sell = new priority_queue<Order, vector<Order>, Comp_to_sell>;
                    new_to_sell->push(o);
                    (*tosell)[o.symbol] = new_to_sell;
                }
                else {
                    (*tosell)[o.symbol]->push(o);
                }
            }

        }
        else {
            if (tosell->find(o.symbol) != tosell->end()) {
                priority_queue<Order, vector<Order>, Comp_to_sell>* newsell = (*tosell)[o.symbol];
                if (!newsell->empty()) {
                    p = newsell->top();
                    while (p.due != -1 && p.due <= timestramp && !newsell->empty()) {
                        newsell->pop();
                        if (!newsell->empty()) {
                            p = newsell->top();
                        }
                    }
                    while (p.price <= o.price && o.quantity > 0 && !newsell->empty()) {
                        traded = false;
                        if (p.quantity >= o.quantity && o.quantity > 0) {
                            if (verbose) {
                                cout << o.client << " purchased " << o.quantity << " shares of " << o.symbol << " from " << p.client << " for $" << p.price << "/share" << endl;
                            }
                            tradeprice = p.price;
                            cter->second.bought = cter->second.bought + o.quantity;
                            cter->second.value = cter->second.value - o.quantity * p.price;
                            map<string, Merchant>::iterator cter2 = transfer->find(p.client);
                            cter2->second.sold = cter2->second.sold + o.quantity;
                            cter2->second.value = cter2->second.value + o.quantity * p.price;
                            trades++;
                            shares = shares + o.quantity;
                            earn = earn + (o.quantity * p.price / 100);
                            earn = earn + (o.quantity * p.price / 100);
                            money_tran = money_tran + o.quantity * p.price;

                            p.quantity = p.quantity - o.quantity;
                            if (p.quantity > 0) {
                                newsell->pop();
                                newsell->push(p);
                            }
                            else {
                                newsell->pop();
                            }
                            o.quantity = 0;
                            traded = true;
                        }
                        else if (p.quantity < o.quantity && p.quantity > 0) {
                            if (verbose) {
                                cout << o.client << " purchased " << p.quantity << " shares of " << o.symbol << " from " << p.client << " for $" << p.price << "/share" << endl;
                            }
                            tradeprice = p.price;
                            cter->second.bought = cter->second.bought + p.quantity;
                            cter->second.value = cter->second.value - p.quantity * p.price;
                            map<string, Merchant>::iterator cter2 = transfer->find(p.client);
                            cter2->second.sold = cter2->second.sold + p.quantity;
                            cter2->second.value = cter2->second.value + p.quantity * p.price;

                            trades++;
                            shares = shares + p.quantity;
                            earn = earn + (p.quantity * p.price / 100);
                            earn = earn + (p.quantity * p.price / 100);
                            money_tran = money_tran + p.quantity * p.price;

                            o.quantity = o.quantity - p.quantity;
                            p.quantity = 0;
                            newsell->pop();
                            if (!newsell->empty()) {
                                p = newsell->top();
                            }
                            while (p.due != -1 && p.due <= timestramp && !newsell->empty()) {
                                newsell->pop();
                                if (!newsell->empty()) {
                                    p = newsell->top();
                                }
                            }
                            traded = true;
                        }
                        if (f_median) {
                            if (median.find(o.symbol) != median.end() && traded) {
                                median[o.symbol]->insert(tradeprice);
                            }
                            else if (traded) {
                                prices = new median_heap();
                                prices->insert(tradeprice);
                                median[o.symbol] = prices;
                            }
                        }
                    }
                }
            }

            if (buy->find(o.symbol) == buy->end()) {
                map<Comp_buy, Order>* new_buy = new map<Comp_buy, Order>;
                new_buy->insert(pair<Comp_buy, Order>(c_b, o));
                buy->insert(pair<string, map<Comp_buy, Order>*>(o.symbol, new_buy));
            }
            else {
                (*buy)[o.symbol]->insert(pair<Comp_buy, Order>(c_b, o));
            }
            if (o.quantity > 0 && o.due != timestramp) {
                if (tobuy->find(o.symbol) == tobuy->end()) {
                    priority_queue<Order, vector<Order>, Comp_to_buy>* new_to_buy = new priority_queue<Order, vector<Order>, Comp_to_buy>;
                    new_to_buy->push(o);
                    (*tobuy)[o.symbol] = new_to_buy;
                }
                else {
                    (*tobuy)[o.symbol]->push(o);
                }
            }
        }
    }

    curr = timestramp;
    if (f_median) {
        for (map<string, median_heap*>::iterator iter = median.begin(); iter != median.end(); iter++) {
            cout << "Median match price of " << iter->first << " at time " << curr << " is $" << iter->second->get_median() << endl;
        }
    }
    if (f_mid) {
        for (set<string>::iterator iter = midpoint.begin(); iter != midpoint.end(); iter++) {
            int maxbuy=-1;
            int minsell=-1;
            if (tobuy->find(*iter) != tobuy->end()) {
                priority_queue<Order, vector<Order>, Comp_to_buy>* newbuy = (*tobuy)[*iter];
                p.due = -2;
                if (!newbuy->empty()) {
                    p = newbuy->top();
                }
                while (p.due <= timestramp && p.due != -1 && !newbuy->empty()) {
                    newbuy->pop();
                    if (!newbuy->empty()) {
                        p = newbuy->top();
                    }
                }
                if (p.due > timestramp || p.due == -1) {
                    maxbuy = p.price;
                }
            }
            if (tosell->find(*iter) != tosell->end()) {
                priority_queue<Order, vector<Order>, Comp_to_sell>* newsell = (*tosell)[*iter];
                p.due = -2;
                if (!newsell->empty()) {
                    p = newsell->top();
                }
                while (p.due <= timestramp && p.due != -1 && !newsell->empty()) {
                    newsell->pop();
                    if (!newsell->empty()) {
                        p = newsell->top();
                    }
                }
                if (p.due > timestramp || p.due == -1) {
                    minsell = p.price;
                }
            }
            if (minsell != -1 && maxbuy != -1) {
                cout << "Midpoint of " << *iter << " at time " << curr << " is $" << (maxbuy + minsell) / 2 << endl;
            }
            else {
                cout << "Midpoint of " << *iter << " at time " << curr << " is undefined" << endl;
            }
        }
    }

    cout << "---End of Day---" << endl;
    cout << "Commission Earnings: $" << earn << endl;
    cout << "Total Amount of Money Transferred: $" << money_tran << endl;
    cout << "Number of Completed Trades: " << trades << endl;
    cout << "Number of Shares Traded: " << shares << endl;

    if (f_transfer) {
        for (map<string, Merchant>::iterator iter = transfer->begin(); iter != transfer->end(); iter++) {
            cout << iter->second.name << " bought " << iter->second.bought << " and sold " << iter->second.sold << " for a net transfer of $" << iter->second.value << endl;
        }
    }

    for (vector<Traveler>::iterator iter = TTT->begin(); iter != TTT->end(); iter++) {
        cout << "Time travelers would buy " << iter->symbol << " at time: " << iter->time_buy << " and sell it at time: " << iter->time_sell << endl;
    }

    fin.close();
}
