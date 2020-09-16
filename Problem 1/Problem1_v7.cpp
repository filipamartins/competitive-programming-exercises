// Filipa Martins 2016267248
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <climits>
#include <numeric>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <utility>
#include <chrono>

using namespace std::chrono;
using namespace std;

class Point
{
public:
    int x;
    int y;

    Point(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
};

class Antenna
{
public:
    int radius;
    int cost;

    Antenna(int radius, int cost)
    {
        this->radius = radius;
        this->cost = cost;
    }
};

//GLOBAL VARIABLES
vector<Antenna> ant_types;
vector<Point> listeners, ant_pos;
unordered_map<int, unordered_map<int, vector<int>>> apos_listeners; // Key: antenna type; Value: Map(key: position Value: listerners covered)
unordered_map<int, vector<pair<int, int>>> listener_apos;           // Key: listener; Value: Pairs of Antenna type - Position
int *covered;                                                       // Array of covered listeners                                                     
int *solution;                                                      // Array to test solutions

bool compareAntennas(Antenna a1, Antenna a2)
{
    if (a1.cost == a2.cost)
        return (a1.radius > a2.radius);
    else
        return (a1.cost < a2.cost);
}

bool compareAntennaPosition(const tuple<int, int, int, int, int> &p1, 
                            const tuple<int, int, int, int, int> &p2)
{
    if ((get<2>(p1) / get<4>(p1)) == (get<2>(p2) / get<4>(p2)))
        return get<3>(p1) > get<3>(p2); // radius
    else
        return (get<2>(p1) / get<4>(p1)) < (get<2>(p2) / get<4>(p2)); // cost/number_of_listeners
}

bool compareListeners(const tuple<int, int> &p1, const tuple<int, int> &p2)
{
    return get<1>(p1) < get<1>(p2); // number of antenna position pairs that cover listener
}

void printArray(int a[], int n)
{
    cout << "[ ";
    for (int i = 0; i < n; i++)
    {
        cout << a[i] << " ";
    }
    cout << "]\n";
}

vector<string> stringToVector(string str, char cSpace)
{
    vector<string> vecsWords;
    stringstream ss(str); // spits strings separated by spaces 1 by 1
    string s;

    while (std::getline(ss, s, cSpace))
    {
        vecsWords.push_back(s);
    }
    return vecsWords;
}

void readInput()
{
    string input;

    //read listeners
    std::getline(cin, input);
    int n_listeners = stoi(input);
    for (int i = 0; i < n_listeners; i++)
    {
        std::getline(cin, input);
        vector<string> str = stringToVector(input, ' ');
        Point p = Point(stoi(str[0]), stoi(str[1]));
        listeners.push_back(p);
    }

    //read antennas
    std::getline(cin, input);
    int n_antennas = stoi(input);
    for (int i = 0; i < n_antennas; i++)
    {
        std::getline(cin, input);
        vector<string> str = stringToVector(input, ' ');
        Point p = Point(stoi(str[0]), stoi(str[1]));
        ant_pos.push_back(p);
    }

    //read antennas types
    std::getline(cin, input);
    int n_types = stoi(input);
    for (int i = 0; i < n_types; i++)
    {
        std::getline(cin, input);
        vector<string> str = stringToVector(input, ' ');
        Antenna a = Antenna(stoi(str[0]), stoi(str[1]));
        ant_types.push_back(a);
    }
}

bool isInsideCircle(int circle_x, int circle_y, int r, int x, int y)
{
    // Compare radius of circle with distance of its center 
    // from given point
    if ((x - circle_x) * (x - circle_x) +
        (y - circle_y) * (y - circle_y) <= r * r)
        return true;
    else
        return false;
}

// Since antenna types are already sorted, if an antenna has the same cost of 
// the previous antenna in the list, its radius is less or equal than the radius 
// of the previous antenna. If an antenna has largest cost than the previous we 
// only need to consider the one with largest radius.
void filterAntennaTypes()
{
    int atual_cost, atual_radius;
    int previous_cost = INT_MAX;
    int previous_max_radius = 0;
    vector<Antenna> ant_filtered;

    for (auto a : ant_types)
    {
        atual_cost = a.cost;
        atual_radius = a.radius;
        if (atual_cost == previous_cost)
            continue;
        else
        { // atual cost > previous cost
            if (atual_radius <= previous_max_radius)
                continue;
        }
        previous_cost = atual_cost;
        previous_max_radius = atual_radius;
        ant_filtered.push_back(a);
    }
    ant_types = ant_filtered;
}


bool saveCoveredListeners(){
    unordered_set<int> total_covered;
    size_t n_listeners = listeners.size(); // number of listeners
    int n_types = ant_types.size();        // number of types of antennas
    int n_pos = ant_pos.size();            // number of antennas positions

    for (int i = 0; i < n_types; i++) //antenna types
    {
        for (int j = 0; j < n_pos; j++) //positions
        {
            vector<int> covered_listeners;
            for (size_t k = 0; k < n_listeners; k++) //listeners
            {
                if (isInsideCircle(ant_pos[j].x, ant_pos[j].y, ant_types[i].radius, listeners[k].x, listeners[k].y))
                {
                    covered_listeners.push_back(k);
                    total_covered.insert(k);
                }
            }
            // Only antennas in a position with covered listeners are considered
            if (covered_listeners.size() != 0)
            {
                apos_listeners[i + 1][j] = covered_listeners; // populate map antenna_position - listeners
                for (auto l : covered_listeners)              // populate map listener - antennas_positions
                {
                    pair<int, int> ap = make_pair(i + 1, j);
                    listener_apos[l].push_back(ap);
                }
            }
        }
    }
    if (total_covered.size() != n_listeners)
    {
        return false;
    }
    return true;
}

bool processData()
{
    size_t n_listeners = listeners.size(); // number of listeners
    // Sort antennas types by cost and then by radius
    sort(ant_types.begin(), ant_types.end(), compareAntennas);

    filterAntennaTypes();

    // If there are listeners that cannot be covered with 
    // any antenna the program should return no solution.
    if(!saveCoveredListeners())
        return false;    

    // SORT LISTENERS: Ascending order regarding the 
    // number of antenna position pairs that cover them. 

    // Construct vector of tuples with index of listener 
    // and number of antenna position pairs
    vector<tuple<int, int>> list_aps;
    for (auto l : listener_apos)
    {
        tuple<int, int> list_ap{l.first, (int)l.second.size()};
        list_aps.push_back(list_ap);
    }
   
    // Sort vector of tuples by number of antenna position pairs
    sort(list_aps.begin(), list_aps.end(), compareListeners);
    
    // Construct new vector of listeners
    vector<Point> listeners_sorted;
    for (size_t i = 0; i < n_listeners; i++)
    {
        int idx_list = get<0>(list_aps[i]);
        listeners_sorted.push_back(listeners[idx_list]);
    }
    listeners = listeners_sorted;
    
    // Update covered listeners Maps
    apos_listeners.clear();
    listener_apos.clear();
    saveCoveredListeners();

    // SORT ANTENNA POSITION PAIRS: Descending order 
    // regarding the number of listeners covered
    for (auto kv : listener_apos)
    {
        vector<tuple<int, int, int, int, int>> apls;
        for (auto p : kv.second)
        {
            vector<int> lts = apos_listeners[p.first][p.second];
            tuple<int, int, int, int, int> apl{p.first, p.second, ant_types[p.first - 1].cost, ant_types[p.first - 1].radius, (int)lts.size()};
            apls.push_back(apl);
        }
      
        sort(apls.begin(), apls.end(), compareAntennaPosition);

        vector<pair<int, int>> sorted;
        for (auto t : apls)
        {
            pair<int, int> p = make_pair(get<0>(t), get<1>(t));
            sorted.push_back(p);
        }
        listener_apos[kv.first] = sorted;
    }

    return true;
}

int updateCovered(int antenna, int pos, int value)
{
    vector<int> covered_listeners = apos_listeners[antenna][pos];
    int new_covered = 0;
    for (auto l : covered_listeners) //for each listener covered by antenna in position
    {
        if (covered[l] == 0){
            new_covered++;
        }
        covered[l] += value; //update covered listeners array
    }
    return new_covered;
}

// backtracking function: recursion by listeners
void minimizeCost(size_t covered_listeners, int i, int n, int previouscost, int cost, int &min_cost)
{   
    // Rejection Test
    if (cost >= min_cost)
        return;
    
    // Base Case
    if (cost != previouscost)
    {   // check if valid solution: coverage of all listeners
        if (covered_listeners == listeners.size())
        {
            min_cost = cost;
            return;
        }
    }

    // Rejection Case
    if (n == 0)
        return;


    for (auto ap : listener_apos[i])
    {
        int new_covered;
        // skip antenna-position if position already occupied
        if (solution[ap.second] != 0)
            continue;
        
        new_covered = updateCovered(ap.first, ap.second, 1);
        // skip antenna-position if no additional listeners covered
        /*if(new_covered == 0){                                  
            updateCovered(ap.first, ap.second, -1);
            continue;
        }*/

        covered_listeners += new_covered;  
        solution[ap.second] = ap.first; 

        // skip next listener if already covered by one antenna-position 
        int new_listener = i + 1;
        while (covered[new_listener])   
        {
            new_listener++;
        }

        minimizeCost(covered_listeners, new_listener, n - 1, cost, cost + ant_types[ap.first - 1].cost, min_cost);
        updateCovered(ap.first, ap.second, -1);
        covered_listeners -= new_covered; 
        solution[ap.second] = 0;
    }
}
   
int main()
{
    //auto start = high_resolution_clock::now();
    readInput();
   
    if (processData())
    {
        int min = INT_MAX;
        int n_pos = ant_pos.size();
        int n_list = listeners.size();

        solution = new int[n_pos];
        fill(solution, solution + n_pos, 0);

        covered = new int[n_list]; 
        fill(covered, covered + n_list, 0);

        minimizeCost(0, 0, n_pos, 0, 0, min);       // covered_listeners, index of first listener,
                                                    // number of positions, previous cost, atual cost, minimum cost
        if (min != INT_MAX)
            printf("%d\n", min);
        else
            printf("no solution\n");
    }
    else
        printf("no solution\n");
    /*
    auto stop = high_resolution_clock::now();
    auto duration = (duration_cast<microseconds>(stop - start));
    std::cout << duration.count() << endl;*/
    return (0);
}