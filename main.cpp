#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <unordered_map>

using namespace std;

unordered_map<string, int> tell_me_index;
unordered_map<string, string> tell_me_type;


class demand {
public:
	string id;
	string customer_id;
	int quantity;
	int post_day;
	int start_delivery_day;
	int end_delivery_day;
	demand(string id, string customer_id, int quantity, int post, int start, int end) :
		id{ id }, customer_id{ customer_id }, quantity{ quantity }, post_day{ post },
		start_delivery_day{ start }, end_delivery_day{ end } {}
};

class node {
public:
	string id;
	string name;
	string type;
	node(string id, string name, string type) : id{ id }, name{ name }, type{type} {}
};

class tank : public node {
public:
	static int counter;
	int capacity;
	int max_input;
	int max_output;
	float overflow_penalty;
	float underflow_penalty;
	float over_input_penalty;
	float over_output_penalty;
	int initial_stock;
	tank(string id, string name, string type) : node(id, name, type) { counter++; }
};

class rafinery : public node {
public:
	static int counter;
	int number_in_vec;
	int capacity;
	int max_output;
	int production;
	float overflow_penalty;
	float underflow_penalty;
	float over_output_penalty;
	float production_cost;
	float production_co2; 
	float initial_stock;
	rafinery(string id, string name, string type) : node{ id, name, type } { counter++; }
};

class customer : public node {
public:
	static int counter;
	int max_input;
	float over_input_penalty;
	float late_delivery_penalty;
	float early_delivery_penalty;
	customer(string id, string name, string type) : node{ id, name, type } { counter++; }
};

class edge {
public:
	static int counter;
	string id;
	string from_id;
	string to_id;
	int distance;
	int lead_time_days;
	string connection_type;
	int max_capacity;
	edge(string id, string from, string to, int distance, int time, string type, int capacity) :
		id{ id }, from_id{ from }, to_id{ to }, distance{ distance }, lead_time_days{ time },
		connection_type{ type }, max_capacity{ capacity } {
		counter++;
	}
};

int tank::counter = 0;
int rafinery::counter = 0;
int customer::counter = 0;
int edge::counter = 0;

vector<node*> all_nodes;
vector<tank*> just_tanks;
vector<rafinery*> just_rafineries;
vector<customer*> just_customers;
vector<edge*> edges;
vector<edge*> trucks;
vector<edge*> pipelines;

vector<demand*> demands;

vector<pair<int, int>> v;
vector<vector<pair<int, int>>> rt(rafinery::counter, v);
vector<vector<pair<int, int>>> tr(rafinery::counter, v);
vector<vector<pair<int, int>>> ct(customer::counter, v);
vector<vector<pair<int, int>>> tc(customer::counter, v);
vector<vector<pair<int, int>>> tt_out(tank::counter, v);
vector<vector<pair<int, int>>> tt_in(tank::counter, v);


void read_customers() {
	int nr;
	string id, name;
	ifstream inc("customers.txt");
	while (inc >> id) {
		inc >> name;
		inc >> nr;
		name += " " + to_string(nr);
		customer* c = new customer(id, name, "CUSTOMER");
		inc >> c->max_input;
		inc >> c->over_input_penalty;
		inc >> c->late_delivery_penalty;
		inc >> c->early_delivery_penalty;
		inc >> c->type;
		all_nodes.push_back(c);
		just_customers.push_back(c);
		tell_me_index[id] = c->counter - 1;
		tell_me_type[id] = c->type;
	}
	inc.close();
}

void read_tanks() {
	string id;
	string name;
	int nr;
	ifstream ins("tanks.txt");
	while (ins >> id) {
		ins >> name;
		ins >> nr;
		name += " " + to_string(nr);
		tank* t = new tank(id, name, "STORAGE_TANK");
		ins >> t->capacity;
		ins >> t->max_input;
		ins >> t->max_output;
		ins >> t->overflow_penalty;
		ins >> t->underflow_penalty;
		ins >> t->over_input_penalty;
		ins >> t->over_output_penalty;
		ins >> t->initial_stock;
		ins >> t->type;
		all_nodes.push_back(t);
		just_tanks.push_back(t);
		tell_me_index[id] = t->counter - 1;
		tell_me_type[id] = t->type;
	}
	ins.close();
}

void read_rafineries() {
	string id;
	string name;
	int nr;
	ifstream inr("rafineries.txt");
	while (inr >> id) {
		inr >> name;
		inr >> nr;
		name += " " + to_string(nr);
		rafinery* r = new rafinery(id, name, "RAFINERY");
		inr >> r->capacity;
		inr >> r->max_output;
		inr >> r->production;
		inr >> r->overflow_penalty;
		inr >> r->underflow_penalty;
		inr >> r->over_output_penalty;
		inr >> r->production_cost;
		inr >> r->production_co2;
		inr >> r->initial_stock;
		inr >> r->type;
		all_nodes.push_back(r);
		just_rafineries.push_back(r);
		tell_me_index[id] = r->counter - 1;
		tell_me_type[id] = r->type;
	}
	inr.close();
}

void read_connections() {
	string id;
	ifstream ine("connections.txt");
	string from_id;
	string to_id;
	int distance;
	int lead_time_days;
	string connection_type;
	int max_capacity;
	while (ine >> id) {
		ine >> from_id;
		ine >> to_id;
		ine >> distance;
		ine >> lead_time_days;
		ine >> connection_type;
		ine >> max_capacity;
		edge* e = new edge(id, from_id, to_id, distance, lead_time_days, connection_type, max_capacity);
		tell_me_index[id] = e->counter - 1;
		tell_me_type[id] = e->connection_type;
		edges.push_back(e);
		if (connection_type == "TRUCK") {
			trucks.push_back(e); // index doesn t work for this
			if (tell_me_type[from_id] == "STORAGE_TANK") {
				int id1 = tell_me_index[from_id];
				int id2 = tell_me_index[to_id];
				ct[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
				tc[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
			}
		}
		else if (connection_type == "PIPELINE") {
			pipelines.push_back(e); // index doesn t work for this
			if (tell_me_type[from_id] == "RAFINERY") {
				rt[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
				tr[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
			}
		}
	}
	ine.close();
}

void read_demand(int i) {
	string filename = "demand" + to_string(i) + ".txt";
	ifstream file(filename);
	while (!file.is_open()) {
		cout << filename << endl;
		cout << "DOESN T EXIST" << endl;

	}
	demands.clear();
	string id;
	string customer_id;
	int quantity;
	int post;
	int start;
	int end;
	while (file >> id) {
		file >> customer_id;
		file >> quantity;
		file >> post;
		file >> start;
		file >> end;
		demand* d = new demand(id, customer_id, quantity, post, start, end);
		demands.push_back(d);
	}
}

int main()
{
	read_customers();
	read_tanks();
	read_rafineries();
	read_connections();

	for (int i = 1; i <= 42; i++) {
		read_demand(i);
		cout << "OTHER DAY\n";

	}

}
