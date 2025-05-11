#ifndef AST_H
#define AST_H

typedef enum {
    NODE_OBJECT,
    NODE_ARRAY,
    NODE_STRING,
    NODE_NUMBER,
    NODE_BOOLEAN,
    NODE_NULL
} NodeType;

typedef struct Pair {
    char* key;
    struct Node* value;
} Pair;

typedef struct Node {
    NodeType type;
    int line;
    int column;
    
    union {
        struct {
            Pair** pairs;
            int pair_count;
        } object;
        
        struct {
            struct Node** elements;
            int element_count;
        } array;
        
        char* string_value;
        double number_value;
        int boolean_value;
    } data;
} Node;

/* Node creation functions */
Node* create_object_node(Pair** pairs, int pair_count);
Node* create_array_node(Node** elements, int element_count);
Node* create_string_node(const char* value);
Node* create_number_node(double value);
Node* create_boolean_node(int value);
Node* create_null_node();
Pair* create_pair_node(const char* key, Node* value);

/* Node manipulation */
void add_pair_to_object(Node* object, Pair* pair);
void add_element_to_array(Node* array, Node* element);

/* AST operations */
void print_ast(Node* node, int indent);
void free_ast(Node* node);

/* Table structure definition */
typedef struct Table {
    char* name;
    char** columns;
    int column_count;
    struct Table* next;
} Table;

typedef struct Schema {
    Table* tables;
    int table_count;
} Schema;

/* AST analysis for CSV generation */
Schema* analyze_ast(Node* root);
void free_schema(Schema* schema);

#endif /* AST_H */