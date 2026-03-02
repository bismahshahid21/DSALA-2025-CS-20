#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cstring>
#include <ctype.h> 

using namespace std;

const unsigned int PK = 1;     
const unsigned int NN = 2;     
const unsigned int UNQ = 4;    

class Column {
public:
    string name, type;
    unsigned int constraints;
    Column(string n, string t, unsigned int c) : name(n), type(t), constraints(c) {}
};

class Row {
public:
    vector<string> values;
};

class Table {
public:
    string tableName;
    vector<Column> columns;
    vector<Row*> rows;

    Table() : tableName("") {}
    Table(string name) : tableName(name) {}

    ~Table() {
        clearRows();
    }

    void clearRows() {
       
        for (size_t i = 0; i < rows.size(); ++i) {
            delete rows[i];
        }
        rows.clear();
    }

    bool validateAndInsert(vector<string> v) {
        if (v.size() != columns.size()) {
            cout << "Error: Column count mismatch!\n";
            return false;
        }

        for (size_t i = 0; i < columns.size(); i++) {
            if (columns[i].type == "int") {
                for (size_t j = 0; j < v[i].length(); j++) {
                    if (!isdigit(v[i][j]) && v[i][j] != '-') {
                        cout << "Error: " << columns[i].name << " must be an integer!\n";
                        return false;
                    }
                }
            }
            if ((columns[i].constraints & NN) && (v[i] == "NULL" || v[i] == "")) {
                cout << "Error: " << columns[i].name << " cannot be null!\n";
                return false;
            }
            if ((columns[i].constraints & PK) || (columns[i].constraints & UNQ)) {
                for (size_t k = 0; k < rows.size(); k++) {
                    if (rows[k]->values[i] == v[i]) {
                        cout << "Error: Duplicate value in " << columns[i].name << "!\n";
                        return false;
                    }
                }
            }
        }
        Row* nr = new Row();
        nr->values = v;
        rows.push_back(nr);
        return true;
    }

    void display() {
        if (tableName == "") { cout << "No table created.\n"; return; }
        cout << "\nTable: " << tableName << endl;
        for (size_t i = 0; i < columns.size(); i++) cout << columns[i].name << "\t";
        cout << "\n------------------------------------------\n";
        for (size_t i = 0; i < rows.size(); i++) {
            for (size_t j = 0; j < rows[i]->values.size(); j++) {
                cout << rows[i]->values[j] << "\t";
            }
            cout << endl;
        }
    }

    void save() {
        if (tableName == "") return;
        
        string fileName = tableName + ".txt";
        ofstream f(fileName.c_str());
        f << "TABLE " << tableName << endl;
        for (size_t i = 0; i < columns.size(); i++) {
            f << columns[i].name << " " << columns[i].type << " " << columns[i].constraints << endl;
        }
        f << "DATA" << endl;
        for (size_t i = 0; i < rows.size(); i++) {
            for (size_t j = 0; j < rows[i]->values.size(); j++) {
                f << rows[i]->values[j] << (j == rows[i]->values.size() - 1 ? "" : " ");
            }
            f << endl;
        }
        f.close();
        cout << "Table saved to " << fileName << endl;
    }

    void load(string name) {
        string fileName = name + ".txt";
        ifstream f(fileName.c_str());
        if (!f) { cout << "File not found.\n"; return; }
        
        clearRows(); 
        columns.clear();

        string word, line;
        f >> word >> tableName; 
        while (f >> word && word != "DATA") {
            string t; unsigned int c;
            f >> t >> c;
            columns.push_back(Column(word, t, c));
        }
        getline(f, line); 
        while (getline(f, line)) {
            stringstream ss(line);
            vector<string> vr;
            while (ss >> word) vr.push_back(word);
            if (!vr.empty()) {
                Row* nr = new Row(); nr->values = vr; rows.push_back(nr);
            }
        }
        f.close();
        cout << "Table " << tableName << " loaded successfully.\n";
    }
};

Table db;

void handleCommand(char* buf) {
    string s(buf);
    if (s.find("CREATE TABLE") == 0) {
        string name = s.substr(13);
        db = Table(name);
        int count; cout << "Enter number of columns: "; cin >> count;
        for (int i = 0; i < count; i++) {
            string cn, ct; unsigned int cc;
            cout << "Col " << i+1 << " (Name Type ConstraintFlag): "; cin >> cn >> ct >> cc;
            db.columns.push_back(Column(cn, ct, cc));
        }
        cin.ignore();
        cout << "Table created.\n";
    } 
    else if (s.find("INSERT INTO") == 0) {
        size_t start = s.find("(") + 1; size_t end = s.find(")");
        if (start == 0 || end == string::npos) { cout << "Invalid syntax.\n"; return; }
        string vals = s.substr(start, end - start);
        stringstream ss(vals); string v; vector<string> vec;
        while (getline(ss, v, ',')) {
         
            size_t first = v.find_first_not_of(" ");
            if (string::npos != first) {
                size_t last = v.find_last_not_of(" ");
                vec.push_back(v.substr(first, (last - first + 1)));
            }
        }
        if (db.validateAndInsert(vec)) cout << "Record inserted.\n";
    } 
    else if (s.find("SELECT * FROM") == 0) {
        string tName = s.substr(14);
        if (tName == db.tableName) db.display();
        else cout << "Table " << tName << " not found in memory.\n";
    } 
    else if (s == "SAVE") db.save();
    else if (s.find("LOAD") == 0) db.load(s.substr(5));
}

int main() {
    char buffer[256]; 
    cout << "--- Mini DB Engine (Lab 3) ---\n";
    while (true) {
        cout << "DB> ";
        cin.getline(buffer, 256);
        if (strcmp(buffer, "EXIT") == 0) break;
        handleCommand(buffer);
    }
    return 0;
}
