#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Node creation functions */
Node* create_object_node(Pair** pairs, int pair_count) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_OBJECT;
    
    if (pairs && pair_count > 0) {
        node->data.object.pairs = pairs;
        node->data.object.pair_count = pair_count;
    } else {
        node->data.object.pairs = NULL;
        node->data.object.pair_count = 0;
    }
    
    return node;
}

Node* create_array_node(Node** elements, int element_count) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_ARRAY;
    
    if (elements && element_count > 0) {
        node->data.array.elements = elements;
        node->data.array.element_count = element_count;
    } else {
        node->data.array.elements = NULL;
        node->data.array.element_count = 0;
    }
    
    return node;
}

Node* create_string_node(const char* value) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_STRING;
    node->data.string_value = strdup(value);
    return node;
}

Node* create_number_node(double value) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_NUMBER;
    node->data.number_value = value;
    return node;
}

Node* create_boolean_node(int value) {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_BOOLEAN;
    node->data.boolean_value = value;
    return node;
}

Node* create_null_node() {
    Node* node = malloc(sizeof(Node));
    node->type = NODE_NULL;
    return node;
}

Pair* create_pair_node(const char* key, Node* value) {
    Pair* pair = malloc(sizeof(Pair));
    pair->key = strdup(key);
    pair->value = value;
    return pair;
}

/* Node manipulation */
void add_pair_to_object(Node* object, Pair* pair) {
    if (object->type != NODE_OBJECT) {
        fprintf(stderr, "Error: Cannot add pair to non-object node\n");
        exit(1);
    }
    
    object->data.object.pair_count++;
    object->data.object.pairs = realloc(
        object->data.object.pairs, 
        object->data.object.pair_count * sizeof(Pair*)
    );
    object->data.object.pairs[object->data.object.pair_count - 1] = pair;
}

void add_element_to_array(Node* array, Node* element) {
    if (array->type != NODE_ARRAY) {
        fprintf(stderr, "Error: Cannot add element to non-array node\n");
        exit(1);
    }
    
    array->data.array.element_count++;
    array->data.array.elements = realloc(
        array->data.array.elements, 
        array->data.array.element_count * sizeof(Node*)
    );
    array->data.array.elements[array->data.array.element_count - 1] = element;
}

/* AST printing */
void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void print_ast(Node* node, int indent) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_OBJECT:
            printf("{\n");
            for (int i = 0; i < node->data.object.pair_count; i++) {
                Pair* pair = node->data.object.pairs[i];
                print_indent(indent + 1);
                printf("\"%s\": ", pair->key);
                print_ast(pair->value, indent + 1);
                if (i < node->data.object.pair_count - 1) {
                    printf(",");
                }
                printf("\n");
            }
            print_indent(indent);
            printf("}");
            break;
            
        case NODE_ARRAY:
            printf("[\n");
            for (int i = 0; i < node->data.array.element_count; i++) {
                print_indent(indent + 1);
                print_ast(node->data.array.elements[i], indent + 1);
                if (i < node->data.array.element_count - 1) {
                    printf(",");
                }
                printf("\n");
            }
            print_indent(indent);
            printf("]");
            break;
            
        case NODE_STRING:
            printf("\"%s\"", node->data.string_value);
            break;
            
        case NODE_NUMBER:
            printf("%g", node->data.number_value);
            break;
            
        case NODE_BOOLEAN:
            printf("%s", node->data.boolean_value ? "true" : "false");
            break;
            
        case NODE_NULL:
            printf("null");
            break;
    }
}

/* AST cleanup */
void free_ast(Node* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_OBJECT:
            for (int i = 0; i < node->data.object.pair_count; i++) {
                Pair* pair = node->data.object.pairs[i];
                free(pair->key);
                free_ast(pair->value);
                free(pair);
            }
            free(node->data.object.pairs);
            break;
            
        case NODE_ARRAY:
            for (int i = 0; i < node->data.array.element_count; i++) {
                free_ast(node->data.array.elements[i]);
            }
            free(node->data.array.elements);
            break;
            
        case NODE_STRING:
            free(node->data.string_value);
            break;
            
        default:
            /* Nothing to free for other node types */
            break;
    }
    
    free(node);
}

/* Helper function to detect object structure */
typedef struct KeySet {
    char** keys;
    int key_count;
    char* table_name;
    struct KeySet* next;
} KeySet;

/* Check if two objects have identical key sets */
int compare_key_sets(Node* obj1, Node* obj2) {
    if (obj1->type != NODE_OBJECT || obj2->type != NODE_OBJECT) {
        return 0;
    }
    
    if (obj1->data.object.pair_count != obj2->data.object.pair_count) {
        return 0;
    }
    
    /* Check if every key in obj1 is in obj2 */
    for (int i = 0; i < obj1->data.object.pair_count; i++) {
        char* key1 = obj1->data.object.pairs[i]->key;
        int found = 0;
        
        for (int j = 0; j < obj2->data.object.pair_count; j++) {
            char* key2 = obj2->data.object.pairs[j]->key;
            if (strcmp(key1, key2) == 0) {
                found = 1;
                break;
            }
        }
        
        if (!found) {
            return 0;
        }
    }
    
    return 1;
}

/* Find object structure in key set list */
KeySet* find_key_set(KeySet* list, Node* obj) {
    KeySet* current = list;
    while (current) {
        int match = 1;
        
        /* Check if object has all keys in key set */
        if (obj->data.object.pair_count == current->key_count) {
            for (int i = 0; i < obj->data.object.pair_count; i++) {
                char* key = obj->data.object.pairs[i]->key;
                int found = 0;
                
                for (int j = 0; j < current->key_count; j++) {
                    if (strcmp(key, current->keys[j]) == 0) {
                        found = 1;
                        break;
                    }
                }
                
                if (!found) {
                    match = 0;
                    break;
                }
            }
            
            if (match) {
                return current;
            }
        }
        
        current = current->next;
    }
    
    return NULL;
}

/* Create a new key set from object */
KeySet* create_key_set(Node* obj, const char* name_hint) {
    KeySet* key_set = malloc(sizeof(KeySet));
    key_set->key_count = obj->data.object.pair_count;
    key_set->keys = malloc(key_set->key_count * sizeof(char*));
    key_set->table_name = strdup(name_hint ? name_hint : "table");
    key_set->next = NULL;
    
    for (int i = 0; i < obj->data.object.pair_count; i++) {
        key_set->keys[i] = strdup(obj->data.object.pairs[i]->key);
    }
    
    return key_set;
}

/* Free a key set */
void free_key_set(KeySet* key_set) {
    for (int i = 0; i < key_set->key_count; i++) {
        free(key_set->keys[i]);
    }
    free(key_set->keys);
    free(key_set->table_name);
    free(key_set);
}

/* First pass: collect all key sets (table structures) */
KeySet* collect_key_sets(Node* node, KeySet* key_sets, const char* name_hint) {
    if (!node) return key_sets;
    
    if (node->type == NODE_OBJECT) {
        /* Process each field in the object */
        for (int i = 0; i < node->data.object.pair_count; i++) {
            Pair* pair = node->data.object.pairs[i];
            Node* value = pair->value;
            
            /* If the value is an array of objects, process it as a table */
            if (value->type == NODE_ARRAY && value->data.array.element_count > 0) {
                Node* first = value->data.array.elements[0];
                if (first->type == NODE_OBJECT) {
                    /* Use the field name as the table name */
                    key_sets = collect_key_sets(first, key_sets, pair->key);
                }
            }
            /* If the value is an object, process it recursively */
            else if (value->type == NODE_OBJECT) {
                key_sets = collect_key_sets(value, key_sets, pair->key);
            }
        }
    }
    
    return key_sets;
}

/* Create table from key set */
Table* create_table_from_key_set(KeySet* key_set) {
    Table* table = malloc(sizeof(Table));
    table->name = strdup(key_set->table_name);
    
    /* Start with ID column, then all keys excluding objects and arrays */
    /* We'll handle those separately */
    table->column_count = 1;  /* Start with id column */
    table->columns = malloc(sizeof(char*) * (key_set->key_count + 1)); /* +1 for id */
    table->columns[0] = strdup("id");
    
    /* Add all scalar keys */
    int col_idx = 1;
    for (int i = 0; i < key_set->key_count; i++) {
        table->columns[col_idx++] = strdup(key_set->keys[i]);
    }
    
    table->column_count = col_idx;
    table->next = NULL;
    
    return table;
}

/* Analyze AST and generate schema */
Schema* analyze_ast(Node* root) {
    if (!root) return NULL;
    
    Schema* schema = malloc(sizeof(Schema));
    if (!schema) return NULL;
    
    schema->tables = NULL;
    schema->table_count = 0;
    
    /* Process root object */
    if (root->type == NODE_OBJECT) {
        for (int i = 0; i < root->data.object.pair_count; i++) {
            Pair* pair = root->data.object.pairs[i];
            Node* value = pair->value;
            
            /* If the value is an array of objects, process it as a table */
            if (value->type == NODE_ARRAY && value->data.array.element_count > 0) {
                Node* first = value->data.array.elements[0];
                if (first->type == NODE_OBJECT) {
                    Table* table = malloc(sizeof(Table));
                    if (!table) continue;
                    table->name = strdup(pair->key);
                    table->next = schema->tables;
                    schema->tables = table;
                    schema->table_count++;
                    table->column_count = first->data.object.pair_count;
                    table->columns = malloc(sizeof(char*) * table->column_count);
                    for (int j = 0; j < first->data.object.pair_count; j++) {
                        Pair* field = first->data.object.pairs[j];
                        table->columns[j] = strdup(field->key);
                    }
                }
            }
            /* If the value is an object, process it as a separate table */
            else if (value->type == NODE_OBJECT) {
                Table* table = malloc(sizeof(Table));
                if (!table) continue;
                table->name = strdup(pair->key);
                table->next = schema->tables;
                schema->tables = table;
                schema->table_count++;
                table->column_count = value->data.object.pair_count;
                table->columns = malloc(sizeof(char*) * table->column_count);
                for (int j = 0; j < value->data.object.pair_count; j++) {
                    Pair* field = value->data.object.pairs[j];
                    table->columns[j] = strdup(field->key);
                }
            }
        }
    }
    return schema;
}

void free_schema(Schema* schema) {
    Table* current = schema->tables;
    while (current) {
        Table* next = current->next;
        
        for (int i = 0; i < current->column_count; i++) {
            free(current->columns[i]);
        }
        free(current->columns);
        free(current->name);
        free(current);
        
        current = next;
    }
    
    free(schema);
}