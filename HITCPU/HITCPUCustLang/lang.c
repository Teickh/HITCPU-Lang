#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "parser.h"
#include "semantic_analysis.h"
#include "code_gen.h"
#include "struct.h"

FILE *open_file(const char *program_name) {
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "programs/%s.txt", program_name);

    FILE *program = fopen(file_path, "r");

    if (program == NULL) {
        fprintf(stderr, "Attempted to open: %s\n", file_path);
        perror("Error opening file");
        return NULL;
    }
    
    printf("File %s open successfully!", file_path);
    return program;
}

FILE *create_assembly_file(const char *program_name) {
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "../HITCPUAssem/programs/%s.as", program_name);

    FILE *program_assem = fopen(file_path, "wb");

    fprintf(stderr, "\nCreating file...");

    if (program_assem == NULL) {
        fprintf(stderr, "Attempted to create: %s\n", file_path);
        perror("Error creating file");
        return NULL;
    }
    
    printf("File %s created successfully!\n", file_path);
    return program_assem;
}

int main() {
    const char *program_name = "TEST";

    FILE *program = open_file(program_name);

    if (program == NULL) {
        return 1;
    }

    TokenList list = { .items = NULL, .count = 0, .capacity = 0 };
    StringPool pool = { .data = NULL, .size = 0, .capacity = 0 };
    ASTTree tree = { .capacity = 0, .count = 0, .nodes = NULL };
    ProgramRegistry registry = {
        .function_capacity = 0,
        .function_count = 0,
        .global_functions = NULL,
        .global_var_capacity = 0,
        .global_var_count = 0,
        .global_variables = NULL,
        .main_function_node_index = -1
    };
    Scope current_scope = {0};
    FILE *program_assembly = create_assembly_file(program_name);
    int total_lines = 0;
    int program_root = 0;

    lexing(program, &list, &pool, &total_lines);
    print_tokens(&list, &pool); 

    parsing(&list, &registry, &pool, &tree, &program_root);
    printf("\n--- AST Structure ---\n");
    print_ast(&registry, &tree, &pool, 0);

    free(list.items);

    analyse(&registry, &tree, &current_scope);

    // int node_count = 0;
    // generate_code(ast, program_assembly, &regs);
    // printf("\n=== [DEBUG: AST TRAVERSAL] ===\n");
    // debug_print_ast(ast, node_count);

    fclose(program);
    fclose(program_assembly);
    free(pool.data);
    // free_ast(tree);
    return 0;
}