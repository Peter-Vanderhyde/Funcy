# calculator.fy

# Global parser state
TOKENS = [];
POS = 0;

# ========== TOKENIZER ==========

func tokenize(expr) {
    tokens = [];
    i = 0;
    while i < length(expr) {
        ch = expr[i];

        # skip whitespace
        if ch == " " or ch == "\t" {
            i = i + 1;
            continue;
        }

        # number literal (integer)
        if ch.isDigit() {
            start = i;
            while i < length(expr) and expr[i].isDigit() {
                i = i + 1;
            }
            num_str = expr[start:i];  # slice
            tokens.append(["NUMBER", num_str]);
            continue;
        }

        # operators
        if ch == "+" or ch == "-" or ch == "*" or ch == "/" {
            tokens.append(["OP", ch]);
            i = i + 1;
            continue;
        }

        # parentheses
        if ch == "(" or ch == ")" {
            tokens.append(["PAREN", ch]);
            i = i + 1;
            continue;
        }

        # unknown char
        print("Unknown character: " + ch);
        return Null;
    }

    return tokens;
}

# ========== PARSER HELPERS ==========

func current_token() {
    if POS >= length(TOKENS) {
        return Null;
    }
    return TOKENS[POS];
}

func advance() {
    global POS;
    POS = POS + 1;
}

func match(type_name, value) {
    tok = current_token();
    if tok == Null {
        return false;
    }
    # tok is like ["TYPE", "value"]
    if tok[0] == type_name and tok[1] == value {
        advance();
        return true;
    }
    return false;
}

# ========== PARSER: expression -> term (('+'|'-') term)* ==========

func parse_expression() {
    value = parse_term();
    while true {
        tok = current_token();
        if tok == Null {
            break;
        }
        if tok[0] == "OP" and (tok[1] == "+" or tok[1] == "-") {
            op = tok[1];
            advance();
            right = parse_term();
            if op == "+" {
                value = value + right;
            } else {
                value = value - right;
            }
        } else {
            break;
        }
    }
    return value;
}

# ========== PARSER: term -> factor (('*'|'/') factor)* ==========

func parse_term() {
    value = parse_factor();
    while true {
        tok = current_token();
        if tok == Null {
            break;
        }
        if tok[0] == "OP" and (tok[1] == "*" or tok[1] == "/") {
            op = tok[1];
            advance();
            right = parse_factor();
            if op == "*" {
                value = value * right;
            } else {
                # basic integer division; adjust if you have floats
                value = value // right;
            }
        } else {
            break;
        }
    }
    return value;
}

# ========== PARSER: factor ==========

func parse_factor() {
    tok = current_token();
    if tok == Null {
        print("Unexpected end of input in factor");
        return 0;
    }

    # unary + / -
    if tok[0] == "OP" and (tok[1] == "+" or tok[1] == "-") {
        op = tok[1];
        advance();
        value = parse_factor();
        if op == "-" {
            return 0 - value;
        } else {
            return value;
        }
    }

    # number
    if tok[0] == "NUMBER" {
        advance();
        return int(tok[1]);
    }

    # parenthesized expression: '(' expression ')'
    if tok[0] == "PAREN" and tok[1] == "(" {
        advance();  # consume '('
        value = parse_expression();
        if not match("PAREN", ")") {
            print("Error: expected ')'");
        }
        return value;
    }

    print("Unexpected token in factor: " + tok[0] + " " + tok[1]);
    advance();
    return 0;
}

# ========== TOP-LEVEL EVAL ==========

func eval_expr(line) {
    global TOKENS;
    global POS;

    TOKENS = tokenize(line);
    if TOKENS == Null {
        return Null;
    }

    POS = 0;
    result = parse_expression();

    # If there are leftover tokens, it's an error / extra input
    if POS < length(TOKENS) {
        print("Warning: extra tokens after end of expression");
    }

    return result;
}

# ========== REPL ==========

func main() {
    print("Simple expression calculator");
    print("Supports + - * / and parentheses, integers only.");
    print("Type 'quit' to exit.");

    while true {
        line = input("expr> ");
        if line == Null {
            break;
        }
        if line == "" {
            continue;
        }
        if line == "quit" or line == "exit" {
            break;
        }

        result = eval_expr(line);
        if result != Null {
            print(str(result));
        }
    }

    print("Goodbye.");
}

main();