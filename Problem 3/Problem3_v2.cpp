// Filipa Martins 2016267248
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

using namespace std;

#define INF 99999

//  ======================================================================================================= UTILITY FUNCTIONS
void printAdjacencyList(vector<pair<int, int>> adj[], int nodes)
{
    for (int i = 0; i < nodes; i++)
    {
        for (auto p : adj[i])
        {
            //      u               v                 w
            cout << i << " " << p.first << " " << p.second << "\n";
        }
    }
}

void printEdges(vector<tuple<int, int, int>> &edges)
{
    for (auto e : edges)
        cout << get<1>(e) << " " << get<2>(e) << " " << get<0>(e) << "\n";
}

void addEdge(vector<pair<int, int>> adj[], int u, int v, int w)
{
    // the graph is undirected, so each edge
    // is added in both directions
    adj[u - 1].push_back({v - 1, w});
    adj[v - 1].push_back({u - 1, w});
}

// ======================================================================================================= FLOYDWARSHAL
// Solves the all-pairs shortest path problem
// dist[][] - output matrix to save the shortest
// distances between every pair of vertices
int **floydWarshall(vector<tuple<int, int, int>> &edges, int nodes)
{
    int **dist, i, j, k;

    dist = new int *[nodes];
    for (int i = 0; i < nodes; i++)
        dist[i] = new int[nodes];

    // Initially, the distance from each node to itself is 0, and the
    // distance between nodes a and b is x if there is an edge between
    // nodes a and b with weight x. All other distances are inﬁnite.
    for (int i = 0; i < nodes; i++)
    {
        for (int j = 0; j < nodes; j++)
        {
            if (i == j)
                dist[i][j] = 0;
            else
                dist[i][j] = INF;
        }
    }

    for (auto e : edges)
    {
        int w = get<0>(e), u = get<1>(e), v = get<2>(e);
        dist[u][v] = dist[v][u] = w;
    }

    // On each round, a new node that can act as an intermediate node in
    // paths is selected and distances are reduced using this node.
    for (k = 0; k < nodes; k++)
    {
        // Pick all vertices as source
        for (i = 0; i < nodes; i++)
        {
            // Pick all vertices as destination for above source
            for (j = 0; j < nodes; j++)
            {
                // If vertex k is on the shortest path from
                // i to j, then update the value of dist[i][j]
                dist[i][j] = min(dist[i][j], dist[i][k] + dist[k][j]);
            }
        }
    }
    return dist;
}

// ======================================================================================================= KRUSKAL_MST
// Initially, all vertices are in different sets and have rank 0
void make_set(int *set, int *rank, int nodes)
{
    for (int i = 0; i < nodes; i++)
    {
        set[i] = i;
        rank[i] = 0;
    }
}

// Find the set(root) of a node 'u'
// Path Compression - connect all the descendents
// in the path from u--> set[u] to set[u]
int find(int *set, int u)
{
    if (set[u] != u)
        set[u] = find(set, set[u]);
    return set[u];
}

// Connect the root of the smaller (height) tree to the root of the
// larger tree, which shortens the time to reach the root
void Union(int *set, int *rank, int u, int v)
{
    int x = find(set, u), y = find(set, v);

    if (rank[x] > rank[y])
        set[y] = x;
    else // If rnk[x] <= rnk[y]
        set[x] = y;

    if (rank[x] == rank[y])
        rank[y]++;
}

// Creates a MST with all nodes of the graph and some of the edges so that
// there is a path between any two nodes and weight is as small as possible
void KruskalMST(vector<tuple<int, int, int>> &edges, int nodes, vector<tuple<int, int, int>> &new_edges)
{
    int *set = new int[nodes];
    int *rank = new int[nodes];

    // Create set for each node
    make_set(set, rank, nodes);

    // Sort edges by weight
    sort(edges.begin(), edges.end());

    for (auto e : edges)
    {
        int w = get<0>(e);
        int u = get<1>(e);
        int v = get<2>(e);
        if (find(set, u) != find(set, v))
        {
            // edge will be in the MST
            tuple<int, int, int> t{w, u, v};
            new_edges.push_back(t);
            Union(set, rank, u, v);
        }
    }
}

// ======================================================================================================= ARTICULATION POINTS
// Function that find articulation points using DFS traversal
// u     - root of DFS tree / vertex to be visited next
// d     - discovery time
// dfs[] - Stores discovery times of visited vertices
// ap[]  - Store articulation points
void articulationPoints(vector<pair<int, int>> *adj, int &d, int u, bool visited[],
                        int dfs[], int low[], int parent[], bool ap[])
{
    int children = 0;

    visited[u] = true;

    // Initialize discovery time and low value
    dfs[u] = low[u] = ++d;

    // Go through all vertices adjacent to this
    for (auto p : adj[u])
    {
        int v = p.first; // v is current adjacent of u

        // If v is not visited yet, then make it a child of u
        // in DFS tree and recur for it
        if (!visited[v])
        {
            children++;
            parent[v] = u;
            articulationPoints(adj, d, v, visited, dfs, low, parent, ap);

            // Check if the subtree has a connection to
            // one of the ancestors of u
            low[u] = min(low[u], low[v]);

            // u is an articulation point in following cases
            // (1) u is root of DFS tree and has two or more children.
            if (dfs[u] == 1 && children > 1)
                ap[u] = true;

            // (2) If u is not root and low value of one of its child is more
            // than discovery value of u.
            if (dfs[u] != 1 && low[v] >= dfs[u])
                ap[u] = true;
        }

        // Update low value of u for parent function calls.
        else if (v != parent[u])
            low[u] = min(low[u], dfs[v]);
    }
}

// ======================================================================================================= SERVERS + CABLE CALCULATION
// Function that for each group of connected components of graph:
// (1) Finds articulation points (servers) - DFS
// (2) Calculates all-pairs shortest path problem - floydWarshall
// (3) Calculates least amount of cable by directly connect all pairs of servers
// (4) Calculates minimum spanning tree (MST) only with edges connecting servers - KruskalMST
// (5) Calculates least amount of cable using a tree topology
void minCable(vector<pair<int, int>> *adj, vector<tuple<int, int, int>> &edges,
              int nodes, int &servers, int &c_pairwise, int &c_tree)
{
    // Articulation Points variables and Initialization
    int *dfs = new int[nodes];
    int *low = new int[nodes];
    int *parent = new int[nodes];
    bool *visited = new bool[nodes];
    bool *ap = new bool[nodes];
    int d;

    for (int i = 0; i < nodes; i++)
    {
        parent[i] = nodes;
        visited[i] = false;
    }

    // FloydWarshal variables
    bool floyd_calculated = false;
    int **dist;

    for (int i = 0; i < nodes; i++)
    {
        // initialization
        d = 0;
        for (int i = 0; i < nodes; i++)
            ap[i] = false;

        // Find all connected components of graph by starting
        // a new DFS if the current node is not visited yet
        if (visited[i] == false)
        {
            // ==================(1)=======================================
            articulationPoints(adj, d, i, visited, dfs, low, parent, ap);

            vector<int> aps;
            for (int i = 0; i < nodes; i++)
            {
                if (ap[i] == true)
                    aps.push_back(i);
            }
            servers += aps.size();

            if (aps.size() <= 1)
                continue;

            // ==================(2)=======================================
            if (!floyd_calculated)
            {
                dist = floydWarshall(edges, nodes);
                floyd_calculated = true;
            }

            // ==================(3)=======================================
            vector<tuple<int, int, int>> server_edges;
            for (size_t i = 0; i < aps.size(); i++)
            {
                for (size_t j = i + 1; j < aps.size(); j++)
                {
                    c_pairwise += dist[aps[i]][aps[j]];

                    // save only the servers and the shortest pairwise edges/paths that connect them
                    // the weight of the edges will be the distance of the pairwise paths
                    tuple<int, int, int> t{dist[aps[i]][aps[j]], aps[i], aps[j]};
                    server_edges.push_back(t);
                }
            }

            // ==================(4)=======================================
            vector<tuple<int, int, int>> new_edges;
            KruskalMST(server_edges, nodes, new_edges);

            // ==================(5)=======================================
            for (auto e : new_edges)
                c_tree += get<0>(e);
        }
    }
}

// ======================================================================================================= INPUT
void readInput(vector<pair<int, int>> *adj, vector<tuple<int, int, int>> &edges, int nodes)
{
    int u, v, w;
    while (cin >> u)
    {
        if (u != 0)
        {
            cin >> v >> w;
            addEdge(adj, u, v, w);
            tuple<int, int, int> t{w, u - 1, v - 1};
            edges.push_back(t);
        }
        else
            return;
    }
}

// ======================================================================================================= MAIN
int main()
{
    int nodes;
    int servers, cable_pairwise, cable_tree;

    // Adjacency list for weighted graph [u] -> {v, w}
    vector<pair<int, int>> *adj;
    // Edge list for weighted graph {w, u, v}
    vector<tuple<int, int, int>> edges;

    // Read input
    while (cin >> nodes)
    {
        adj = new vector<pair<int, int>>[nodes];
        edges.clear();
        servers = 0;
        cable_pairwise = cable_tree = 0;

        readInput(adj, edges, nodes);

        // Calculate servers and minimum amount of cable
        minCable(adj, edges, nodes, servers, cable_pairwise, cable_tree);

        if (servers == 0)
            printf("no server\n");

        else
            printf("%d %d %d\n", servers, cable_pairwise, cable_tree);
    }

    return (0);
}