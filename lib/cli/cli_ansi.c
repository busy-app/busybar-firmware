#include "cli_ansi.h"

typedef enum {
    CliAnsiParserStateInitial,
    CliAnsiParserStateEscape,
    CliAnsiParserStateEscapeBrace,
    CliAnsiParserStateEscapeBraceOne,
    CliAnsiParserStateEscapeBraceOneSemicolon,
    CliAnsiParserStateEscapeBraceOneSemicolonModifiers,
} CliAnsiParserState;

struct CliAnsiParser {
    CliAnsiParserState state;
    CliModKey modifiers;
};

CliAnsiParser* cli_ansi_parser_alloc(void) {
    CliAnsiParser* parser = malloc(sizeof(CliAnsiParser));
    return parser;
}

void cli_ansi_parser_free(CliAnsiParser* parser) {
    free(parser);
}

/**
 * @brief Converts a single character representing a special key into the enum
 * representation
 */
static CliKey cli_ansi_key_from_mnemonic(char c) {
    switch(c) {
    case 'A':
        return CliKeyUp;
    case 'B':
        return CliKeyDown;
    case 'C':
        return CliKeyRight;
    case 'D':
        return CliKeyLeft;
    case 'F':
        return CliKeyEnd;
    case 'H':
        return CliKeyHome;
    default:
        return CliKeyUnrecognized;
    }
}

#define PARSER_RESET_AND_RETURN(parser, modifiers_val, key_val) \
    do {                                                        \
        parser->state = CliAnsiParserStateInitial;              \
        return (CliAnsiParserResult){                           \
            .is_done = true,                                    \
            .result = (CliKeyCombo){                            \
                .modifiers = modifiers_val,                     \
                .key = key_val,                                 \
            }};                                                 \
    } while(0);

CliAnsiParserResult cli_ansi_parser_feed(CliAnsiParser* parser, char c) {
    switch(parser->state) {
    case CliAnsiParserStateInitial:
        // <key> -> <key>
        if(c != CliKeyEsc) PARSER_RESET_AND_RETURN(parser, CliModKeyNo, c); // -V1048

        // <ESC> ...
        parser->state = CliAnsiParserStateEscape;
        break;

    case CliAnsiParserStateEscape:
        // <ESC> <ESC> -> <ESC>
        if(c == CliKeyEsc) PARSER_RESET_AND_RETURN(parser, CliModKeyNo, c);

        // <ESC> <key> -> Alt + <key>
        if(c != '[') PARSER_RESET_AND_RETURN(parser, CliModKeyAlt, c);

        // <ESC> [ ...
        parser->state = CliAnsiParserStateEscapeBrace;
        break;

    case CliAnsiParserStateEscapeBrace:
        // <ESC> [ <key mnemonic> -> <key>
        if(c != '1') PARSER_RESET_AND_RETURN(parser, CliModKeyNo, cli_ansi_key_from_mnemonic(c));

        // <ESC> [ 1 ...
        parser->state = CliAnsiParserStateEscapeBraceOne;
        break;

    case CliAnsiParserStateEscapeBraceOne:
        // <ESC> [ 1 <non-;> -> error
        if(c != ';') PARSER_RESET_AND_RETURN(parser, CliModKeyNo, CliKeyUnrecognized);

        // <ESC> [ 1 ; ...
        parser->state = CliAnsiParserStateEscapeBraceOneSemicolon;
        break;

    case CliAnsiParserStateEscapeBraceOneSemicolon:
        // <ESC> [ 1 ; <modifiers> ...
        parser->modifiers = (c - '0');
        parser->modifiers &= ~1;
        parser->state = CliAnsiParserStateEscapeBraceOneSemicolonModifiers;
        break;

    case CliAnsiParserStateEscapeBraceOneSemicolonModifiers:
        // <ESC> [ 1 ; <modifiers> <key mnemonic> -> <modifiers> + <key>
        PARSER_RESET_AND_RETURN(parser, parser->modifiers, cli_ansi_key_from_mnemonic(c));
    }

    return (CliAnsiParserResult){.is_done = false};
}

CliAnsiParserResult cli_ansi_parser_feed_timeout(CliAnsiParser* parser) {
    CliAnsiParserResult result = {.is_done = false};

    if(parser->state == CliAnsiParserStateEscape) {
        result.is_done = true;
        result.result.key = CliKeyEsc;
        result.result.modifiers = CliModKeyNo;
    }

    parser->state = CliAnsiParserStateInitial;
    return result;
}

size_t cli_ansi_printable_strlen(const char* string) {
    size_t count = 0;

    bool inside_escape_sequence = false;
    char seen_history[3] = {0};
    while((seen_history[2] = *string++)) {
        seen_history[0] = seen_history[1];
        seen_history[1] = seen_history[2];
        seen_history[2] = '\0';
        char c = seen_history[1];

        if(strcmp(seen_history, "\e[") == 0) {
            inside_escape_sequence = true;
            continue;
        }
        bool escape_sequence_terminator = (c >= 0x40) && (c <= 0x7e);
        if(inside_escape_sequence && escape_sequence_terminator) {
            inside_escape_sequence = false;
            continue;
        }
        if(inside_escape_sequence) continue;

        if(c >= 128) continue; // UTF-8 extension
        if(c < ' ') continue; // ASCII non-printable

        count++;
    }

    return count;
}
