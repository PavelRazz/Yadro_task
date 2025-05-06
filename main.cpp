#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <queue>
#include <algorithm>

class Time {
public:
    int minutes;

    Time() : minutes(0) {}

    explicit Time(const std::string& s) {
        minutes = parse(s);
    }
    Time(int mins) : minutes(mins) {}

    std::string t_string() const
    {
        int h = minutes / 60;
        int m = minutes % 60;
        std::string hh = (h < 10 ? "0" : "") + std::to_string(h);
        std::string mm = (m < 10 ? "0" : "") + std::to_string(m);
        return hh + ":" + mm;
    }

    static bool valid(const std::string& s) {
        int m = Time(s).minutes;
        return m >= 0;
    }
private:
    int parse(const std::string& s)
    {
        if (s.size() != 5 || s[2] != ':')  return -1;
        int h = std::stoi(s.substr(0, 2));
        int m = std::stoi(s.substr(3, 2));
        if (h < 0 || h >= 24 || m < 0 || m > 59) return -1;
        return h * 60 + m;
    }
};

class Event {
public:
    Time time;
    int id;
    std::vector<std::string> arg;
    Event(const std::string& s, int id, const std::vector<std::string>& a) : time(Time(s)), id(id), arg(a) {}
};

class Table {
public:
    int id;
    bool busy;
    Time start_time;
    int use_minutes;
    int revenue;
    std::string client;
    Table(int id) : id(id), busy(false), start_time(), use_minutes(0), revenue(0) {}

    void occupy(const std::string& c, const Time& t) {
        busy = true;
        client = c;
        start_time = t;
    }

    void free(const Time& t, int price)
    {
        int play_time = t.minutes - start_time.minutes;
        use_minutes += play_time;
        int hours = (play_time + 59) / 60;
        busy = false;
        revenue += hours * price;
    }
};

class Club {
public:
    int n_tables;
    Time open, close;
    int price;
    std::map<std::string, int> client; // -1 если он в клубе, 0 если в очереди, №компа если за столом
    std::vector<Table> tables;
    std::queue<std::string> wait_queue;
    std::vector<Event> events;

    Club(int n, const std::string& open, const std::string& close, int price) :n_tables(n), open(Time(open)), close(Time(close)), price(price) {
        for (int i = 1; i <= n; ++i)
        {
            tables.emplace_back(i);
        }
    }

    void process_event(const Event& e)
    {
        std::cout << e.time.t_string() << " " << e.id;
        for (const auto& arg : e.arg)
            std::cout << " " << arg;
        std::cout << std::endl;


        if (e.id == 1) {
            std::string client_name = e.arg[0];
            if (e.time.minutes < open.minutes)
                std::cout << e.time.t_string() << " 13 NotOpenYet" << std::endl;
            else if (client.count(client_name))
                std::cout << e.time.t_string() << " 13 YouShallNotPass" << std::endl;
            else
                client[client_name] = -1;
        }
        else if (e.id == 2) {
            std::string client_name = e.arg[0];
            int table_num = std::stoi(e.arg[1]);
            if (!client.count(client_name))
                std::cout << e.time.t_string() << " 13 ClientUnknown" << std::endl;
            else if (tables[table_num - 1].busy)
                std::cout << e.time.t_string() << " 13 PlaceIsBusy" << std::endl;
            else {
                if (client[client_name] > 0)
                    tables[client[client_name] - 1].free(e.time, price);
                if (client[client_name] == 0)
                    remove_from_queue(client_name);
                tables[table_num - 1].occupy(client_name, e.time);
                client[client_name] = table_num;
            }
        }
        else if (e.id == 3) {
            std::string client_name = e.arg[0];
            bool has_free_table = false;
            for (const auto& table : tables) {
                if (!table.busy) {
                    has_free_table = true;
                    break;
                }
            }
            if (has_free_table)
                std::cout << e.time.t_string() << " 13 ICanWaitNoLonger!" << std::endl;
            else if (wait_queue.size() >= n_tables) {
                std::cout << e.time.t_string() << " 11 " << client_name << std::endl;
                client.erase(client_name);
            }
            else {
                if (client[client_name] > 0)
                    tables[client[client_name] - 1].free(e.time, price);
                if (client[client_name] != 0) {
                    wait_queue.push(client_name);
                    client[client_name] = 0;
                }
            }
        }
        else if (e.id == 4) {
            std::string client_name = e.arg[0];
            if (!client.count(client_name))
                std::cout << e.time.t_string() << " 13 ClientUnknown" << std::endl;
            else {
                if (client[client_name] > 0)
                {
                    int table_num = client[client_name];
                    tables[client[client_name] - 1].free(e.time, price);
                    if (!wait_queue.empty()) {
                        std::string next_client = wait_queue.front();
                        wait_queue.pop();
                        tables[table_num - 1].occupy(next_client, e.time);
                        client[next_client] = table_num;
                        std::cout << e.time.t_string() << " 12 " << next_client << " " << table_num << std::endl;
                    }
                }
                else if (client[client_name] == 0) {
                    remove_from_queue(client_name);
                }
                client.erase(client_name);
            }
        }
    }
    void close_club() {
        std::vector<std::string> remaining_clients;
        for (const auto& c : client)
            remaining_clients.push_back(c.first);
        std::sort(remaining_clients.begin(), remaining_clients.end());
        for (const auto& client_name : remaining_clients)
        {
            if (client[client_name] > 0)
                tables[client[client_name] - 1].free(close, price);
            std::cout << close.t_string() << " 11 " << client_name << std::endl;
        }
        std::cout << close.t_string() << std::endl;
        for (const auto& table : tables)
            std::cout << table.id << " " << table.revenue << " " << Time(table.use_minutes).t_string() << std::endl;
    }
private:
    void remove_from_queue(const std::string& client_name) {
        std::queue<std::string> temp;
        while (!wait_queue.empty()) {
            if (wait_queue.front() != client_name) temp.push(wait_queue.front());
            wait_queue.pop();
        }
        wait_queue = temp;
    }
};
bool valid_name(const std::string& s) {
    if (s.empty()) return false;
    for (char c : s) {
        if (!(std::islower(static_cast<unsigned char>(c)) ||
            std::isdigit(static_cast<unsigned char>(c)) ||
            c == '_' || c == '-')) return false;
    }
    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 2) { std::cerr << "Usage: " << argv[0] << " <input_file>\n"; return 1; }
    std::ifstream file(argv[1]);
    if (!file.is_open()) { std::cerr << "Error: Cannot open file " << argv[1] << "\n"; return 1; }

    std::string line;
    int line_no = 0;

 
    if (!std::getline(file, line)) return 0;
    ++line_no;
    for (char c : line) if (!std::isdigit(static_cast<unsigned char>(c))) { std::cout << line << "\n"; return 1; }
    int n_tables = std::stoi(line);

    if (!std::getline(file, line)) return 0;
    ++line_no;
    if (line.size() < 11 || line[5] != ' ' || !Time::valid(line.substr(0, 5)) || !Time::valid(line.substr(6, 5))) {
        std::cout << line << "\n"; return 1;
    }
    std::string open_s = line.substr(0, 5), close_s = line.substr(6, 5);


    if (!std::getline(file, line)) return 0;
    ++line_no;
    for (char c : line) if (!std::isdigit(static_cast<unsigned char>(c))) { std::cout << line << "\n"; return 1; }
    int price = std::stoi(line);

    Club club(n_tables, open_s, close_s, price);
    std::cout << club.open.t_string() << "\n";

    int last_time = -1;

    while (std::getline(file, line)) {
        ++line_no;
        if (line.empty()) { std::cout << line << "\n"; return 1; }

        std::vector<std::string> tok;
        size_t pos = 0;
        while (pos < line.size()) {
            size_t nxt = line.find(' ', pos);
            tok.push_back(line.substr(pos, nxt - pos));
            if (nxt == std::string::npos) break;
            pos = nxt + 1;
        }
        if (tok.size() < 2) { std::cout << line << "\n"; return 1; }

        if (!Time::valid(tok[0])) { std::cout << line << "\n"; return 1; }
        int cur = Time(tok[0]).minutes;
        if (cur < last_time) { std::cout << line << "\n"; return 1; }
        last_time = cur;

        int id;
        try { id = std::stoi(tok[1]); }
        catch (...) { std::cout << line << "\n"; return 1; }
        if (id < 1 || id>4) { std::cout << line << "\n"; return 1; }

        int exp_args = (id == 2 ? 2 : 1);
        if ((int)tok.size() != 2 + exp_args) { std::cout << line << "\n"; return 1; }

        const std::string& name = tok[2];
        if (!valid_name(name)) { std::cout << line << "\n"; return 1; }

        if (id == 2) {
            int tbl;
            try { tbl = std::stoi(tok[3]); }
            catch (...) { std::cout << line << "\n"; return 1; }
            if (tbl<1 || tbl>n_tables) { std::cout << line << "\n"; return 1; }
        }
        club.process_event(Event(tok[0], id, std::vector<std::string>(tok.begin() + 2, tok.end())));
    }
    club.close_club();
    return 0;
}

