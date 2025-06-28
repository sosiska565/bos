#include "string/string.h"
#include "vga/vga.h"

int color = 0x07;
int bgcolor = 0x0;
int text_color;

void init_shell(){
    char input_buffer[100];
    while(1) {
        text_color = color & 0x0F;
        color = (bgcolor << 4) | text_color;

        printString("> ", color);
        readString(input_buffer, sizeof(input_buffer), color);

        if(strcmp(input_buffer, "help") == 0){
            printString("--Available commands--\n", color);
            printString("help - displays all available commands on the screen.\n", color);
            printString("clear - clear screen.\n", color);
            printString("add - adding two numbers, required arguments: <int number>, <int number>.\n", color);
            printString("sub - subtraction two numbers, required arguments: <int number>, <int number>.\n", color);
            printString("mul - multiplication two numbers, required arguments: <int number>, <int number>.\n", color);
            printString("div - division two numbers, required arguments: <int number>, <int number>.\n", color);
            printString("bgcolor - change background color, required arguments: <string color>.\n", color);
        }
        else if(strcmp(input_buffer, "clear") == 0){
            clear(color);
        }
        else if (strncmp(input_buffer, "add ", 4) == 0){
            char* args_start = input_buffer + 4;
            char* comma_ptr = 0;
            
            for (int i = 0; args_start[i] != '\0'; i++) {
                if (args_start[i] == ',') {
                    comma_ptr = &args_start[i];
                    break;
                }
            }
            
            if (comma_ptr) {
                *comma_ptr = '\0';
                
                char* num1_str = args_start;
                char* num2_str = comma_ptr + 1;
                
                int num1 = atoi(num1_str);
                int num2 = atoi(num2_str);
                
                int sum = num1 + num2;
                
                char result_buffer[20];
                itoa(sum, result_buffer, 10);
                printString(result_buffer, color);
            } else {
                printString("ERR: Incorrect syntax. Use: add <number1>, <number2>", color);
            }
        }
        else if (strncmp(input_buffer, "sub ", 4) == 0){
            char* args_start = input_buffer + 4;
            char* comma_ptr = 0;
            
            for (int i = 0; args_start[i] != '\0'; i++) {
                if (args_start[i] == ',') {
                    comma_ptr = &args_start[i];
                    break;
                }
            }
            
            if (comma_ptr) {
                *comma_ptr = '\0';
                
                char* num1_str = args_start;
                char* num2_str = comma_ptr + 1;
                
                int num1 = atoi(num1_str);
                int num2 = atoi(num2_str);
                
                int sum = num1 - num2;
                
                char result_buffer[20];
                itoa(sum, result_buffer, 10);
                printString(result_buffer, color);
            } else {
                printString("ERR: Incorrect syntax. Use: sub <number1>, <number2>", color);
            }
        }
        else if (strncmp(input_buffer, "mul ", 4) == 0){
            char* args_start = input_buffer + 4;
            char* comma_ptr = 0;
            
            for (int i = 0; args_start[i] != '\0'; i++) {
                if (args_start[i] == ',') {
                    comma_ptr = &args_start[i];
                    break;
                }
            }
            
            if (comma_ptr) {
                *comma_ptr = '\0';
                
                char* num1_str = args_start;
                char* num2_str = comma_ptr + 1;
                
                int num1 = atoi(num1_str);
                int num2 = atoi(num2_str);
                
                int sum = num1 * num2;
                
                char result_buffer[20];
                itoa(sum, result_buffer, 10);
                printString(result_buffer, color);
            } else {
                printString("ERR: Incorrect syntax. Use: mul <number1>, <number2>", color);
            }
        }
        else if (strncmp(input_buffer, "div ", 4) == 0){
            char* args_start = input_buffer + 4;
            char* comma_ptr = 0;
            
            for (int i = 0; args_start[i] != '\0'; i++) {
                if (args_start[i] == ',') {
                    comma_ptr = &args_start[i];
                    break;
                }
            }
            
            if (comma_ptr) {
                *comma_ptr = '\0';
                
                char* num1_str = args_start;
                char* num2_str = comma_ptr + 1;
                
                int num1 = atoi(num1_str);
                int num2 = atoi(num2_str);
                if (num2 == 0)
                {
                    printString("EXC: division by zero", color);
                }
                else{
                    int sum = num1 / num2;
                
                    char result_buffer[20];
                    itoa(sum, result_buffer, 10);
                    printString(result_buffer, color);
                }
            } else {
                printString("ERR: Incorrect syntax. Use: div <number1>, <number2>", color);
            }
        }
        else if(strncmp(input_buffer, "bgcolor ", 8) == 0){
            char *colorArg = input_buffer + 8;

            if(strcmp(colorArg, "black") == 0){
                bgcolor = 0;
                fill_background(0x0);
            }
            else if(strcmp(colorArg, "red") == 0){
                bgcolor = 4;
                fill_background(0x04);
            }
            else if(strcmp(colorArg, "green") == 0){
                bgcolor = 2;
                fill_background(0x02);
            }
            else if(strcmp(colorArg, "blue") == 0){
                bgcolor = 1;
                fill_background(0x01);
            }
            else{
                printString("ERR: Incorrect syntax. Use: bgcolor <color>", color);
            }
        }
        else{
            if (input_buffer[0] != '\0') {
                printString("ERR: Unknown command: ", color);
                printString(input_buffer, color);
            }
        }
        
        printString("\n", color);
    }
}