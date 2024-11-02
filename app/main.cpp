#include <algorithm>
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

/// ionut

std::vector<edge*> customerEdge; // for end to end

bool validatingId(std::vector<edge*>& ExistingEdges, std::string nodeId);
void sortAuxEdge(std::vector<edge*>& AuxEdge);
bool bkt(std::vector<edge*>& minCost, std::string crrNodeId);

/// end ionut

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
vector<pair<string, int>> functie2(int currentDay);


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
		if (tell_me_type[from_id] == "STORAGE_TANK" && tell_me_type[to_id] == "STORAGE_TANK") {
			tt_out[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tt_in[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}
		else if (tell_me_type[from_id] == "STORAGE_TANK") {
			ct[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tc[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}
		else if (tell_me_type[from_id] == "RAFINERY") {
			rt[tell_me_index[from_id]].push_back({ tell_me_index[to_id], tell_me_index[id] });
			tr[tell_me_index[to_id]].push_back({ tell_me_index[from_id], tell_me_index[id] });
		}

	}
	ine.close();
}

void read_demand(int i) {
	string filename = "result_demands/demand" + to_string(i) + ".txt";
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

void print_demand(int i) {
	string filename = "return_demands/demand" + to_string(i) + ".txt";
	ofstream file(filename);
	file << "hi";
	for (int i = 0; i < output.size(); i++) {
		file << output[i].first << " "<< output[i].second << "\n";
	}
}

std::vector<std::vector<edge*>> minCostAll;

int main()
{

	read_customers();
	read_tanks();
	read_rafineries();

	vector<pair<int, int>> v;
	for (int i = 0; i < rafinery::counter; i++) {
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

    // ionut - ARBORE MIN

	std::vector<edge*> AuxEdge; // aux for current possible solutions

	for (auto& customer : just_customers) {
		std::vector<edge*> minCostLoc;
		bkt(minCostLoc, customer->id);
		minCostAll.push_back(minCostLoc);
	}

	// end ionut

	for (int i = 0; i < 5; i++) {
		read_demand(i);
		//YOU WRITE ALGO JUST HERE

		output = functie2(i);

		cout << "OTHER DAY\n";
		print_demand(i);

	}
}


/// ionut

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
		edge* aux = AuxEdge[i];
		for (int j = i + 1; j < AuxEdge.size(); j++) {
			if (AuxEdge[j]->distance * AuxEdge[j]->max_capacity < aux->distance * AuxEdge[j]->max_capacity) {
				aux = AuxEdge[j];
			}
		}
	}
}

bool bkt(std::vector<edge*>& minCost, std::string crrNodeId) {
	for (auto& refinery : just_rafineries) {
		if (refinery->id == crrNodeId) {
			return true;
		}
	}

	std::vector<edge*> AuxEdge;
	for (auto& edge : edges) {
		if (edge->to_id == crrNodeId) {
			if (validatingId(minCost, crrNodeId)) {
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

	for (auto& edge : AuxEdge) {}
}

/// end ionut


// andrei

class demand2 {
public:
	int index_cust;
	int cant;
	int firstDay;
	int endDay;
};

class transport {
public:
	vector<edge*> path;
	int ziInit;
	float cost;
	int cant;
};

class movee {
public:
	string edge_id;
	int cant;
	int zi;
	movee(string id, int c, int z) {
		edge_id = id;
		cant = c;
		zi = z;
	}
};

vector<movee> moves; //
//vector<vector<edge*>> BP; //(best-paths)
vector<vector<int>> CAT; // (continut-anticipat-tanks) zile-noduri
//vector<demand> demands2;

vector<pair<string, int>> functie2(int currentDay) {
	vector<pair<string, int>> raspuns;
	//trecem prin demanduri
	transport min;
	for (int i = 0; i < demands.size(); i++) {
		//stabilim un minim standard
		for (int zi = currentDay; zi < demands[i]->end_delivery_day; zi++) {
			float dist = 0;
			float timp = 0;
			float penalizare = 0;
			for (int ed = 0; ed < minCostAll[tell_me_index[demands[i]->customer_id]].size(); ed++) {
				edge TE = *minCostAll[tell_me_index[demands[i]->customer_id]][ed]; //temp-edge
				dist += TE.distance;
				timp += TE.lead_time_days;
				if (ed != minCostAll[tell_me_index[demands[i]->customer_id]].size() - 1) { //verificam daca am ajuns la rafinarie
					if (CAT[zi][tell_me_index[TE.to_id]] + demands[i]->quantity > just_tanks[tell_me_index[TE.to_id]]->capacity) {
						penalizare += 1; //tb gasita formula de calcul pt penalizare
					}
					if (zi + timp > demands[i]->end_delivery_day) {
						penalizare += 1;
					}
				}
			}
			//calculam cost total
			float cost = dist + timp + penalizare;
			//actualizam minimul
			if (i == 0) {
				min.path = minCostAll[tell_me_index[demands[i]->customer_id]];
				min.cost = cost;
				min.ziInit = zi;
				min.cant = demands[i]->quantity;
			}
			else {
				if (cost < min.cost) {
					min.path = minCostAll[tell_me_index[demands[i]->customer_id]];
					min.cost = cost;
					min.ziInit = zi;
				}
			}
		}
		//am obtinut minimul
		//generare mutari
		int zileMin = 0;
		for (int i = 0; i < min.path.size(); i++) {
			edge* TE = min.path[i];
			CAT[min.ziInit + zileMin][tell_me_index[TE->to_id]] += min.cant;
			CAT[min.ziInit + zileMin][tell_me_index[TE->from_id]] -= min.cant;
			moves.push_back(movee(TE->id, min.cant, min.ziInit + zileMin));
			zileMin += TE->lead_time_days;
		}
		//adaugare move-uri la fisier de raspuns
		//actualizare CAT!!!


	}
	for (int i = 0; i < moves.size(); i++) {
		if (moves[i].zi == currentDay) {
			raspuns.push_back(pair<string, int>(moves[i].edge_id, moves[i].cant));
			for (int j = i; j < moves.size() - 1; j++) {
				moves[j] = moves[j + 1];
				moves.pop_back();
			}
		}
	}
	return raspuns;
}

// end andrei