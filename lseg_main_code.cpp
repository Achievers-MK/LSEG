#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bits/stdc++.h>
#include <ctime>
#include <chrono>
#include <iomanip>
using namespace std;


//order class initalizing
class orders{
    public:
        string order_ID;
        string client_ID;
        string instrument;
        int side;
        int execution_status;
        int quantity;
        double price;
        string reason = "-";
        orders(string order_ID,string client_ID,string instrument, int side, int quantity, double price, string reason) : order_ID(order_ID),client_ID(client_ID),instrument(instrument),side(side),quantity(quantity),price(price),reason(reason){}
};

//buy and sell books initializing 
vector<orders> Buy_Rose,Buy_Lavender,Buy_Lotus,Buy_Tulip,Buy_Orchid;
vector<orders> Sell_Rose,Sell_Lavender,Sell_Lotus,Sell_Tulip,Sell_Orchid;

//getting the execution status as a string
string exe_status(orders order){
    if(order.execution_status == 0) return "New";
    else if(order.execution_status == 1) return "Rejected";
    else if(order.execution_status == 2) return "Fill";
    else return "Pfill";
}

//function to calculate the current time
string get_time(){
    auto now = chrono::system_clock::now();
    auto ms = chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    time_t t = chrono::system_clock::to_time_t(now);
    tm tm = *std::localtime(&t);

    ostringstream oss;
    oss << setfill('0') << setw(4) << tm.tm_year + 1900 << setfill('0') << setw(2) << tm.tm_mon + 1 << setfill('0') << setw(2) << tm.tm_mday << "-"
        << setfill('0') << setw(2) << tm.tm_hour << setfill('0') << setw(2) << tm.tm_min << setfill('0') << setw(2) << tm.tm_sec << "." << setfill('0') << setw(3) << (ms % 1000);
    
    string current_time = oss.str();
    return current_time;
}

//sorting the sell book according to the prices(descending order)
bool compareSellBook(const orders &transaction1, const orders &transaction2) {
    return transaction1.price < transaction2.price;
}

//sorting the sell book according to the prices(ascending order)
bool compareBuyBook(const orders &transaction1, const orders &transaction2) {
    return transaction1.price > transaction2.price;
}

//writing an order to the execution.csv file
bool writeOrder(ofstream &file, orders &order){
    string current_time = get_time();
    file << order.order_ID << "," << order.client_ID << "," << order.instrument << "," << order.side << "," << exe_status(order) << "," << order.quantity << "," << order.price << "," << order.reason << "," << current_time << endl;
    return true;
}

//detecting errors of the orders in orders.csv file
bool error_detect(orders &order)
{
    if (order.client_ID.length()<1){
        order.reason = "Invalid ID";
        return true;
    }

    else if (order.instrument.length()<1 || !(order.instrument == "Rose" || order.instrument == "Lavender" || order.instrument == "Lotus" || order.instrument == "Tulip" || order.instrument == "Orchid")){
        order.reason = "Invalid Instrument";
        return true;
    }

    else if (!(order.side == 1 || order.side == 2)){
        order.reason = "Invalid Side";
        return true;
    }

    else if (!(order.quantity%10==0) || !((order.quantity)>=10 && (order.quantity<=1000))){
        order.reason = "Invalid Quantity";
        return true;
    }

    else if (order.price<0){
        order.reason = "Invalid Price";
        return true;
    }
    return false;
}

//process buy and sell orders
void processOrders(vector<orders> &Buy, vector<orders> &Sell, orders &order, ofstream &file) {
    int neworder = 1;
    int checkNum = 0;
    
    //if order is a buy order
    if(order.side == 1){
        while ((Sell.size() > 0) && (Sell[0].price <= order.price) && checkNum == 0) {
            neworder = 0;
            int temp_price = Sell[0].price;
            if (Sell[0].price < order.price){
                Sell[0].price = order.price;
            }
            if (Sell[0].quantity == order.quantity){
                Sell[0].execution_status = 2;
                order.execution_status = 2;
                Sell.erase(Sell.begin());
                checkNum = 1;
            } else if (Sell[0].quantity > order.quantity) {
                Sell[0].execution_status = 3;
                order.execution_status = 2;
                Sell[0].quantity = order.quantity;
                checkNum = 1;
            } else if (Sell[0].quantity < order.quantity) {
                Sell[0].execution_status = 2;
                order.execution_status = 3;
                int temp_quantity = order.quantity;
                order.quantity = Sell[0].quantity;
                writeOrder(file, order);
                writeOrder(file, Sell[0]);
                order.quantity = temp_quantity-Sell[0].quantity;
                Sell.erase(Sell.begin());
            }


            //writing to the file
            if (checkNum == 1 && Sell[0].execution_status==3){
                writeOrder(file, order);
                writeOrder(file, Sell[0]);
                Sell[0].quantity = Sell[0].quantity - order.quantity;
            }
            else if (checkNum == 1 && Sell[0].execution_status==2){
                writeOrder(file, order);
                writeOrder(file, Sell[0]);
                // order.quantity = order.quantity - Sell[0].quantity;
            }
            Sell[0].price = temp_price;
    }
        //checking the order is new
        if (neworder == 1) {
            order.execution_status = 0;
            writeOrder(file, order);
            Buy.push_back(order);
            sort(Buy.begin(), Buy.end(), compareBuyBook);
        }

        //checking the order is parctially filled
        if (order.execution_status == 3){
            Buy.push_back(order);
            sort(Buy.begin(), Buy.end(), compareBuyBook);
        }
    }

    //if order is a sell order
    else if(order.side==2){
        while ((Buy.size() > 0) && (Buy[0].price >= order.price) && checkNum == 0) {
            neworder = 0;
            int temp_price = order.price;
            if (Buy[0].price > order.price){
                order.price = Buy[0].price;
            }
            if (Buy[0].quantity == order.quantity) {
                Buy[0].execution_status = 2;
                order.execution_status = 2;
                Buy.erase(Buy.begin());
                checkNum = 1;
            } else if (Buy[0].quantity > order.quantity) {
                Buy[0].execution_status = 3;
                order.execution_status = 2;
                Buy[0].quantity = order.quantity;
                checkNum = 1;
            } else if (Buy[0].quantity < order.quantity) {
                Buy[0].execution_status = 2;
                order.execution_status = 3;
                int temp_quantity = order.quantity;
                order.quantity = Buy[0].quantity;
                writeOrder(file, order);
                writeOrder(file, Buy[0]);
                order.quantity = temp_quantity-Buy[0].quantity;
                Buy.erase(Buy.begin());
            }

            //writing to the file
            if (checkNum == 1 && Buy[0].execution_status==3){
                writeOrder(file, order);
                writeOrder(file, Buy[0]);
                Buy[0].quantity = Buy[0].quantity - order.quantity;
            }
            else if (checkNum == 1 && Buy[0].execution_status==2){
                    writeOrder(file, order);
                    writeOrder(file, Buy[0]);
                    // Buy[0].quantity = Buy[0].quantity - order.quantity;
            }
            order.price = temp_price;
        }
        
        //checking the order is new
        if (neworder == 1) {
            order.execution_status = 0;
            writeOrder(file, order);
            Sell.push_back(order);
            sort(Sell.begin(), Sell.end(), compareSellBook);
        }

        //checking the order is parctially filled
        if (order.execution_status == 3) {
            Sell.push_back(order);
            sort(Sell.begin(), Sell.end(), compareSellBook);
        }
    }
}

int main(){
    auto start = chrono :: steady_clock :: now();

    //creating the execution file and headings
    ofstream out_file("execution.csv");
    if (!out_file.is_open()) {
        cerr << "Error creating file." << endl;
        return 1;
    }
    out_file << "Order ID" << "," << "Cliend ID" << "," << "Instrument" << "," << "Side" << "," << "Status" << "," << "Quantity" << "," << "Price" << "," << "Reason" << "," << "Ececution Time" << std::endl;

    
    //opening the orders csv file
    ifstream ip("orders.csv");
    if(!ip.is_open()) {
        std::cout << "Error: File open" << "\n";
        return 1;
    }

    string client_ID;
    string instrument;
    string side;
    string quantity;
    string price;
    string status;
    string reason = "-";
    int i=-1;

    //reading line by line in the orders.csv file
    while(ip.good()){
        i++;
        getline(ip,client_ID,',');
        getline(ip,instrument,',');
        getline(ip,side,',');
        getline(ip,quantity,',');
        getline(ip,price,'\n');
        string order_ID = "ord" + to_string(i);
        
        //skipping the first line of the orders.csv file
        if (i==0){
            continue;
        }

        //creating the order object under the orders class
        orders order(order_ID, client_ID, instrument, stoi(side), stoi(quantity), stod(price), reason);

        //checking for errors in the order
        if (error_detect(order)) {
            order.execution_status = 1;
            string current_time = get_time();
            writeOrder(out_file, order);
        }

        //if no errors process the orders according to the instrument type
        else{
            if (order.instrument == "Rose"){
                processOrders(Buy_Rose, Sell_Rose, order,out_file);
            }
            else if(order.instrument  == "Lavender"){
                processOrders(Buy_Lavender, Sell_Lavender, order,out_file);
            }
            else if(order.instrument  == "Lotus"){
                processOrders(Buy_Lotus, Sell_Lotus, order,out_file);
            }
            else if(order.instrument == "Tulip"){
                processOrders(Buy_Tulip, Sell_Tulip, order,out_file);
            }
            else {
                processOrders(Buy_Orchid, Sell_Orchid, order, out_file);
            }
        }
    }
    ip.close();

    //output the time taken to generate the execution.scv file to check the speed
    auto end = chrono :: steady_clock :: now();
    auto diff = end - start;
    cout << chrono :: duration<double,milli>(diff).count() << " ms" << endl;
    return 0;
}
