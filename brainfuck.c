/*
Copyright 2017 Victor C. Martins
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_CELL_SIZE 30000

char *pprogram;

void print_tab(int times, FILE *out)
{
    for (; times > 0; --times)
        fputs("\t", out);
}

int next_char()
{
    int c;
    while (1) {
        switch (*pprogram) {
        case 0:
            return 0;
        case '[':
        case ']':
        case '+':
        case '-':
        case '>':
        case '<':
        case '.':
        case ',':
            c = *pprogram;
            ++pprogram; 
            return c;
        default:
            ++pprogram;
            break;
        }
    }
}

void emit_opt_instruction(const char *base, const char *opt, 
                          int ind, int sum, FILE *out)
{
    print_tab(ind, out);
    if (sum > 2) {
        fprintf(out, opt, sum);       
    } else {
        fputs(base, out);
        if (sum == 2) {
            print_tab(ind, out);
            fputs(base, out);
        }
    }
}

void c_compile()
{
    int c;
    int sum = 0;
    int indent = 1;

    FILE *out = fopen("out.c", "w");

    fputs("#include <stdio.h>\n\n", out);
    fputs("int main()\n{\n", out);
    fprintf(out, "\tint cell[%d] = {0};\n", MAX_CELL_SIZE);
    fputs("\tint *pcell = cell;\n\n", out);
    
    while ((c = next_char()) != 0) {
        switch (c) {
        case '+':
            while (c == '+') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("++*pcell;\n", "*pcell += %d;\n", indent, sum, out);
            break;
        case '-':
            while (c == '-') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("--*pcell;\n", "*pcell -= %d;\n", indent, sum, out);
            break;
        case '>':
            while (c == '>') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("++pcell;\n", "pcell += %d;\n", indent, sum, out);
            break;
        case '<':
            while (c == '<') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("--pcell;\n", "pcell -= %d;\n", indent, sum, out);
            break;
        case '.':
            print_tab(indent, out);
            fputs("putchar(*pcell);\n", out);
            break;
        case ',':
            print_tab(indent, out);
            fputs("*pcell = getchar();\n", out);
            break;
        case '[':
            print_tab(indent++, out);
            fputs("while (*pcell) {\n", out);
            break;
        case ']':
            print_tab(--indent, out);
            fputs("}\n", out);
            break;
        default:
            break;
        }
        sum = 0;
    }
    
    fputs("\n\treturn 0;\n}\n", out);
    fclose(out);
}

void intel_compile()
{
    int stack[256];
    int c;
    int top = 0;
    int sum = 0;
    int loop = 0;

    FILE *out = fopen("out.asm", "w");

    fputs("SECTION .data\n", out);
    fprintf(out, "\tcell: times %d dd 0\n\n", MAX_CELL_SIZE);
    fputs("extern putchar\n", out);
    fputs("extern getchar\n\n", out);
    fputs("SECTION .text\n", out);
    fputs("global main\n", out);
    fputs("main:\n", out);
    fputs("\tpush ebp\n", out);    
    fputs("\tmov ebp, esp\n", out);
    fputs("\tmov ebx, cell\n", out);

    while ((c = next_char()) != 0) {
        switch (c) {
        case '+':
            while (c == '+') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("inc dword [ebx]\n", "add dword [ebx], %d\n", 1, sum, out);
            break;
        case '-':
            while (c == '-') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            emit_opt_instruction("dec dword [ebx]\n", "sub dword [ebx], %d\n", 1, sum, out);
            break;
        case '>':
            while (c == '>') {
                c = next_char();          
                sum += 4;
            }
            --pprogram;
            fprintf(out, "\tadd ebx, %d\n", sum);
            break;
        case '<':
            while (c == '<') {
                c = next_char();          
                sum += 4;
            }
            --pprogram;
            fprintf(out, "\tsub ebx, %d\n", sum);
            break;
        case '.':
            fputs("\tmov eax, dword [ebx]\n", out);
            fputs("\tmov dword [esp], eax\n", out);
            fputs("\tcall putchar\n", out);
            break;
        case ',':
            fputs("\tcall getchar\n", out);
            fputs("\tmov dword [ebx], eax\n", out);
            break;
        case '[':
            stack[top++] = loop;
            fprintf(out, ".L%d:\n", loop);
            fputs("\tcmp dword [ebx], 0\n", out);
            fprintf(out, "\tje .LE%d\n", loop++);
            break;
        case ']':
            fprintf(out, "\tjmp .L%d\n", stack[--top]);
            fprintf(out, ".LE%d:\n", stack[top]);
            break;
        default:
            break;
        }
        sum = 0;
    }

    fputs("\tleave\n", out);
    fputs("\tret", out);
    fclose(out);
}

void mips_compile()
{
    int stack[256];
    int c;
    int top = 0;
    int sum = 0;
    int loop = 0;

    FILE *out = fopen("out.asm", "w");

    fputs(".data\n", out);
    fprintf(out, "\tcell: .word 0:%d\n", MAX_CELL_SIZE);
    fputs(".text\n", out);
    fputs("main:\n", out);
    fputs("\tla $a1, cell\n", out);

    while ((c = next_char()) != 0) {
        switch (c) {
        case '+':
            while (c == '+') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            fputs("\tlw $t0, ($a1)\n", out);
            fprintf(out, "\taddi $t0, $t0, %d\n", sum);
            fputs("\tsw $t0, ($a1)\n", out);
            break;
        case '-':
            while (c == '-') {
                c = next_char();          
                ++sum;
            }
            --pprogram;
            fputs("\tlw $t0, ($a1)\n", out);
            fprintf(out, "\taddiu $t0, $t0, -%d\n", sum);
            fputs("\tsw $t0, ($a1)\n", out);
            break;
        case '>':
            while (c == '>') {
                c = next_char();          
                sum += 4;
            }
            --pprogram;
            fprintf(out, "\taddi $a1, $a1, %d\n", sum);
            break;
        case '<':
            while (c == '<') {
                c = next_char();          
                sum += 4;
            }
            --pprogram;
            fprintf(out, "\taddiu $a1, $a1, -%d\n", sum);
            break;
        case '.':
            fputs("\tli $v0, 11\n", out);
            fputs("\tlw $a0, ($a1)\n", out);
            fputs("\tsyscall\n", out);
            break;
        case ',':
            fputs("\tli $v0, 12\n", out);
            fputs("\tsyscall\n", out);
            fputs("\tsw $v0, ($a1)\n", out);
            break;
        case '[':
            stack[top++] = loop;
            fprintf(out, "L%d:\n", loop);
            fputs("\tlw $t0, ($a1)\n", out);
            fprintf(out, "\tbeq $t0, $zero, LE%d\n", loop++);
            break;
        case ']':
            fprintf(out, "\tj L%d\n", stack[--top]);
            fprintf(out, "LE%d:\n", stack[top]);
            break;
        default:
            break;
        }
        sum = 0;
    }

    fputs("\tli $v0, 10\n", out);
    fputs("\tsyscall\n\n", out);
    fclose(out);
}


int check_syntax(char *program)
{
    int openp = 0, closep = 0;    
    
    while (*program != 0) {
        if (*program == '[')
            ++openp;
        else if (*program == ']')
            ++closep;
        ++program;
    }
    
    if (openp != closep) {
        fputs("syntax error: unmatched parenthesis\n", stderr);
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[])
{
    FILE *in;
    char *program;
    int file_len;

    if (argc < 2) {
        fputs("error: no input file\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (!(in = fopen(argv[1], "rb"))) {
        if (errno == ENOENT) {
            fputs("error: file not found\n", stderr);
            exit(EXIT_FAILURE);        
        }
    }

    fseek(in, 0, SEEK_END);
    file_len = ftell(in);
    fseek(in, 0, SEEK_SET);

    pprogram = program = (char *) malloc(sizeof(char) * (file_len+1));
    fread(program, sizeof(char), file_len, in);
    program[file_len] = 0;
    fclose(in);
    
    if (check_syntax(program)) {
        if (argc >= 3) {
            if (strcmp(argv[2], "-c") == 0)
                c_compile();
            else if (strcmp(argv[2], "-i386") == 0)
                intel_compile();
            else if (strcmp(argv[2], "-mips") == 0)
                mips_compile();
            else
                fputs("error: invalid target\n", stderr);
        } else {
            c_compile();
        }
    }

    free(program);    
    return 0;
}
