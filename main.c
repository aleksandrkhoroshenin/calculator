/*
 * Разработайте программу-калькулятор, умеющую вычислять арифметические выражения над множествами строк.
 * Входные данные (выражение с описанием множеств и производимых над ними операций) подаются на стандартный поток ввода программы,
 * результат вычислений должен подаваться на стандартный поток вывода.
 * Поддерживаемые операции: 'U' - объединение, '^' - пересечение, '\' - разность множеств,
 * '()' - задание приоритета вычислений.Множества задаются в виде [el1,el2,...],
 * где элементами являются строки в формате "abc def" (могут содержать пробелы).
 * Множества могут быть пустыми. Между операциями, множествами и элементами множеств пробелы запрещены.
 * При вычислениях должны учитываться приоритеты операций (в том числе, заданные вручную при помощи круглых скобочек).
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t BUFF_SIZE = 10;
const size_t REALLOC_COEFFICIENT = 2;

#define START_SET_CH '['
#define END_SET_CH ']'
#define BORDER_ELEM_CH '"'
#define ELEM_SEPARATOR ','
#define UNION 'U'
#define INTERSECTION '^'
#define COMPLEMENT '\\' // спец символ
#define OPEN_BRACKET '('
#define CLOSE_BRACKET ')'
#define SET 's'

//==================Set======================

//объект множества принимает во владение объекты строк, которые являются елементами множества
//перед добавлением строки выделяются динамически
//за освобождением памяти, занимаемой этими строками, следит множество

struct SetNode {
    char* str;
    struct SetNode* next;
    struct SetNode* prev;
};

struct Set {
    struct SetNode* head;
};

void initSet(struct Set* set);
void deleteSet(struct Set* set);
int addSet(struct Set* set, char *data);
void outputSet(struct Set set);
int unionSet(struct Set a, struct Set b, struct Set *res);
int intersectionSet(struct Set a, struct Set b, struct Set *res);
int complementSet(struct Set a, struct Set b, struct Set *res);

//==================Set======================

//==================Lexem====================

struct Lexem {
    char type;
    struct Set set;
};

//==================Lexem====================

//==================Stack====================

//хранит структуры лексем

struct StackNode {
    struct Lexem lex;
    struct StackNode* next;
};

struct Stack {
    struct StackNode* top;
};

void deleteStack(struct Stack* stack);
int push(struct Stack* stack, struct Lexem val);
struct Lexem pop(struct Stack* stack);
struct Lexem top(struct Stack* stack);
int emptyStack(struct Stack* stack);

//==================Stack====================

//==================DynamicArray=============

//хранит структуры лексем
//используется для хранения ОПЗ
//следит за удалением множеств входящих в состав лексем

struct DynamicArray {
    struct Lexem* data;
    size_t size;
    size_t capacity;
};

int initDArray(struct DynamicArray* d_array);

int addDArray(struct DynamicArray* d_array, struct Lexem lex);

void deleteDArray(struct DynamicArray *d_array);

//==================DynamicArray=============
char* inputString(char end);

char* copyString(const char* source);

//инициализирует динамический массив
//проверяет корректность входных данных
int initAndSetArray(struct DynamicArray *dest);
int setArray(struct Set *dest);

//выполняет операцию над множествами, переданную по указателю
//операнды извлекаются из стека
//полученный результат заносится в стэк и в массив
int execOperation(int(*operation)(struct Set, struct Set, struct Set *),
                  struct Stack *stack, struct DynamicArray *dest);

//выполняет вычисления
int count(const struct DynamicArray exp, struct Set* dest);
//возвращает приоритет операции
int priority(char operator1);

int main() {
    // Инициализируем массив для строк и выделяем память
    struct DynamicArray exp;
    int res = initDArray(&exp);
    if (!res) {
        deleteDArray(&exp);
        printf("[error]\n");
        return 0;
    }
    res = initAndSetArray(&exp);
    if (!res) {
        deleteDArray(&exp);
        printf("[error]\n");
        return 0;
    }
    struct Set res_set;
    res =  count(exp, &res_set);
    if (res) {
        outputSet(res_set);
    } else {
        deleteDArray(&exp);
        printf("[error]\n");
    }
    deleteSet(&res_set);
    deleteDArray(&exp);
    return 0;
}

char* inputString(char end) {
    size_t cur_size = BUFF_SIZE;
    char* str = (char*)malloc(sizeof(char) * cur_size);
    if (str == NULL) {
        return NULL;
    }
    char cur_char = getchar();
    size_t i;
    for (i = 0; cur_char != end && cur_char != EOF; ++i, cur_char = getchar()) {
        //grow if necessary
        if (i >= cur_size) {
            cur_size *= REALLOC_COEFFICIENT;
            char* res = realloc(str, sizeof(char) * cur_size);
            if (res == NULL) {
                free(str);
                return NULL;
            }
            str = res;
        }
        //add
        str[i] = cur_char;
    }
    //free and return if EOF is first read
    if (i == 0 && cur_char == EOF) {
        free(str);
        return NULL;
    }
    //grow if necessary
    if (i >= cur_size) {
        ++cur_size;
        void * res = realloc(str, sizeof(char) * cur_size);
        if (res == NULL) {
            free(str);
            return NULL;
        }
        str = res;
    }
    //add a terminating null character
    str[i] = '\0';
    return str;
}

char* copyString(const char* source) {
    size_t length = strlen(source);
    char* new_str = malloc(length + 1);
    if (new_str == NULL) {
        return NULL;
    }
    memcpy(new_str, source, length + 1);
    return new_str;
}

int setArray(struct Set *dest) {
    struct Set res;
    initSet(&res);
    char cur_ch = getchar();
    while (cur_ch != EOF && cur_ch != '\n' && cur_ch != END_SET_CH) {
        if (cur_ch == BORDER_ELEM_CH) {
            char* elem = inputString(BORDER_ELEM_CH);
            if (elem == NULL) {
                deleteSet(&res);
                return 0;
            }
            if (elem[0] == '\0') {
                free(elem);
                deleteSet(&res);
                return 0;
            }
            addSet(&res, elem);
            cur_ch = getchar();
            if (cur_ch != ELEM_SEPARATOR && cur_ch != END_SET_CH) {
                return 0;
            }
        } else if (cur_ch == ELEM_SEPARATOR){
            cur_ch = getchar();
        } else {
            deleteSet(&res);
            return 0;
        }
    }
    if (cur_ch != END_SET_CH) {
        deleteSet(&res);
        return 0;
    }
    *dest = res;
    return 1;
}

int initAndSetArray(struct DynamicArray *dest) {
    struct Lexem tmp_lex;
    struct Stack stack;
    stack.top = NULL;
    char cur_char;
    for (cur_char = getchar(); cur_char != '\n' && cur_char != EOF; cur_char = getchar()){
        switch ((int)cur_char) {
            case START_SET_CH:
                tmp_lex.type = SET;
                int res = setArray(&tmp_lex.set);
                if (res == 0) {
                    deleteStack(&stack);
                    return 0;
                }
                if (!addDArray(dest, tmp_lex)) {
                    deleteStack(&stack);
                    return 0;
                }
                break;
            case UNION:
                tmp_lex.type = UNION;
                if (!emptyStack(&stack)) {
                    while (priority(tmp_lex.type) <= priority(top(&stack).type)) {
                        if (!addDArray(dest, pop(&stack))) {
                            deleteStack(&stack);
                            return 0;
                        }
                        if (emptyStack(&stack)) break;
                    }
                }
                push(&stack, tmp_lex);
                break;
            case INTERSECTION:
                tmp_lex.type = INTERSECTION;
                if (!emptyStack(&stack)) {
                    while (priority(tmp_lex.type) <= priority(top(&stack).type)) {
                        if (!addDArray(dest, pop(&stack))) {
                            deleteStack(&stack);
                            return 0;
                        }
                        if (emptyStack(&stack)) break;
                    }
                }
                push(&stack, tmp_lex);
                break;
            case COMPLEMENT:
                tmp_lex.type = COMPLEMENT;
                if (!emptyStack(&stack)) {
                    while (priority(tmp_lex.type) <= priority(top(&stack).type)) {
                        if (!addDArray(dest, pop(&stack))) {
                            deleteStack(&stack);
                            return 0;
                        }
                        if (emptyStack(&stack)) break;
                    }
                }
                push(&stack, tmp_lex);
                break;
            case OPEN_BRACKET:
                tmp_lex.type = OPEN_BRACKET;
                push(&stack, tmp_lex);
                break;
            case CLOSE_BRACKET:
                if (emptyStack(&stack)) {
                    deleteStack(&stack);
                    return 0;
                } else {
                    while (top(&stack).type != OPEN_BRACKET) {
                        if (!addDArray(dest, pop(&stack))) {
                            deleteStack(&stack);
                            return 0;
                        }
                        if (emptyStack(&stack)) {
                            deleteStack(&stack);
                            return 0;
                        }
                    }
                    pop(&stack);
                }
                break;
            default:
                deleteStack(&stack);
                return 0;
        }
    }
    while (!emptyStack(&stack)) {
        if(top(&stack).type == OPEN_BRACKET) {
            deleteStack(&stack);
            return 0;
        }
        if (!addDArray(dest, pop(&stack))) {
            deleteStack(&stack);
            return 0;
        }
    }
    return 1;
}

int execOperation(int(*operation)(struct Set, struct Set, struct Set *),
                  struct Stack *stack, struct DynamicArray *dest) {
    if (emptyStack(stack)) {
        return 0;
    }
    struct Lexem b = pop(stack);
    if (emptyStack(stack)) {
        return 0;
    }
    struct Lexem a = pop(stack);
    struct Lexem res;
    res.type = SET;
    int operation_res = operation(a.set, b.set, &res.set);
    if (operation_res == 0) {
        return 0;
    }
    if (!push(stack, res)) {
        return 0;
    }
    if (!addDArray(dest, res)) {
        return 0;
    }
    return 1;
}

int count(const struct DynamicArray exp, struct Set *dest) {
    initSet(dest);
    struct Stack stack;
    stack.top = NULL;

    struct DynamicArray tmp_sets;
    int res = initDArray(&tmp_sets);
    if (!res) {
        deleteStack(&stack);
        deleteSet(dest);
        return 0;
    }
    size_t i = 0;
    for (i = 0; i < exp.size; ++i) {
        switch (exp.data[i].type) {
            case SET: {
                if (!push(&stack, exp.data[i])) {
                    deleteStack(&stack);
                    deleteSet(dest);
                    deleteDArray(&tmp_sets);
                    return 0;
                }
                break;
            }
            case UNION: {
                int res = execOperation(unionSet, &stack, &tmp_sets);
                if (!res) {
                    deleteStack(&stack);
                    deleteSet(dest);
                    deleteDArray(&tmp_sets);
                }
                break;
            }
            case INTERSECTION: {
                int res = execOperation(intersectionSet, &stack, &tmp_sets);
                if (!res) {
                    deleteStack(&stack);
                    deleteSet(dest);
                    deleteDArray(&tmp_sets);
                }
                break;
            }
            case COMPLEMENT: {
                int res = execOperation(complementSet, &stack, &tmp_sets);
                if (!res) {
                    deleteStack(&stack);
                    deleteSet(dest);
                    deleteDArray(&tmp_sets);
                }
                break;
            }
            default:
                deleteStack(&stack);
                deleteSet(dest);
                deleteDArray(&tmp_sets);
                return 0;
        }
    }

    *dest = pop(&stack).set;
    if (!emptyStack(&stack)) {
        deleteStack(&stack);
        deleteSet(dest);
        deleteDArray(&tmp_sets);
        return 0;
    }
    //уменьшаю размер массива с временными множествами.
    //чтобы результирующее множество, также находящееся в этом массиве не было очищенно при выходе из функции
    if (tmp_sets.size != 0) {
        --tmp_sets.size;
    }
    deleteDArray(&tmp_sets);
    return 1;
}

int priority(char operator1) {
    switch(operator1) {
        case UNION:
            return 1;
        case INTERSECTION:
            return 2;
        case COMPLEMENT:
            return 3;
    }
    return -1;
}

//==================Set======================

void initSet(struct Set* set) {
    set->head = NULL;
}

void deleteSet(struct Set* set) {
    while (set->head != NULL) {
        struct SetNode* tmp = set->head->next;
        free(set->head->str);
        free(set->head);
        set->head = tmp;
    }
}

int addSet(struct Set* set,char* data) {
    struct SetNode* tmp = (struct SetNode*)malloc(sizeof(struct SetNode));
    if (tmp == NULL) return 0;
    tmp->str = data;
    if (set->head == NULL) {
        tmp->next = NULL;
        tmp->prev = NULL;
        set->head = tmp;
        return 1;
    }
    struct SetNode* cur_node = set->head;
    while (cur_node->next != NULL) {
        int res = strcmp(data, cur_node->str);
        if (res == 0) {
            free(tmp);
            free(data);
            return 1;
        } else if (res < 0) {
            tmp->prev = cur_node->prev;
            if (cur_node->prev != NULL) {
                cur_node->prev->next = tmp;
            } else {
                set->head = tmp;
            }
            cur_node->prev = tmp;
            tmp->next = cur_node;
            return 1;
        }
        cur_node = cur_node->next;
    }
    int res = strcmp(data, cur_node->str);
    if (res == 0) {
        free(tmp);
        free(data);
        return 1;
    } else if (res < 0) {
        tmp->prev = cur_node->prev;
        if (cur_node->prev != NULL) {
            cur_node->prev->next = tmp;
        } else {
            set->head = tmp;
        }
        cur_node->prev = tmp;
        tmp->next = cur_node;
    } else {
        cur_node->next = tmp;
        tmp->prev = cur_node;
        tmp->next = NULL;
    }
    return 1;
}

void outputSet(struct Set set)
{
    struct SetNode* cur_node = set.head;
    printf("[");
    while (cur_node != NULL) {
        printf("\"%s\"",cur_node->str);
        if (cur_node->next != NULL) printf(",");
        cur_node = cur_node->next;
    }
    printf("]\n");
}

int unionSet(struct Set a, struct Set b, struct Set* res) {
    initSet(res);
    struct SetNode* cur_elem = a.head;
    while (cur_elem != NULL) {
        char* copy_str = copyString(cur_elem->str);
        if (copy_str == NULL) {
            deleteSet(res);
            return 0;
        }
        addSet(res, copy_str);
        cur_elem = cur_elem->next;
    }
    cur_elem = b.head;
    while (cur_elem != NULL) {
        char* copy_str = copyString(cur_elem->str);
        if (copy_str == NULL) {
            deleteSet(res);
            return 0;
        }
        addSet(res, copy_str);
        cur_elem = cur_elem->next;
    }
    return 1;
}

int intersectionSet(struct Set a, struct Set b, struct Set *res) {
    initSet(res);
    struct SetNode* a_elem = a.head;
    while (a_elem != NULL) {
        struct SetNode* b_elem = b.head;
        while (b_elem != NULL) {
            if (strcmp(a_elem->str, b_elem->str) == 0) {
                char* copy_str = copyString(a_elem->str);
                if (copy_str == NULL) {
                    deleteSet(res);
                    return 0;
                }
                addSet(res, copy_str);
            }
            b_elem = b_elem->next;
        }
        a_elem = a_elem->next;
    }
    return 1;
}

int complementSet(struct Set a, struct Set b, struct Set *res) {
    initSet(res);
    struct SetNode* a_elem = a.head;
    while (a_elem != NULL) {
        struct SetNode* b_elem = b.head;
        int key = 1;
        while (b_elem != NULL) {
            if (strcmp(a_elem->str, b_elem->str) == 0) {
                key = 0;
                break;
            }
            b_elem = b_elem->next;
        }
        if (key){
            char* copy_str = copyString(a_elem->str);
            if (copy_str == NULL) {
                deleteSet(res);
                return 0;
            }
            addSet(res, copy_str);
        }
        a_elem = a_elem->next;
    }
    return 1;
}

//==================Set======================

//==================Stack====================

void deleteStack(struct Stack* stack) {
    while (stack->top != NULL) {
        pop(stack);
    }
}

int push(struct Stack* stack, struct Lexem val) {
    struct StackNode* tmp = (struct StackNode*)malloc(sizeof(struct StackNode));
    if (tmp == NULL) return 0;
    tmp->lex = val;
    tmp->next = stack->top;
    stack->top = tmp;
    return 1;
}

struct Lexem pop(struct Stack* stack) {
    struct StackNode* tmp = stack->top;
    struct Lexem top_lex = tmp->lex;
    stack->top = stack->top->next;
    free(tmp);
    return top_lex;
}

struct Lexem top(struct Stack* stack) {
    return stack->top->lex;
}

int emptyStack(struct Stack* stack) {
    return stack->top == NULL ? 1: 0;
}

//==================Stack====================

int initDArray(struct DynamicArray* d_array) {
    d_array->data = (struct Lexem*)malloc(sizeof(struct Lexem) * BUFF_SIZE);
    if (d_array->data == NULL) return 0;
    d_array->capacity = BUFF_SIZE;
    d_array->size = 0;
    return 1;
}

void deleteDArray(struct DynamicArray* d_array) {
    size_t i = 0;
    for (; i < d_array->size; ++i) {
        if (d_array->data[i].type == SET) {
            deleteSet(&(d_array->data[i].set));
        }
    }
    free(d_array->data);
}

int addDArray(struct DynamicArray* d_array, struct Lexem lex) {
    if (d_array->size == d_array->capacity) {
        struct Lexem* tmp = (struct Lexem*)realloc(d_array->data, sizeof(struct Lexem) * d_array->capacity * REALLOC_COEFFICIENT);
        if (tmp == NULL) {
            return 0;
        }
        d_array->capacity *= REALLOC_COEFFICIENT;
        d_array->data = tmp;
    }
    d_array->data[d_array->size++] = lex;
    return 1;
}