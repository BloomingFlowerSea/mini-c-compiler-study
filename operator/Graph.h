
typedef struct GraphNode {
    int id; // Unique identifier for the node
    struct GraphNode** neighbors; // Array of pointers to neighboring nodes
    int neighbor_count; // Number of neighbors
    bool is_accepting; // Indicates if this node is an accepting state (for NFA)
} GraphNode;

