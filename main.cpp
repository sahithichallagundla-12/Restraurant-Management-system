#include <bits/stdc++.h>
using namespace std;

struct MenuItem {
    int id;
    string name;
    string category;
    double price;
    bool available;
    int qty; // quantity available
    int prepTime; // preparation time in minutes
};

struct CartItem {
    int menuId;
    int qty;
};

enum class OrderStatus { PENDING=0, ACCEPTED=1, PREPARING=2, READY=3, COMPLETED=4, CANCELLED=5 };

string statusToString(OrderStatus s){
    switch(s){
        case OrderStatus::PENDING: return "Pending";
        case OrderStatus::ACCEPTED: return "Accepted";
        case OrderStatus::PREPARING: return "Preparing";
        case OrderStatus::READY: return "Ready";
        case OrderStatus::COMPLETED: return "Completed";
        case OrderStatus::CANCELLED: return "Cancelled";
    }
    return "Unknown";
}

struct Order {
    long long orderId;
    string userName;
    string mobile;
    vector<CartItem> items;
    double totalAmount;
    OrderStatus status;
    int tokenNumber;
    time_t placedAt;
    time_t estimatedReadyAt; // timestamp
};

// GLOBAL STORAGE
vector<MenuItem> MENU;
vector<Order> ORDERS;
// otpStore stores OTP and creation time: mobile -> (otp, createdAt)
map<string, pair<int,time_t>> otpStore;
long long NEXT_ORDER_ID = 1;
int NEXT_TOKEN = 100;

string ORDERS_FILE = "orders.txt";
string ORDERID_FILE = "orderid.txt";
string MENU_FILE = "menu.txt";
string TOKEN_FILE = "token.txt";

// HELPERS
int inputInt(const string &msg){
    int x;
    while(true){
        cout << msg;
        if(cin >> x){
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return x;
        }
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again.\n";
    }
}

void pressEnter(){ cout << "\nPress Enter..."; cin.ignore(numeric_limits<streamsize>::max(), '\n'); }

double calculateTotal(const vector<CartItem> &cart){
    double t = 0;
    for(auto &c : cart){
        for(auto &m : MENU){
            if(m.id == c.menuId) t += m.price * c.qty;
        }
    }
    return t;
}

MenuItem* getMenu(int id){
    for(auto &m : MENU) if(m.id == id) return &m;
    return nullptr;
}

int countActiveOrdersBefore(long long orderId){
    int cnt = 0;
    for(auto &o : ORDERS){
        if(o.orderId == orderId) break;
        if(o.status != OrderStatus::COMPLETED && o.status != OrderStatus::CANCELLED) cnt++;
    }
    return cnt;
}

// Simple estimate calculation
// base = max prep time among items (minutes)
// queueDelay = number of active orders ahead * 2 minutes
// estimatedReadyAt = reference + (base + queueDelay)*60

time_t computeEstimateForNewOrder(const Order &o, bool useNow = false){
    int maxPrep = 0;
    for(auto &ci : o.items){
        MenuItem *m = getMenu(ci.menuId);
        if(m) maxPrep = max(maxPrep, m->prepTime);
    }
    int pendingAhead = 0;
    if(useNow){
        for(auto &x : ORDERS){
            if(x.status != OrderStatus::COMPLETED && x.status != OrderStatus::CANCELLED) pendingAhead++;
        }
    } else {
        pendingAhead = countActiveOrdersBefore(o.orderId);
    }
    int queueDelay = pendingAhead * 2; // 2 min per order
    time_t ref = useNow ? time(nullptr) : o.placedAt;
    return ref + (time_t)((maxPrep + queueDelay) * 60);
}

// FILE HANDLING
void saveOrderId(){
    ofstream out(ORDERID_FILE);
    if(out) out << NEXT_ORDER_ID;
}

void loadOrderId(){
    ifstream in(ORDERID_FILE);
    if(in) in >> NEXT_ORDER_ID;
}

void saveToken(){
    ofstream out(TOKEN_FILE);
    if(out) out << NEXT_TOKEN;
}

void loadToken(){
    ifstream in(TOKEN_FILE);
    if(in) in >> NEXT_TOKEN;
}

// orders file format per line:
// orderId|userName|mobile|status|tokenNumber|placedAt|totalAmount|estimatedReadyAt|item1Id:qty,item2Id:qty,...
void saveOrders(){
    ofstream out(ORDERS_FILE);
    if(!out.is_open()) return;
    for(auto &o : ORDERS){
        out << o.orderId << "|"
            << o.userName << "|"
            << o.mobile << "|"
            << o.tokenNumber << "|"
            << o.totalAmount << "|";
        for(size_t i=0;i<o.items.size();++i){
            out << o.items[i].menuId << ":" << o.items[i].qty;
            if(i+1 < o.items.size()) out << ",";
        }
        out << "\n";
    }
}

void loadOrders(){
    ORDERS.clear();
    ifstream in(ORDERS_FILE);
    if(!in.is_open()) return;
    string line;
    while(getline(in, line)){
        if(line.empty()) continue;
        stringstream ss(line);
        vector<string> p;
        string part;
        while(getline(ss, part, '|')) p.push_back(part);
        if(p.size() < 9) continue; // expect at least 9 parts
        try{
            Order o;
            o.orderId = stoll(p[0]);
            o.userName = p[1];
            o.mobile = p[2];
            o.status = (OrderStatus)stoi(p[3]);
            o.tokenNumber = stoi(p[4]);
            o.placedAt = stoll(p[5]);
            o.totalAmount = stod(p[6]);
            o.estimatedReadyAt = (time_t)stoll(p[7]);
            string items = p[8];
            stringstream is(items);
            string token;
            while(getline(is, token, ',')){
                size_t pos = token.find(':');
                if(pos == string::npos) continue;
                CartItem c;
                c.menuId = stoi(token.substr(0, pos));
                c.qty = stoi(token.substr(pos+1));
                o.items.push_back(c);
            }
            ORDERS.push_back(o);
        } catch(...) {
            // skip invalid line
            continue;
        }
    }
}

// MENU file format per line:
// id|name|category|price|qty|prepTime
void loadMenu(){
    MENU.clear();
    ifstream in(MENU_FILE);
    if(!in.is_open()) return;
    string line;
    while(getline(in, line)){
        if(line.empty()) continue;
        stringstream ss(line);
        string sid,name,cat,sprice,sqty,sprep;
        if(!getline(ss,sid,'|')) continue;
        if(!getline(ss,name,'|')) continue;
        if(!getline(ss,cat,'|')) continue;
        if(!getline(ss,sprice,'|')) continue;
        if(!getline(ss,sqty,'|')) continue;
        if(!getline(ss,sprep)) continue;
        try{
            MenuItem m;
            m.id = stoi(sid);
            m.name = name;
            m.category = cat;
            m.price = stod(sprice);
            m.qty = stoi(sqty);
            m.available = (m.qty > 0);
            m.prepTime = stoi(sprep);
            MENU.push_back(m);
        } catch(...) { continue; }
    }
}

void saveMenu(){
    ofstream out(MENU_FILE);
    if(!out.is_open()) return;
    for(auto &m : MENU){
        out << m.id << "|"
            << m.name << "|"
            << m.category << "|"
            << m.price << "|"
            << m.qty << "|"
            << m.prepTime << "\n";
    }
}

// ORDER CREATION
Order* findOrder(long long id){
    for(auto &o : ORDERS) if(o.orderId == id) return &o;
    return nullptr;
}

void restockOrderItems(const Order &o){
    for(auto &ci : o.items){
        MenuItem *m = getMenu(ci.menuId);
        if(m){
            m->qty += ci.qty;
            if(m->qty > 0) m->available = true;
        }
    }
    saveMenu();
}

Order createOrder(const string &name,const string &mob,const vector<CartItem> &cart){
    // validate stock
    for(auto &c : cart){
        MenuItem *m = getMenu(c.menuId);
        if(!m || !m->available || m->qty < c.qty){
            throw runtime_error("Item not available in requested quantity");
        }
    }
    // deduct stock
    for(auto &c : cart){
        MenuItem *m = getMenu(c.menuId);
        if(m){
            m->qty -= c.qty;
            if(m->qty <= 0) m->available = false;
        }
    }
    saveMenu();

    Order o;
    o.orderId = NEXT_ORDER_ID++;
    o.userName = name;
    o.mobile = mob;
    o.items = cart;
    o.totalAmount = calculateTotal(cart);
    o.status = OrderStatus::PENDING;
    o.tokenNumber = 0;
    o.placedAt = time(nullptr);
    o.estimatedReadyAt = computeEstimateForNewOrder(o, true);

    ORDERS.push_back(o);
    saveOrders();
    saveOrderId();
    return o;
}

// ADMIN PANEL
string ADMIN_USER = "admin";
string ADMIN_PASS = "password";

void adminAddItem(){
    cout << "\n--- Add Menu Item ---\n";
    string name,cat;
    double price;
    int qty,prep;
    cout << "Name: "; getline(cin,name);
    cout << "Category: "; getline(cin,cat);
    cout << "Price: "; if(!(cin >> price)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout << "Invalid price\n"; return; }
    cout << "Quantity: "; if(!(cin >> qty)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout << "Invalid qty\n"; return; }
    cout << "Prep time (minutes): "; if(!(cin >> prep)){ cin.clear(); cin.ignore(numeric_limits<streamsize>::max(),'\n'); cout << "Invalid prep time\n"; return; }
    cin.ignore();
    int newId = 1; for(auto &m : MENU) newId = max(newId, m.id + 1);
    MenuItem it;
    it.id = newId; it.name = name; it.category = cat; it.price = price; it.qty = max(0,qty); it.available = (it.qty>0); it.prepTime = max(0,prep);
    MENU.push_back(it);
    saveMenu();
    cout << "Item added with ID: " << it.id << "\n";
}

void adminDeleteItem(){
    cout << "\n--- Delete Menu Item ---\n";
    int id = inputInt("Enter Item ID to delete (0 cancel): ");
    if(id == 0) return;
    size_t before = MENU.size();
    MENU.erase(remove_if(MENU.begin(), MENU.end(), [&](const MenuItem &m){ return m.id == id; }), MENU.end());
    if(MENU.size() == before){ cout << "Item not found.\n"; return; }
    saveMenu();
    cout << "Item deleted.\n";
}

void adminViewOrders(){
    cout << "\n--- All Orders ---\n";
    for(auto &o : ORDERS){
        cout << "OrderID: " << o.orderId << " | User: " << o.userName << " | Mobile: " << o.mobile << " | Status: " << statusToString(o.status)
             << " | Token: " << (o.tokenNumber==0?"-":to_string(o.tokenNumber)) << " | Total: " << o.totalAmount;
        if(o.estimatedReadyAt > 0) cout << " | Est: " << ctime(&o.estimatedReadyAt);
        else cout << "\n";
    }
    long long id;
    cout << "\nEnter OrderID to manage (0 back): "; cin >> id; cin.ignore();
    if(id == 0) return;
    Order *o = findOrder(id);
    if(!o){ cout << "Not found.\n"; return; }
    cout << "\nManaging Order " << o->orderId << "\n";
    cout << "Current Status: " << statusToString(o->status) << "\n";

    // Options depending on status
    if(o->status == OrderStatus::PENDING){
        cout << "1. Accept Order\n2. Cancel Order\n3. Back\n";
        int x = inputInt("Choose: ");
        if(x==1){
            o->tokenNumber = ++NEXT_TOKEN;
            saveToken();
            o->status = OrderStatus::ACCEPTED;
            o->estimatedReadyAt = computeEstimateForNewOrder(*o, true);
            saveOrders();
            cout << "Order Accepted. Token: " << o->tokenNumber << "\n";
        } else if(x==2){
            o->status = OrderStatus::CANCELLED;
            restockOrderItems(*o);
            saveOrders();
            cout << "Order Cancelled and items restocked.\n";
        }
    } else if(o->status == OrderStatus::ACCEPTED){
        cout << "1. Mark Preparing\n2. Cancel Order\n3. Back\n";
        int x = inputInt("Choose: ");
        if(x==1){
            o->status = OrderStatus::PREPARING;
            o->estimatedReadyAt = computeEstimateForNewOrder(*o, true);
            saveOrders();
            cout << "Marked Preparing.\n";
        } else if(x==2){
            o->status = OrderStatus::CANCELLED;
            restockOrderItems(*o);
            saveOrders();
            cout << "Order Cancelled and items restocked.\n";
        }
    } else if(o->status == OrderStatus::PREPARING){
        cout << "1. Mark Ready\n2. Back\n";
        int x = inputInt("Choose: ");
        if(x==1){
            o->status = OrderStatus::READY;
            o->estimatedReadyAt = time(nullptr);
            saveOrders();
            cout << "Marked Ready.\n";
        }
    } else if(o->status == OrderStatus::READY){
        cout << "1. Mark Completed\n2. Back\n";
        int x = inputInt("Choose: ");
        if(x==1){
            o->status = OrderStatus::COMPLETED;
            o->estimatedReadyAt = time(nullptr);
            saveOrders();
            cout << "Completed.\n";
        }
    } else if(o->status == OrderStatus::CANCELLED){
        cout << "Order already cancelled.\n";
    } else {
        cout << "Already Completed.\n";
    }
}

void adminPanel(){
    while(true){
        cout << "\n--- Admin Panel ---\n";
        cout << "1. View Orders\n2. Add Item\n3. Delete Item\n4. View Menu\n5. Logout\n";
        int ch = inputInt("Choose: ");
        if(ch==1) adminViewOrders();
        else if(ch==2) adminAddItem();
        else if(ch==3) adminDeleteItem();
        else if(ch==4){
            cout << "\n--- Current Menu ---\n";
            for(auto &m : MENU){
                cout << m.id << ". " << m.name << " (" << m.category << ") - Rs." << m.price << " | Qty: " << m.qty << " | " << (m.available?"Available":"Not Available") << " | Prep: " << m.prepTime << " min\n";
            }
            pressEnter();
        } else break;
    }
}

// USER FLOW
void showMenu(){
    cout << "\n--- Menu Categories ---\n";
    set<string> cats;
    for(auto &m : MENU) cats.insert(m.category);
    int idx=1;
    vector<string> catList;
    for(auto &c : cats){ cout << idx << ". " << c << "\n"; catList.push_back(c); idx++; }
    int csel = inputInt("Choose category (0 Cancel): ");
    if(csel<=0 || csel>(int)catList.size()) return;
    string cat = catList[csel-1];
    cout << "\n--- Items (" << cat << ") ---\n";
    for(auto &m : MENU){ if(m.category == cat){
        cout << m.id << ". " << m.name << " - Rs." << m.price << " | Qty: " << m.qty << (m.available?"":" (Not Available)") << " | Prep: " << m.prepTime << " min\n";
    }}
}

void userPlaceOrder(const string &name,const string &mobile){
    vector<CartItem> cart;
    while(true){
        cout << "\n--- User Menu ---\n";
        cout << "1. Show Menu\n2. View Cart\n3. Place Order\n4. Back\n";
        int ch = inputInt("Choose: ");
        if(ch==1){
            showMenu();
            int iid = inputInt("Enter Item ID (0 cancel): ");
            if(iid == 0) continue;
            MenuItem *m = getMenu(iid);
            if(!m){ cout << "Invalid.\n"; continue; }
            if(!m->available){ cout << "Not available.\n"; continue; }
            int q = inputInt("Quantity: ");
            if(q<=0){ cout << "Invalid qty.\n"; continue; }
            if(q > m->qty){ cout << "Only " << m->qty << " available.\n"; continue; }
            bool found=false;
            for(auto &c : cart) if(c.menuId == iid){
                if(c.qty + q > m->qty){ cout << "Not enough stock for additional quantity.\n"; found=true; break; }
                c.qty += q; found=true; break;
            }
            if(!found) cart.push_back({iid,q});
            cout << "Added.\n";
        } else if(ch==2){
            cout << "\n--- Cart ---\n";
            double total = 0;
            for(auto &c : cart){
                MenuItem *m = getMenu(c.menuId);
                if(m){ cout << m->name << " x" << c.qty << " = Rs." << (m->price*c.qty) << "\n"; total += m->price * c.qty; }
            }
            cout << "Total = Rs." << total << "\n";
            pressEnter();
        } else if(ch==3){
            if(cart.empty()){ cout << "Cart empty.\n"; continue; }
            try{
                Order o = createOrder(name, mobile, cart);
                cout << "\nOrder Placed! OrderID = " << o.orderId << "\n";
                if(o.estimatedReadyAt > 0) cout << "Estimated ready at: " << ctime(&o.estimatedReadyAt);
                // Tracking view (simple loop)
                while(true){
                    Order *p = findOrder(o.orderId);
                    cout << "\nStatus: " << statusToString(p->status) << " | Token: " << (p->tokenNumber==0?"-":to_string(p->tokenNumber)) << "\n";
                    if(p->estimatedReadyAt > 0) cout << "Estimated ready at: " << ctime(&p->estimatedReadyAt);
                    if(p->status == OrderStatus::COMPLETED || p->status == OrderStatus::CANCELLED){ cout << "Order Finished.\n"; break; }
                    cout << "1. Refresh\n2. Exit Tracking\n";
                    int t = inputInt("Choose: ");
                    if(t==1) continue; else break;
                }
            } catch(const exception &ex){ cout << "Failed to place order: " << ex.what() << "\n"; }
            break;
        } else break;
    }
}

void userHistory(const string &mobile){
    cout << "\n--- Completed Orders ---\n";
    vector<Order*> hist;
    for(auto &o : ORDERS) if(o.mobile == mobile && o.status == OrderStatus::COMPLETED) hist.push_back(&o);
    if(hist.empty()){ cout << "No completed orders.\n"; return; }
    for(auto *o : hist) cout << "OrderID: " << o->orderId << " Total: Rs." << o->totalAmount << " Date: " << ctime(&o->placedAt);
    long long id;
    cout << "\nReorder? Enter OrderID (0 cancel): "; cin >> id; cin.ignore();
    if(id == 0) return;
    Order *old = findOrder(id);
    if(!old || old->mobile != mobile){ cout << "Invalid.\n"; return; }
    try{ Order newO = createOrder(old->userName, old->mobile, old->items); cout << "Reorder placed! New OrderID = " << newO.orderId << "\n"; if(newO.estimatedReadyAt>0) cout << "Estimated ready at: " << ctime(&newO.estimatedReadyAt); } catch(const exception &ex){ cout << "Reorder failed: " << ex.what() << "\n"; }
}

// MAIN
int main(){
    srand((unsigned)time(nullptr));

    loadMenu();
    if(MENU.empty()){
        MENU = {
            {1,"Veg Thali","Veg",90.0,true, 10, 15},
            {2,"Paneer Masala","Veg",120.0,true, 8, 20},
            {3,"Chicken Biryani","Non-Veg",150.0,true, 12, 25},
            {4,"Mutton Curry","Non-Veg",200.0,true, 5, 30},
            {5,"Gulab Jamun","Desserts",40.0,true, 15, 5},
            {6,"Ice Cream","Desserts",35.0,true, 20, 2},
            {7,"Masala Dosa","Snacks",50.0,true, 10, 12},
            {8,"Samosa","Snacks",20.0,true, 25, 8},
            {9,"Tea","Drinks",10.0,true, 30, 3},
            {10,"Cold Drink","Drinks",40.0,true, 18, 1},
        };
        saveMenu();
    }

    loadOrders();
    loadOrderId();
    loadToken();

    while(true){
        cout << "\n=== Smart Restaurant Console ===\n";
        cout << "1. User\n2. Admin\n3. Exit\n";
        int ch = inputInt("Choose: ");
        if(ch==1){
            string name,mob;
            cout << "Enter Name: "; getline(cin,name);
            cout << "Enter Mobile: "; getline(cin,mob);
            // generate OTP and store with timestamp
            int otp = 1000 + rand()%9000;
            otpStore[mob] = make_pair(otp, time(nullptr));
            cout << "[Debug OTP] = " << otp << "\n";
            int e;
            cout << "Enter OTP: "; cin >> e; cin.ignore();
            auto it = otpStore.find(mob);
            if(it == otpStore.end()){ cout << "OTP not found.\n"; continue; }
            // expire OTP after 120 seconds
            if(time(nullptr) - it->second.second > 120){ cout << "OTP expired. Request new.\n"; otpStore.erase(mob); continue; }
            if(e != it->second.first){ cout << "OTP Wrong.\n"; continue; }
            otpStore.erase(mob);
            cout << "Login success.\n";
            while(true){
                cout << "\n--- User Home ---\n";
                cout << "1. Place Order\n2. Track Order\n3. Order History\n4. Logout\n";
                int u = inputInt("Choose: ");
                if(u==1) userPlaceOrder(name, mob);
                else if(u==2){ long long id; cout << "Enter OrderID: "; cin >> id; cin.ignore(); Order *o = findOrder(id); if(o && o->mobile == mob){ cout << "Status: " << statusToString(o->status) << " | Token: " << (o->tokenNumber==0?"-":to_string(o->tokenNumber)) << "\n"; if(o->estimatedReadyAt>0) cout << "Estimated ready at: " << ctime(&o->estimatedReadyAt); } else cout << "Not found.\n"; }
                else if(u==3) userHistory(mob);
                else break;
            }
        } else if(ch==2){
            string id,pass;
            cout << "Admin ID: "; getline(cin,id);
            cout << "Password: "; getline(cin,pass);
            if(id==ADMIN_USER && pass==ADMIN_PASS) adminPanel(); else cout << "Wrong credentials.\n";
        } else break;
    }

    return 0;
}
