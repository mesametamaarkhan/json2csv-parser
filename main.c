#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "csv_generator.h"

/* External variables from parser */
extern Node* root;
extern FILE* yyin;
extern int yyparse();

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input.json>\n", argv[0]);
        return 1;
    }

    printf("Opening input file: %s\n", argv[1]);

    /* Open input file */
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        fprintf(stderr, "Error: Could not open input file '%s'\n", argv[1]);
        return 1;
    }

    printf("Parsing JSON...\n");

    /* Parse JSON */
    if (yyparse() != 0) {
        fprintf(stderr, "Error: Failed to parse JSON\n");
        fclose(yyin);
        return 1;
    }
    fclose(yyin);

    if (!root) {
        fprintf(stderr, "Error: No valid JSON data found\n");
        return 1;
    }

    printf("JSON parsed successfully. Analyzing AST...\n");

    /* Analyze AST to generate schema */
    Schema* schema = analyze_ast(root);
    if (!schema) {
        fprintf(stderr, "Error: Failed to analyze AST\n");
        free_ast(root);
        return 1;
    }

    printf("AST analyzed. Schema created with %d tables.\n", schema->table_count);

    /* Initialize CSV context */
    printf("Initializing CSV context...\n");
    CSVContext* context = init_csv_context("output");
    if (!context) {
        fprintf(stderr, "Error: Failed to initialize CSV context\n");
        free_schema(schema);
        free_ast(root);
        return 1;
    }

    printf("Generating CSV files...\n");

    /* Generate CSV files */
    generate_csv(root, schema, context);

    printf("CSV generation complete.\n");

    /* Cleanup */
    free_csv_context(context);
    free_schema(schema);
    free_ast(root);

    return 0;
} 