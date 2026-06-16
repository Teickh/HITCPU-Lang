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

    snprintf(file_path, sizeof(file_path), "%s.txt", program_name);

    FILE *program = fopen(file_path, "r");

    if (program == NULL) {
        fprintf(stderr, "Attempted to open: %s\n", file_path);
        perror("Error opening file");
        return NULL;
    }
    
    printf("File %s open successfully!\n", file_path);
    return program;
}

FILE *create_assembly_file(const char *program_name) {
    char file_path[256];

    snprintf(file_path, sizeof(file_path), "%s.as", program_name);

    FILE *program_assem = fopen(file_path, "wb");

    fprintf(stderr, "Creating file...\n");

    if (program_assem == NULL) {
        fprintf(stderr, "Attempted to create: %s\n", file_path);
        perror("Error creating file");
        return NULL;
    }
    
    printf("File %s created successfully!\n", file_path);
    return program_assem;
}

int main(int argc, char * argv[]) {
    if (argc < 2) {
        printf("Usage: lang.exe <files...>\n");
        return 1;
    }

    int files_capacity = argc - 1;
    ProgramList programs = {
        .count = 0,
        .capacity = files_capacity,
        .files = malloc(files_capacity * sizeof(FileRegistry)),
        .main_file_index = -1,
        .main_function_index = -1
    };

    for (int i = 1; i < argc; i++) {
        programs.files[programs.count].filename = argv[i];
        programs.files[programs.count].main_function_node_index = -1;
        programs.files[programs.count].global_functions = malloc(8 * sizeof(int));
        programs.files[programs.count].function_count = 0;
        programs.files[programs.count].function_capacity = 8;
        programs.files[programs.count].global_variables = malloc(8 * sizeof(int));
        programs.files[programs.count].global_var_count = 0;
        programs.files[programs.count].global_var_capacity = 8;

        programs.count++;
    }

    StringPool pool = { .data = malloc(1024), .size = 0, .capacity = 1024 };
    ScopeStack current_scope = { 
        .scope_capacity = 8,
        .scopes = malloc(8 * sizeof(SymbolTable)),
        .stack = -1
    };
    SymbolLists symbol_lists = {
        .capacity = 8,
        .count = 0,
        .historical_scopes = malloc(8 * sizeof(SymbolTable))
    };
    for (int i = 0; i < programs.count; i++) {
        FILE *program = open_file(argv[i + 1]);

        if (program == NULL) {
            return 1;
        }

        TokenList list = { .items = malloc(8 * sizeof(Token)), .count = 0, .capacity = 8 };
        int total_lines = 0;

        lexing(program, &list, &pool, &total_lines);
        print_tokens_to_html(&list, &pool, "tokens.html");

        ASTTree tree = { .capacity = 8, .count = 0, .nodes = malloc(8 * sizeof(ASTNode)) };
        parsing(&list, &programs.files[i], &pool, &tree);

        free(list.items);

        analyse(&programs.files[i], &tree, &current_scope, &symbol_lists, "semantic_analysis.html", &pool);

        InstructionStream stream = {
            .capacity = 8,
            .count = 0,
            .data = malloc(8 * sizeof(AsmInstruction))
        };
        generate_code(&programs.files[i], &tree, &pool, &stream);

        print_string_pool_to_html(&pool, "string_pool.html");
        generate_ast_html("ast.html", &programs.files[i], &tree, &pool);
        dump_stream_to_html("Instruction_stream.html", &stream, &symbol_lists, &pool);

        fclose(program);
        free(pool.data);
        free(tree.nodes);
    }

    return 0;
}