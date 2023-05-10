#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAXBUFSIZE 1000000

struct vm {
    uint8_t *cells;
    size_t n_cells;
    size_t pc;
    size_t ap;
};

struct vm vm_create(size_t n_cells)
{
    return (struct vm) {
        .n_cells = n_cells,
        .cells = calloc(n_cells, sizeof(uint8_t)),
        .pc = 0,
        .ap = 0
    };
}

void vm_destroy(struct vm *vm)
{
    free(vm->cells);
    vm->cells = NULL;
}

void vm_execute(struct vm *vm, char *program)
{
    while (program[vm->pc]) {
        switch(program[vm->pc]) {
        case '+': vm->cells[vm->ap]++; break;
        case '-': vm->cells[vm->ap]--; break;
        case '.': putchar(vm->cells[vm->ap]); break;
        case ',': vm->cells[vm->ap] = getchar(); break;
        case '>':
            vm->ap++;
            if (vm->ap >= vm->n_cells)
                vm->ap -= vm->n_cells;
            break;
        case '<':
            if (vm->ap == 0)
                vm->ap = vm->n_cells - 1;
            else
                vm->ap--;
            break;
        case '[': {
            if (vm->cells[vm->ap] != 0)
                break;

            int q = 1;
            for (;;) {
                vm->pc++;
                if (program[vm->pc] == ']') q--;
                if (program[vm->pc] == '[') q++;
                if (q == 0) break;
                if (program[vm->pc] == '\0') {
                    fprintf(stderr, "Error: No matching ']'.\n");
                    exit(1);
                }
            }

            break;
        }
        case ']': {
            if (vm->cells[vm->ap] == 0)
                break;

            int q = 1;
            for (;;) {
                if (vm->pc == 0) {
                    fprintf(stderr, "Error: no matching '['\n");
                    exit(1);
                }
                vm->pc--;
                if (program[vm->pc] == '[') q--;
                if (program[vm->pc] == ']') q++;
                if (q == 0) break;
            }

            break;
        }
        }

        vm->pc++;
    }
}

void test()
{
    {
        struct vm vm = vm_create(10000);
        vm_execute(&vm, "+");
        assert(vm.cells[0] == 1);
        vm_destroy(&vm);
    }
    {
        struct vm vm = vm_create(10000);
        vm_execute(&vm, "-");
        assert(vm.cells[0] == 255);
        vm_destroy(&vm);
    }
    {
        struct vm vm = vm_create(10000);
        vm_execute(&vm, ">>");
        assert(vm.ap == 2);
        vm_destroy(&vm);
    }
    {
        struct vm vm = vm_create(100);
        vm_execute(&vm, "<<");
        /* We expect the address pointer to wrap, but this behaviour should not
         * be relied upon */
        assert(vm.ap == 98);
        vm_destroy(&vm);
    }
    {
        struct vm vm = vm_create(1000);
        vm_execute(&vm, "[]+");
        assert(vm.ap == 0);
        assert(vm.cells[0] == 1);
        vm_destroy(&vm);
    }

    printf("Tests passed (lol..)\n");
}

int main(int argc, const char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: cbrainfuck [file]\n");
        exit(1);
    }

    FILE *program = fopen(argv[1], "r");
    if (program == NULL) {
        fprintf(stderr, "Error with reading file :(\n");
        exit(1);
    }

    char buf[MAXBUFSIZE + 1];
    fread(buf, 1, MAXBUFSIZE, program);

    struct vm vm = vm_create(100000);
    vm_execute(&vm, buf);
    vm_destroy(&vm);

    fclose(program);
}
