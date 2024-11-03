#include <algorithm>
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <random>
#include <thread>
#include <filesystem>

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
	node(string id, string name, string type) : id{ id }, name{ name }, type{ type } {}
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

std::vector<edge*> customerEdge; // for end to end

bool validatingId(std::vector<edge*>& ExistingEdges, std::string nodeId);
void sortAuxEdge(std::vector<edge*>& AuxEdge);
bool bkt(std::vector<edge*>& minCost, std::string crrNodeId);

vector<node*> all_nodes;
vector<tank*> just_tanks;
vector<customer*> just_customers;
vector<rafinery*> just_rafineries;
vector<edge*> edges;
vector<edge*> trucks;
vector<edge*> pipelines;
vector<demand*> demands;

vector<vector<pair<int, int>>> rt;
vector<vector<pair<int, int>>> tr;
vector<vector<pair<int, int>>> ct;
vector<vector<pair<int, int>>> tc;
vector<vector<pair<int, int>>> tt_out;
vector<vector<pair<int, int>>> tt_in;

vector<pair<string, int>> output;
vector<pair<string, int>> solve(int currentDay);

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
		rafinery* r = new rafinery(id, name, "REFINERY");
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
		if (tell_me_type[from_id] == "STORAGE_TANK" && tell_me_type[to_id] == "STORAGE_TANK") {
			tt_out[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tt_in[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}
		else if (tell_me_type[from_id] == "STORAGE_TANK") {
			ct[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tc[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}
		else if (tell_me_type[from_id] == "REFINERY") {
			rt[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tr[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}

	}
	ine.close();
}

void demand_sort() {
	for (int i = 0; i < demands.size()-1; i++) {
		for (int j = i+1; j < demands.size(); j++) {
			if (demands[i]->end_delivery_day > demands[j]->end_delivery_day) {
				auto aux = demands[i];
				demands[i] = demands[j];
				demands[j] = aux;
			}
		}
	}
}

void read_demand(int i) {
	string filename = "result_demands/demand" + to_string(i) + ".txt";
	while (true) {
		ifstream file(filename);
		if(filesystem::exists(filename)) {
			string id;
			string customer_id;
			int quantity;
			int post;
			int start;
			int end;
	while (!file.is_open()) {
		cout << filename << endl;
		cout << "DOESN T EXIST" << endl;

	}
	// demands.clear();

	file.close();
			while (file >> id) {
				customer_id = id;
				file >> quantity;
				file >> post;
				file >> start;
				file >> end;
				demand* d = new demand(id, customer_id, quantity, post, start, end);
				demands.push_back(d);
			}
			break;
		}
		else {
			cout<<filename<<" does not exist"<<endl;
		}
		file.close();
		this_thread::sleep_for(chrono::seconds(1));
	}
	demand_sort();
}

void print_demand(int i) {
	string filename = "give_demands/demand" + to_string(i) + ".txt";
	ofstream file(filename);
	
	for (int i = 0; i < output.size(); i++) {
		file << output[i].first << " "<< output[i].second << "\n";
	}
}

std::vector<std::vector<edge*>> minCostAll;

vector<string> visitedNodeIds;
vector<edge*> finalRoute;
int getCost(string refId);
long long myAlg(string nodeId);

vector<pair<string, int>> fillFirst(int currentDay) {
	vector<pair<string, int>> raspuns;

	for(int i = 0; i < minCostAll.size(); i++) {
		auto v_edge = minCostAll[i];
		auto lastEdge = v_edge[v_edge.size()-1];
		for (auto& tank : just_tanks) {
			if (tank->id == lastEdge->to_id) {
				int min = tank->max_input;
				if (tank->capacity - tank->initial_stock > 0) {
					if (min > lastEdge->max_capacity) {
						min = lastEdge->max_capacity;
					}
					tank->initial_stock += min;
					raspuns.push_back(std::pair<string,int>(lastEdge->id, min));
				}
				break;
			}
		}
	}

	return raspuns;
}

int main()
{
	read_customers();
	read_tanks();
	read_rafineries();

	vector<pair<int, int>> v;
	for (int i = 0; i < tank::counter; i++) {
		rt.push_back(v);
		tr.push_back(v);
	}
	for (int i = 0; i < customer::counter; i++) {
		ct.push_back(v);
		tc.push_back(v);
	}
	for (int i = 0; i < tank::counter; i++) {
		tt_out.push_back(v);
		tt_in.push_back(v);
	}

	read_connections();

	for (auto& customer : just_customers) {
		std::vector<edge*> minCostLoc;
		bkt(minCostLoc, customer->id);
		minCostAll.push_back(minCostLoc);
	}

	for (int i = 0; i < 42; i++) {
		read_demand(i);
		output = solve(i);
		cout << "Day completed!\n";
		print_demand(i);
	}
}

bool validatingId(std::vector<edge*>& ExistingEdges, std::string nodeId) {
	for (auto& edge : ExistingEdges) {
		if (edge->to_id == nodeId || edge->from_id == nodeId) {
			return false;
		}
	}
	return true;
}

void sortAuxEdge(std::vector<edge*>& AuxEdge) {
	for (int i = 0; i + 1 < AuxEdge.size(); i++) {
		for (int j = i + 1; j < AuxEdge.size(); j++) {
			if (AuxEdge[j]->distance / AuxEdge[j]->max_capacity < AuxEdge[i]->distance / AuxEdge[j]->max_capacity ) {
				auto aux = AuxEdge[j];
				AuxEdge[j] = AuxEdge[i];
				AuxEdge[i] = aux;
			}
		}
	}
}

bool bkt(std::vector<edge*>& minCost, std::string crrNodeId) {
	if(tell_me_type[crrNodeId] == "REFINERY") {return true;}

	std::vector<edge*> AuxEdge;

	for (auto& edge : edges) {
		if (edge->to_id == crrNodeId) {
			if (validatingId(minCost, edge->from_id)) {
				AuxEdge.push_back(edge);
			}
		}
	}

	sortAuxEdge(AuxEdge);

	for (auto& edge : AuxEdge) {
		minCost.push_back(edge);
		if (bkt(minCost, edge->from_id)) { return true; }
		minCost.pop_back();
	}

}

vector<vector<int>> CAT;

vector<pair<string, int>> solve(int currentDay) {
	vector<pair<string, int>> raspuns;

	for (auto& demand : demands) {
		if (demand->start_delivery_day == currentDay) {
			vector<edge*> temp = minCostAll[tell_me_index[demand->customer_id]];
			for (auto& edge : temp) {
				raspuns.push_back(std::pair<string, int>(edge->id, demand->quantity));
			}
		}
	}
	return raspuns;
}



int getCost(string refId) {
	for (auto& ref : just_rafineries) {
		if (ref->id == refId) {
			return (ref->production_cost + ref->production_co2 + ref->overflow_penalty + ref->underflow_penalty
				- ref->capacity - ref->max_output - ref->initial_stock);
		}
	}
}

long long myAlg(string nodeId) {
	for (auto& nod : visitedNodeIds) {
		if (nod == nodeId) {
			return INT32_MAX;
		}
	}

	visitedNodeIds.push_back(nodeId);

	if(tell_me_type[nodeId] == "REFINERY") {
		return getCost(nodeId) ;
	}

	unordered_map<long long, edge*> costs;
	int minCost;

	for (auto& edge : edges) {
		if (edge->to_id == nodeId) {
			int c = myAlg(edge->from_id);

			costs.insert(make_pair(c + edge->distance, edge));

			minCost = c + edge->distance + edge->max_capacity;
		}
	}

	for(auto& [cost, edge] : costs) {
		if (cost < minCost) { minCost = cost; }
	}

	finalRoute.push_back(costs[minCost]);

	return minCost;
}