#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>
#include <cstdlib>
using namespace std;


template <class T>
class LinkedList {
public:
    struct Node { T data; Node* next; };
    Node* head = nullptr;

    void push_front(const T& v) { head = new Node{v, head}; }         
    void push_back(const T& v) {                                       
        Node* n = new Node{v, nullptr};
        if (!head) { head = n; return; }
        Node* cur = head;
        while (cur->next) cur = cur->next;
        cur->next = n;
    }
    ~LinkedList() {
        while (head) { Node* t = head; head = head->next; delete t; }
    }
};


template <class K, class V>
class HashTable {
    struct Node { K key; V value; Node* next; };
    static const int SIZE = 101;
    Node* table[SIZE] = {nullptr};

    int index(const K& key) const { return hash<K>()(key) % SIZE; }  
public:
    void put(const K& key, const V& value) {                          
        int i = index(key);
        for (Node* c = table[i]; c; c = c->next)
            if (c->key == key) { c->value = value; return; }
        table[i] = new Node{key, value, table[i]};
    }
    V* get(const K& key) {                                          
        for (Node* c = table[index(key)]; c; c = c->next)
            if (c->key == key) return &c->value;
        return nullptr;
    }
    ~HashTable() {
        for (int i = 0; i < SIZE; i++)
            while (table[i]) { Node* t = table[i]; table[i] = t->next; delete t; }
    }
};


class MinHeap {
    vector<pair<double, int>> a;              
public:
    bool empty() const { return a.empty(); }
    void push(pair<double, int> x) {           
        a.push_back(x);
        int i = a.size() - 1;
        while (i > 0 && a[i] < a[(i - 1) / 2]) {
            swap(a[i], a[(i - 1) / 2]);
            i = (i - 1) / 2;
        }
    }
    pair<double, int> pop() {                  
        pair<double, int> top = a[0];
        a[0] = a.back();
        a.pop_back();
        int i = 0, n = a.size();
        while (true) {
            int l = 2 * i + 1, r = 2 * i + 2, small = i;
            if (l < n && a[l] < a[small]) small = l;
            if (r < n && a[r] < a[small]) small = r;
            if (small == i) break;
            swap(a[i], a[small]);
            i = small;
        }
        return top;
    }
};


struct Edge { int to; double km; string line; };

struct Route {
    bool found = false;
    vector<int> path;         
    vector<string> lineOf;     
    double km = 0;
    int interchanges = 0;
};

static string lower(string s) { for (auto& c : s) c = tolower((unsigned char)c); return s; }

class Graph {
public:
    vector<string> names;                  
    vector<vector<Edge>> adj;              
    HashTable<string, int> idOf;          

    int addStation(const string& name) {
        if (int* p = idOf.get(lower(name))) return *p;
        names.push_back(name);
        adj.push_back({});
        idOf.put(lower(name), names.size() - 1);
        return names.size() - 1;
    }
    void addEdge(int a, int b, double km, const string& line) {      
        adj[a].push_back({b, km, line});
        adj[b].push_back({a, km, line});
    }

    Route dijkstra(int src, int dst, int mode) {
        int n = names.size();
        vector<double> dist(n, 1e18);
        vector<int> prev(n, -1);
        vector<string> prevLine(n);
        vector<double> prevKm(n, 0);
        MinHeap heap;

        dist[src] = 0;
        heap.push({0, src});
        while (!heap.empty()) {
            auto [d, u] = heap.pop();
            if (d > dist[u]) continue;            
            if (u == dst) break;
            for (auto& e : adj[u]) {
                double w = (mode == 1) ? e.km : 1;
                if (dist[u] + w < dist[e.to]) {
                    dist[e.to] = dist[u] + w;
                    prev[e.to] = u;
                    prevLine[e.to] = e.line;
                    prevKm[e.to] = e.km;
                    heap.push({dist[e.to], e.to});
                }
            }
        }

        Route r;
        if (dist[dst] >= 1e18) return r;       
        r.found = true;
        vector<string> lines;
        for (int v = dst; v != -1; v = prev[v]) { 
            r.path.push_back(v);
            lines.push_back(prevLine[v]);
            r.km += prevKm[v];
        }
        reverse(r.path.begin(), r.path.end());
        reverse(lines.begin(), lines.end());
        r.lineOf = lines;
        for (size_t i = 2; i < r.path.size(); i++)
            if (r.lineOf[i] != r.lineOf[i - 1]) r.interchanges++;
        return r;
    }
};

struct Ticket { int id; string cardId; int from, to; double km; int fare; string status; };

struct Commuter {
    string cardId, name;
    double balance;
    LinkedList<int> history;                       
};

int fareFor(double km) {                           
    if (km <= 2) return 11;
    if (km <= 5) return 21;
    if (km <= 12) return 32;
    if (km <= 21) return 43;
    if (km <= 32) return 54;
    return 64;
}

class DelhiMetroX {
    Graph graph;
    HashTable<string, Commuter*> commuters;        
    HashTable<int, Ticket> tickets;            
    int nextTicket = 1001;


    string input(const string& prompt) {
        cout << prompt;
        string s;
        if (!getline(cin, s)) exit(0);
        return s;
    }
    double inputNum(const string& prompt) {
        while (true) {
            string s = input(prompt);
            try { size_t i; double v = stod(s, &i); if (i == s.size()) return v; } catch (...) {}
            cout << "Invalid number. Try again.\n";
        }
    }
    void heading(const string& t) { cout << "\n==== " << t << " ====\n"; }


    void buildNetwork() {
        struct Line { string name; vector<pair<string, double>> stops; };
        vector<Line> lines = {
            {"Yellow", {{"Vishwavidyalaya",1.4},{"Kashmere Gate",1.2},{"Chandni Chowk",1.3},{"New Delhi",1.6},{"Rajiv Chowk",1.3},
                        {"Central Secretariat",3.5},{"INA",3.0},{"Hauz Khas",3.1},{"Qutub Minar",2.5},{"Saket",13.0},{"Huda City Centre",0}}},
            {"Blue",   {{"Dwarka Sector 21",1.6},{"Dwarka Mor",3.0},{"Janakpuri West",5.8},{"Rajouri Garden",3.4},{"Kirti Nagar",9.6},
                        {"Rajiv Chowk",1.3},{"Mandi House",8.6},{"Yamuna Bank",2.5},{"Akshardham",14.5},{"Noida City Centre",0}}},
            {"Red",    {{"Rithala",3.9},{"Netaji Subhash Place",11.0},{"Kashmere Gate",2.8},{"Shastri Park",2.4},{"Dilshad Garden",0}}},
            {"Violet", {{"Kashmere Gate",8.6},{"Mandi House",2.0},{"Central Secretariat",8.9},{"Lajpat Nagar",3.2},{"Kalkaji Mandir",0}}},
        };
        for (auto& ln : lines) {
            LinkedList<pair<string, double>> list;                 
            for (auto& s : ln.stops) list.push_back(s);
            for (auto* c = list.head; c && c->next; c = c->next) { 
                int a = graph.addStation(c->data.first);
                int b = graph.addStation(c->next->data.first);
                graph.addEdge(a, b, c->data.second, ln.name);
            }
        }
    }

    int readStation(const string& prompt) {
        while (true) {
            string s = input(prompt + " (name or id, ? for list): ");
            if (s == "?") { listStations(); continue; }
            if (!s.empty() && all_of(s.begin(), s.end(), ::isdigit)) {
                int i = stoi(s);
                if (i < (int)graph.names.size()) return i;
            }
            if (int* p = graph.idOf.get(lower(s))) return *p;     
            cout << "Unknown station.\n";
        }
    }
    void listStations() {
        cout << "\n";
        for (size_t i = 0; i < graph.names.size(); i++) cout << "  " << i << ". " << graph.names[i] << "\n";
    }

   
    void printRoute(const Route& r) {
        cout << "  " << r.lineOf[1] << " Line:\n";
        cout << "    * " << graph.names[r.path[0]] << "  (Start)\n";
        for (size_t i = 1; i < r.path.size(); i++) {
            string note;
            if (i + 1 == r.path.size()) note = "  (Destination)";
            else if (r.lineOf[i + 1] != r.lineOf[i]) note = "  (Change to " + r.lineOf[i + 1] + " Line)";
            cout << "    * " << graph.names[r.path[i]] << note << "\n";
            if (note.find("Change") != string::npos) cout << "  " << r.lineOf[i + 1] << " Line:\n";
        }
        cout << "\n  Distance     : " << r.km << " km\n"
             << "  Stops        : " << r.path.size() - 1 << "\n"
             << "  Interchanges : " << r.interchanges << "\n"
             << "  Fare         : Rs " << fareFor(r.km) << "\n";
    }

    void registerCommuter() {
        heading("REGISTER CARD");
        string id = input("Card ID: ");
        if (id.empty() || commuters.get(id)) { cout << "Invalid or duplicate card ID.\n"; return; }
        string name = input("Name: ");
        commuters.put(id, new Commuter{id, name, 100});
        cout << "Card issued with Rs 100 balance.\n";
    }
    Commuter* findCommuter() {
        string id = input("Card ID: ");
        Commuter** p = commuters.get(id);                          
        if (!p) { cout << "Card not found.\n"; return nullptr; }
        return *p;
    }
    void recharge() {
        heading("RECHARGE");
        Commuter* c = findCommuter(); if (!c) return;
        double a = inputNum("Amount: ");
        if (a <= 0) { cout << "Invalid amount.\n"; return; }
        c->balance += a;
        cout << "New balance: Rs " << c->balance << "\n";
    }
    void planRoute() {
        heading("PLAN ROUTE");
        int s = readStation("From"), d = readStation("To");
        if (s == d) { cout << "Same station.\n"; return; }
        const char* title[] = {"", "Option 1: Shortest distance", "Option 2: Fewest stations"};
        for (int m = 1; m <= 2; m++) {
            cout << "\n" << title[m] << "\n";
            Route r = graph.dijkstra(s, d, m);
            if (!r.found) { cout << "  No route.\n"; return; }
            printRoute(r);
        }
    }
    void bookJourney() {
        heading("BOOK JOURNEY");
        Commuter* c = findCommuter(); if (!c) return;
        int s = readStation("From"), d = readStation("To");
        if (s == d) { cout << "Same station.\n"; return; }
        int mode = (int)inputNum("Route type (1 = shortest distance, 2 = fewest stations): ");
        Route r = graph.dijkstra(s, d, mode == 2 ? 2 : 1);
        if (!r.found) { cout << "No route.\n"; return; }
        cout << "\n"; printRoute(r);
        int fare = fareFor(r.km);
        if (c->balance < fare) { cout << "\nInsufficient balance. Please recharge.\n"; return; }
        c->balance -= fare;
        Ticket t{nextTicket++, c->cardId, s, d, r.km, fare, "BOOKED"};
        tickets.put(t.id, t);                                      
        c->history.push_front(t.id);                               
        cout << "\nTicket #" << t.id << " booked. Balance: Rs " << c->balance << "\n";
    }
    void cancelTicket() {
        heading("CANCEL TICKET");
        int id = (int)inputNum("Ticket ID: ");
        Ticket* t = tickets.get(id);                              
        if (!t) { cout << "Ticket not found.\n"; return; }
        if (t->status != "BOOKED") { cout << "Ticket already " << t->status << ".\n"; return; }
        int refund = (int)(t->fare * 0.8);
        t->status = "CANCELLED";
        (*commuters.get(t->cardId))->balance += refund;
        cout << "Ticket cancelled. Refund: Rs " << refund << " (20% deducted)\n";
    }
    void showHistory() {
        heading("JOURNEY HISTORY");
        Commuter* c = findCommuter(); if (!c) return;
        cout << "Name: " << c->name << "   Balance: Rs " << c->balance << "\n\n";
        if (!c->history.head) { cout << "No journeys yet.\n"; return; }
        for (auto* n = c->history.head; n; n = n->next) {          
            Ticket* t = tickets.get(n->data);
            cout << "  #" << t->id << "  " << graph.names[t->from] << " -> " << graph.names[t->to]
                 << "  " << t->km << " km  Rs " << t->fare << "  " << t->status << "\n";
        }
    }

public:
    void run() {
        buildNetwork();
        while (true) {
            cout << "\n========================================\n"
                 << "              DELHI METRO X\n"
                 << "========================================\n"
                 << "  1. Register card\n  2. Recharge card\n  3. Plan route\n  4. Book journey\n"
                 << "  5. Cancel ticket\n  6. Journey history\n  7. Station list\n  0. Exit\n"
                 << "========================================\n";
            int ch = (int)inputNum("Enter choice: ");
            switch (ch) {
                case 1: registerCommuter(); break;
                case 2: recharge(); break;
                case 3: planRoute(); break;
                case 4: bookJourney(); break;
                case 5: cancelTicket(); break;
                case 6: showHistory(); break;
                case 7: listStations(); break;
                case 0: cout << "Thank you for using DelhiMetroX.\n"; return;
                default: cout << "Invalid choice.\n";
            }
        }
    }
};

int main() { DelhiMetroX app; app.run(); }
