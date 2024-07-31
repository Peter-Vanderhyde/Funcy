#include "lexer.h"


char Lexer::get_next_character() {
    char next = source_code[current_position];
    current_position += 1;
    column += 1;
    if (next == '\n') {
        column = 0;
        line += 1;
    }
    return next;
}

char Lexer::peek_next_character() {
    return source_code[current_position];
}

void Lexer::tokenize() {
    while (current_position < source_code.length()) {
        char character = get_next_character();

        if (iswspace(character)) {
            // Handle whitespace
            continue;
        }

        else if (character == '#') {
            // Grab characters until it's reached the next line
            while (get_next_character() != '\n') {
                continue;
            }
        }

        else if (isalpha(character) || character == '_') {
            // Handle identifiers
            int l = line;
            int c = column;
            std::string identifier = "";
            identifier += character;

            char next = peek_next_character();
            while (isalnum(next) || next == '_') {
                identifier += get_next_character();
                next = peek_next_character();
            }

            // Check if it's a keyword
            if (keyword_tokens.contains(identifier)) {
                Token token{keyword_tokens[identifier], l, c};
                tokens.push_back(token);
            }
            else if (identifier == "true" || identifier == "false") {
                bool val{identifier == "true"};
                Token token{TokenType::_Boolean, val, l, c};
            }
            else {
                Token token{TokenType::_Identifier, identifier, l, c};
                tokens.push_back(token);
            }
        }
        else if (isdigit(character) || character == '"') {
            // Handle literals
            std::string literal = "";
            literal += character;
            int l = line;
            int c = column;

            char next = peek_next_character();
            if (character == '"') {
                while (current_position < source_code.length() && next != '"')
            }
            else {
                while (current_position < source_code.length() && (isdigit(next) || next == '.' || next == '"'))
            }
        }
    }
}

function tokenize()
    while not end of source_code
        character = get_next_character()
        
        if character is whitespace
            continue
        
        if character starts a comment
            handle_comment()
        
        if character starts an identifier
            handle_identifier()
        
        if character starts a literal
            handle_literal()
        
        if character is an operator or punctuation
            handle_operator_or_punctuation()
        
        if character is unrecognized
            handle_error("Unrecognized character", line, column)
    
    add end-of-file (EOF) token to tokens list
    return tokens

function handle_comment()
    while not end of comment
        character = get_next_character()

function handle_identifier()
    collect characters until a non-identifier character is found
    determine if it is a keyword or identifier
    create token with appropriate type
    add token to tokens list

function handle_literal()
    if character starts a numeric literal
        collect numeric characters
        create numeric literal token
        add token to tokens list
    elif character starts a string literal
        collect characters until closing quote is found
        create string literal token
        add token to tokens list

function handle_operator_or_punctuation()
    collect characters for multi-character operators if necessary
    create token for operator or punctuation
    add token to tokens list

function handle_error(message, line, column)
    print error message and location
    # Optionally, add an error token or stop processing

function main()
    source_code = read source code from file or input
    lexer = create Lexer with source_code
    tokens = lexer.tokenize()
    for token in tokens
        print token
