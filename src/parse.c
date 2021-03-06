#include "mdcc.h"
#include <string.h>

// Lexer
/* if next token is expected to be a symbol
 * then read next token and return true
 * else return false
 */
bool consume(char *op) {
/*     printf("token->str: %s\n", token->str);
    printf("op: %s\n", op);
    printf("token->len: %d\n", token->len);
    printf("strlen(op): %ld\n", strlen(op)); */
    if((token->kind != TK_RESERVED) ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
            return false;
    }
    token = token->next;
    return true;
}

Token *consume_ident() {
    if(token != NULL && token->kind == TK_IDENT) {
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = TK_IDENT;
        tok->str = token->str;
        tok->len = token->len;
        token = token->next;
        return tok;
    } else {
        return NULL;
    }
}

/* if next token is expected symbol
 * then read next token
 * else report error
 */
void expect(char *op) {
    // printf("%s\n", op);
    if(token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
            error_at(token->str, "It's not %s\n", op);
    }
    token = token->next;
}

int expect_number() {
    if(token->kind != TK_NUM) {
        error_at(token->str, "It's not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

LVar *find_lvar(Token *tok) {
    for(LVar *var = locals; var; var = var->next) {
        if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

GVar *find_gvar(Token *tok) {
    for(GVar *var = globals; var; var = var->next) {
        if(var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
            return var;
        }
    }
    return NULL;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
            ('A' <= c && c <= 'Z') ||
            ('0' <= c && c <= '9') ||
            (c == '_');
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// create new token and connect to cur
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    // printf("%s\n", str);
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

void print_tokens(Token *token) {
    if(token != NULL) {
        switch(token->kind) {
            case TK_EOF:
                printf("TK_EOF\n");
                return;
                break;
            case TK_NUM:
                printf("TK_NUM: %d\n", token->val);
                break;
            case TK_IDENT:
                printf("TK_IDENT: %.*s\n", token->len, token->str);
                break;
            case TK_RESERVED:
                printf("TK_RESERVED: %.*s\n", token->len,  token->str);
                break;
            default:
                error("Can't find token in print_tokens");
                break;
        }
    }
    print_tokens(token->next);
}

// tokenize p and return it
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while(*p) {
        // printf("%ld\n", ph - p);
        // printf("%ld\n", cur - &head);
        // skip space
        if(isspace(*p)) {
            p++;
        } else if(!strncmp(p, "\n", 1) || !strncmp(p, "\t", 1)) {
            p++;
        } else if(!strncmp(p, "sizeof", 6) && !is_alnum(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
        } else if(!strncmp(p, "return", 6) && !is_alnum(p[6])) {
            cur = new_token(TK_RESERVED, cur, p, 6);
            p += 6;
        } else if(!strncmp(p, "while", 5) && !is_alnum(p[5])) {
            cur = new_token(TK_RESERVED, cur, p, 5);
            p += 5;
        } else if(!strncmp(p, "else", 4) && !is_alnum(p[4])) {
            cur = new_token(TK_RESERVED, cur, p, 4);
            p += 4;
        } else if(!strncmp(p, "char", 4) && !is_alnum(p[4])) {
            cur = new_token(TK_RESERVED, cur, p, 4);
            p += 4;
        } else if(!strncmp(p, "for", 3) && !is_alnum(p[3])) {
            cur = new_token(TK_RESERVED, cur, p, 3);
            p += 3;
        } else if(!strncmp(p, "int", 3) && !is_alnum(p[3])) {
            cur = new_token(TK_RESERVED, cur, p, 3);
            p += 3;
        } else if(!strncmp(p, "if", 2) && !is_alnum(p[2])) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
        } else if(!strncmp(p, ">=", 2) || !strncmp(p, "<=", 2) ||
                    !strncmp(p, "==", 2) || !strncmp(p, "!=", 2)) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
        } else if(*p == '+' || *p == '-' || *p == '*' || *p == '/' ||
                    *p == '(' || *p == ')' || *p == '>' || *p == '<' ||
                    *p == '=' || *p == ',' || *p == '{' || *p == '}' ||
                     *p == ';' || *p == '&' || *p == '[' || *p == ']') {
            cur = new_token(TK_RESERVED, cur, p++, 1);
        } else if(*p == '"') {
            char *q = ++p;
            while(*p && *p != '"') {
                p++;
            }
            if(!*p) {
                error_at(q, "string literal is not closed");
            }
            char s[p-q];
            char *now = q;
            int i = 0;
            while(now != p) {
                s[i] = *now;
                i++;
                now++;
            }
            cur = new_token(TK_STR, cur, q, p - q);

            cur->str = s;
            cur->len = p - q + 1;
        } else if('a' <= *p && *p <= 'z') {
            char *head = p;
            while(('a' <= *p && *p <= 'z') || ('0' <= *p && *p <= '9')) {
                p++;
            }
            int len = p - head;
            cur = new_token(TK_IDENT, cur, head, len);
            p = head + len;
        } else if(isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, -1);
            cur->val = strtol(p, &p, 10);
        } else {
            error_at(p, "Cant tokenize");
        }
    }

    new_token(TK_EOF, cur, p, -1);
    return head.next;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Parser

/*
 * expr = mul ("+" mul | "-" mul)*
 * mul = term ("*" term | "/" term)*
 * term = num | "(" expr ")"
 */


Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

char *enum2str(NodeKind kind) {
    switch(kind) {
        case ND_ADD:
            return "+";
        case ND_SUB:
            return "-";
        case ND_MUL:
            return "*";
        case ND_DIV:
            return "/";
        case ND_BEQ:
            return "==";
        case ND_NEQ:
            return "!=";
        case ND_LT:
            return "<";
        case ND_LE:
            return "<=";
        case ND_ASSIGN:
            return "=";
        case ND_RETURN:
            return "ret";
        case ND_APP:
            return "app ";
        case ND_NUM:
            return "val ";
        case ND_LVAR:
            return "var ";
        default:
            return "";
    }
}

void pprint_node(Node *node, int depth) {
    for(int i=0;i<depth;++i) {
        printf("  ");
    }
    printf("%s", enum2str(node->kind));
    if(node->kind == ND_LVAR || node->kind == ND_APP) {
        printf("%c\n", 'a' + node->lvar->offset);
    } else if(node->kind == ND_NUM) {
        printf("%d\n", node->val);
    } else {
        printf("\n");
    }
}

void pprint(char *str, int depth) {
    for(int i=0;i<depth;++i) {
        printf("  ");
    }
    printf("%s\n", str);
}

void print_nodes(Node *node, int depth) {
    if(node == NULL) return;
    switch(node->kind) {
        case ND_NUM:
            pprint_node(node, depth);
            break;
        case ND_LVAR:
            pprint_node(node, depth);
            break;
        case ND_IF:
            pprint("if", depth);
            print_nodes(node->children[0], depth + 1);
            pprint("then", depth);
            print_nodes(node->children[1], depth + 1);
            pprint("else", depth);
            print_nodes(node->children[2], depth + 1);
            pprint("endif", depth);
            break;
        case ND_WHILE:
            pprint("while", depth);
            print_nodes(node->children[0], depth + 1);
            pprint("do", depth);
            print_nodes(node->children[1], depth + 1);
            pprint("endwhile", depth);
            break;
        case ND_FOR:
            pprint("for", depth);
            print_nodes(node->children[0], depth + 1);
            print_nodes(node->children[1], depth + 1);
            print_nodes(node->children[2], depth + 1);
            pprint("do", depth);
            print_nodes(node->children[3], depth + 1);
            pprint("endfor", depth);
            break;
        case ND_RETURN:
            pprint("return", depth);
            print_nodes(node->lhs, depth + 1);
            break;
        case ND_BLOCK:
            pprint("{", depth);
            for(int i=0;node->children[i];++i) {
                print_nodes(node->children[i], depth + 1);
            }
            pprint("}", depth);
            break;
        case ND_APP:
            pprint_node(node, depth);
            for(int i=0;node->children[i];++i) {
                pprint_node(node->children[i], depth + 1);
            }
            printf("\n");
            break;
        case ND_DEREF:
            pprint("!", depth);
            print_nodes(node->lhs, depth + 1);
            break;
        case ND_ADDR:
            pprint("ref", depth);
            print_nodes(node->lhs, depth + 1);
            break;
        default:
            print_nodes(node->lhs, depth + 1);
            pprint_node(node, depth);
            print_nodes(node->rhs, depth + 1);
    }
}

void program() {
    int i = 0;
    while(!at_eof()) {
        locals = NULL;
        code[i] = top();
        code[i]->lvar = locals;
        i++;
    }
    
    code[i] = NULL;
}

void read_ident(Top *fun) {
    
    Type *tyarg = calloc(1, sizeof(Type));
    if(consume("int")) {
        tyarg->kind = INT;
    } else {
        tyarg->kind = CHAR;
        expect("char");
    }
    
    Token *tok = consume_ident();
    while(consume("*")) {
        Type *head = calloc(1, sizeof(Type));
        head->kind = PTR;
        head->ptr_to = tyarg;
        tyarg = head;
    }
    Arg *arg = calloc(1, sizeof(Arg));
    arg->name = tok->str;
    arg->len = tok->len;
    arg->ty = tyarg;
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->ty = tyarg;

    arg->next = fun->arg;
    fun->arg = arg;
    lvar->next = locals;
    locals = lvar;
    return;
}

Top *top() {

    Type *ty = calloc(1, sizeof(Type));
    if(consume("int")) {
        ty->kind = INT;
    } else if(consume("char")) {
        ty->kind = CHAR;
    }
    while(consume("*")) {
        Type *head = calloc(1, sizeof(Type));
        head->kind = PTR;
        head->ptr_to = ty;
        ty = head;
    }

    Token *tok = consume_ident();
    Top *top = calloc(1, sizeof(Top));
    if(consume("(")) {
        if(!consume(")")) {
            read_ident(top);
            while(consume(",")){
                read_ident(top);
            }
            expect(")");
        }

        int i = 0;
        expect("{");
        while(!consume("}")) {
            top->children[i] = stmt();
            i++;
        }
        top->name = tok->str;
        top->len = tok->len;
        top->ty = ty;
    } else if(consume("[")) {
        GVar *gvar = calloc(1, sizeof(GVar));
        Type *head = calloc(1, sizeof(Type));
        head->kind = ARRAY;
        head->array_size = expect_number();
        head->ptr_to = ty;
        ty = head;
        gvar->name = tok->str;
        gvar->len = tok->len;
        gvar->next = globals;
        gvar->ty = ty;
        globals = gvar;
        expect("]");
        expect(";");
        top->gvar = gvar;
        top->ty = ty;
    } else {
        GVar *gvar = calloc(1, sizeof(GVar));
        gvar->name = tok->str;
        gvar->len = tok->len;
        gvar->next = globals;
        gvar->ty = ty;
        globals = gvar;
        expect(";");
        top->gvar = gvar;
        top->ty = ty;
    }
    return top;
}

Node *stmt() {
    Node *node;
    if(consume("if")) {
        node = new_node(ND_IF, NULL, NULL);        
        expect("(");
        node->children[0] = expr();
        expect(")");
        node->children[1] = stmt();
        if(consume("else")) {
            node->children[2] = stmt();
        } else {
            node->children[2] = NULL;
        }
        return node;
    } else if(consume("while")) {
        node = new_node(ND_WHILE, NULL, NULL);
        expect("(");
        node->children[0] = expr();
        expect(")");
        node->children[1] = stmt();
        return node;
    } else if(consume("for")) {
        node = new_node(ND_FOR, NULL, NULL);
        expect("(");
        if(!consume(";")) {
            node->children[0] = expr();
            expect(";");
        }
        if(!consume(";")) {
            node->children[1] = expr();
            expect(";");
        }
        if(!consume(")")) {
            node->children[2] = expr();
            expect(")");
        }
        node->children[3] = stmt();
        return node;
    } else if(consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    } else if(consume("{")) {
        node = new_node(ND_BLOCK, NULL, NULL);
        int i = 0;
        while(!consume("}")) {
            if(token == NULL) {
                error("expected '}' at end of input");
            }
            // TODO "if(1 == 1) {;" とかで無限ループしそう　確認
            node->children[i++] = stmt();
        }
        node->children[i] = NULL;
    } else if(consume("int")) {
        node = new_node(ND_NULL, NULL, NULL);
        Type *ty = calloc(1, sizeof(Type));
        ty->kind = INT;
        while(consume("*")) {
            Type *head = calloc(1, sizeof(Type));
            head->kind = PTR;
            head->ptr_to = ty;
            ty = head;
        }
        Token *tok = consume_ident();
        if(tok == NULL) {
            error("ident expected");
        }
        if(consume("[")) {
            LVar *lvar = calloc(1, sizeof(LVar));
            Type *head = calloc(1, sizeof(Type));
            head->kind = ARRAY;
            head->array_size = expect_number();
            head->ptr_to = ty;
            ty = head;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->next = locals;
            lvar->ty = ty;
            locals = lvar;
            expect("]");
        } else {
            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->next = locals;
            lvar->ty = ty;
            locals = lvar;
        }
        expect(";");
        return node;
    } else if(consume("char")) {
        node = new_node(ND_NULL, NULL, NULL);
        Type *ty = calloc(1, sizeof(Type));
        ty->kind = CHAR;
        while(consume("*")) {
            Type *head = calloc(1, sizeof(Type));
            head->kind = PTR;
            head->ptr_to = ty;
            ty = head;
        }
        Token *tok = consume_ident();
        if(tok == NULL) {
            error("ident expected");
        }
        if(consume("[")) {
            LVar *lvar = calloc(1, sizeof(LVar));
            Type *head = calloc(1, sizeof(Type));
            head->kind = ARRAY;
            head->array_size = expect_number();
            head->ptr_to = ty;
            ty = head;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->next = locals;
            lvar->ty = ty;
            locals = lvar;
            expect("]");
        } else {
            LVar *lvar = calloc(1, sizeof(LVar));
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->next = locals;
            lvar->ty = ty;
            locals = lvar;
        }
        expect(";");
        return node;
    } else {
        node = expr();
        expect(";");
        return node;
    }
}

Node *expr() {
    return assign();
}

Node *assign() {
    Node *node = equality();
    if(consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Node *equality() {
    Node *node = relational();
    while(1) {
        if(consume("==")) {
            node = new_node(ND_BEQ, node, relational());
        } else if(consume("!=")) {
            node = new_node(ND_NEQ, node, relational());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = add();
    while(1) {
        if(consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if(consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if(consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else if(consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    while(1) {
        if(consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if(consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *mul() {
    Node *node = unary();
    for(;;) {
        // printf("mul: %s\n", token->str);
        if(consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if(consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else {
            return node;
        }
    }
}

Node *unary() {
    // printf("unary: %s\n", token->str);
    if(consume("sizeof")) {
        return new_node(ND_SIZEOF, unary(), NULL);
    } else if(consume("+")) {
        return term();
    } else if(consume("-")) {
        return new_node(ND_SUB, new_node_num(0), term());
    } else if(consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    } else if(consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    } else {
        return postfix();
    }
}

Node *postfix() {
    Node *node1 = term();

    while(consume("[")) {
        Node *node2 = new_node(ND_ADD, node1, expr());
        expect("]");
        node1 = new_node(ND_DEREF, node2, NULL);
    }
    return node1;
}

Node *term() {
    // if next_token == '(' then term must be '(' expr ')'
    // printf("term: %s\n", token->str);
    Node *node = calloc(1, sizeof(Node));
    Token *tok = consume_ident();

    if(tok) {
        int func_number = 0;
        while(code[func_number]) {
            if(tok->str == code[func_number]->name) break;
            func_number++;
        }
        if(consume("(")) {
            node->kind = ND_APP;
            int i = 0;
            if(!consume(")")) {
                node->children[i++] = assign();
                while(consume(",")) {
                    node->children[i++] = assign();
                }
                expect(")");
            }
            LVar *lvar = calloc(1, sizeof(LVar));
            node->lvar = lvar;
            node->lvar->name = tok->str;
            node->lvar->len = tok->len;
            return node;
        } else if(node->lvar = find_lvar(tok)) {
            node->kind = ND_LVAR;
            return node;
        } else if(node->gvar = find_gvar(tok)) {
            node->kind = ND_GVAR;
            return node;
        }
        
    } else if(consume("(")) {
        node = expr();
        expect(")");
        return node;
    } else if(token->kind = TK_STR) {
        Str *heads = calloc(1, sizeof(Str));
        heads->body = token->str;
        heads->len = token->len;
        heads->next = strings;
        heads->label = fresh_str_id();
        strings = heads;

        Type *ty = calloc(1, sizeof(Type));
        ty->kind = CHAR;
        Type *headt = calloc(1, sizeof(Type));
        headt->kind = ARRAY;
        headt->array_size = token->len;
        headt->ptr_to = ty;
        ty = headt;

        node = new_node(ND_GVAR, NULL, NULL);
        node->str = heads;
        node->ty = ty;
        node->gvar = calloc(1, sizeof(GVar));
        node->gvar->ty = ty;
        node->gvar->label = heads->label;
        token = token->next;
    } else {
        // term muse be a number
        return new_node_num(expect_number());
    }
}