#include <stdio.h>
#include <string.h>

// Global structures to hold TAC data
char tac[100][100];
int tac_index = 0;
int temp_var = 1;
int current_line = 100;

// Function to add a formatted string to our TAC instruction array
void add_tac(const char* instruction) {
    sprintf(tac[tac_index++], "%d: %s", current_line++, instruction);
}

// Simple parser for assignments (requires spaces around operators)
void process_assignment(const char* assignment) {
    char lhs[20], rhs1[20], rhs2[20], op;
    char buffer[100];

    // Attempt to parse a complex assignment: "x = y + z"
    // Note: The space before %c tells sscanf to ignore leading whitespace
    if (sscanf(assignment, "%s = %s %c %s", lhs, rhs1, &op, rhs2) == 4) {
        sprintf(buffer, "t%d = %s %c %s", temp_var, rhs1, op, rhs2);
        add_tac(buffer);
        
        sprintf(buffer, "%s = t%d", lhs, temp_var);
        add_tac(buffer);
        
        temp_var++;
    }
    // Attempt to parse a simple assignment: "x = y"
    else if (sscanf(assignment, "%s = %s", lhs, rhs1) == 2) {
        sprintf(buffer, "%s = %s", lhs, rhs1);
        add_tac(buffer);
    } else {
        sprintf(buffer, "Unrecognized statement: %s", assignment);
        add_tac(buffer);
    }
}

int main() {
    char condition[50], true_block[50], false_block[50];
    char buffer[100];

    printf("--- TAC Generator for If-Else Construct ---\n");
    printf("Note: Ensure spaces between operands and operators (e.g., x = y + z)\n\n");

    // 1. Gather Inputs
    printf("Enter the condition (e.g., a < b): ");
    fgets(condition, sizeof(condition), stdin);
    condition[strcspn(condition, "\n")] = 0; // Strip newline

    printf("Enter the statement if TRUE (e.g., x = y + z): ");
    fgets(true_block, sizeof(true_block), stdin);
    true_block[strcspn(true_block, "\n")] = 0;

    printf("Enter the statement if FALSE (e.g., x = y - z): ");
    fgets(false_block, sizeof(false_block), stdin);
    false_block[strcspn(false_block, "\n")] = 0;

    // 2. Process Control Flow and Generate TAC
    
    // Evaluate condition and jump to the TRUE block
    int condition_line = current_line;
    sprintf(buffer, "if (%s) goto %d", condition, condition_line + 2);
    add_tac(buffer);

    // Reserve a line for the FALSE block jump (Backpatching)
    int jump_to_false_index = tac_index; 
    current_line++; 
    tac_index++;

    // Process the TRUE block
    process_assignment(true_block);

    // Reserve a line to skip the FALSE block after TRUE block finishes
    int jump_to_end_index = tac_index; 
    int jump_to_end_line = current_line;
    current_line++; 
    tac_index++;

    // Mark where the FALSE block actually starts
    int false_block_line = current_line;

    // Process the FALSE block
    process_assignment(false_block);

    // Mark the end of the construct
    int end_line = current_line;

    // 3. Resolve the Jumps (Backpatching)
    sprintf(tac[jump_to_false_index], "%d: goto %d", condition_line + 1, false_block_line);
    sprintf(tac[jump_to_end_index], "%d: goto %d", jump_to_end_line, end_line);

    // 4. Output the Final Code
    printf("\n--- Generated Three Address Code ---\n");
    for (int i = 0; i < tac_index; i++) {
        printf("%s\n", tac[i]);
    }

    return 0;
}
