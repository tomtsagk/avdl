#include "lexer.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"
#include "dd_commands.h"
#include <ctype.h>

extern char buffer[];

static void lexer_parse_command(FILE *f, struct ast_node *node, int lineNumber, int hasStarted) {
	int startedCommand = 0;
	int finishedCommand = 0;
	int result = 0;
	struct ast_node *cmd = 0;
	struct ast_node *arrayNode = 0;
	struct ast_node *lastIdentifier = 0;

	if (hasStarted) {
		cmd = ast_create(0, 0);
	}
	//printf("new command\n");
	while (!finishedCommand && !feof(f)) {

		// ignore whitespace
		fscanf(f, "%100[ \t]*", buffer);

		// read a character
		fscanf(f, "%1c", buffer);
		buffer[1] = '\0';

		// ignore comments
		if (buffer[0] == '#') {
			fscanf(f, "%*[^\n]*");
		}
		else
		// new lines
		if (buffer[0] == '\n') {
			lineNumber++;
		}
		else
		// start of command
		if (buffer[0] == '(') {

			if (arrayNode) {
				lexer_parse_command(f, arrayNode, lineNumber, 1);
			}
			else
			// a command within a command
			if (cmd) {
				lexer_parse_command(f, cmd, lineNumber, 1);
			}
			else {
				cmd = ast_create(0, 0);
			}
		}
		else
		// string
		if (buffer[0] == '\"') {

			fscanf(f, "%499[^\"]", buffer);
			buffer[499] = '\0';
			//printf("string: %s\n", buffer);
			struct ast_node *identifier = ast_create(AST_STRING, 0);
			ast_addLex(identifier, buffer);
			ast_child_add(cmd, identifier);

			fscanf(f, "%1c", buffer);
		}
		else
		// period (.)
		if (buffer[0] == '.') {
			lastIdentifier = dd_da_get(&cmd->children, cmd->children.elements-1);
		}
		else
		// array start
		if (buffer[0] == '[') {
			arrayNode = ast_create(AST_GROUP, 0);
		}
		else
		// array end
		if (buffer[0] == ']') {
			struct ast_node *lastNode = dd_da_get(&cmd->children, cmd->children.elements-1);

			// on an identifier chain, apply it to the last parsed element
			if (lastNode->children.elements > 0) {
				struct ast_node *lastIdentifier = dd_da_get(&lastNode->children, lastNode->children.elements-1);
				ast_child_add(lastIdentifier, arrayNode);
			}
			// no identifier chain, just apply it on the identifier
			else {
				ast_child_add(lastNode, arrayNode);
			}
			arrayNode = 0;
		}
		else
		// special characters only meant for native commands
		if (buffer[0] == '-'
		||  buffer[0] == '+'
		||  buffer[0] == '/'
		||  buffer[0] == '*'
		||  buffer[0] == '%'
		||  buffer[0] == '='
		||  buffer[0] == '<'
		||  buffer[0] == '>'
		||  buffer[0] == '!'
		||  buffer[0] == '&'
		||  buffer[0] == '|') {
			struct ast_node *identifier = 0;
			identifier = ast_create(AST_IDENTIFIER, 0);

			// check if some symbols come with "="
			if (buffer[0] == '='
			||  buffer[0] == '<'
			||  buffer[0] == '>'
			||  buffer[0] == '!') {
				long pos = ftell(f);
				char restId;
				fscanf(f, "%1c", &restId);
				if (restId != '=') {
					fseek(f, pos, SEEK_SET);
				}
				else {
					buffer[1] = restId;
				}
			}
			else
			if (buffer[0] == '&'
			||  buffer[0] == '|') {
				long pos = ftell(f);
				char restId;
				fscanf(f, "%1c", &restId);
				if (restId != buffer[0]) {
					fseek(f, pos, SEEK_SET);
				}
				else {
					buffer[1] = restId;
				}
			}
			ast_addLex(identifier, buffer);

			// command's name
			if (cmd->children.elements == 0) {

				// native command
				if (dd_commands_isNative(identifier->lex)
				&& identifier->children.elements == 0) {
					cmd->node_type = AST_COMMAND_NATIVE;
				}
				// custom command
				else {
					cmd->node_type = AST_COMMAND_CUSTOM;
				}
			}

			// signal to attach new identifier to the last one found (in a chain)
			if (lastIdentifier) {
				ast_child_add(lastIdentifier, identifier);
				lastIdentifier = 0;
			}
			else
			if (arrayNode) {
				ast_child_add(arrayNode, identifier);
			}
			else {
				ast_child_add(cmd, identifier);
			}
		}
		else
		// number
		if ((buffer[0] >= '0' && buffer[0] <= '9')) {

			char restNumber[500];
			restNumber[0] = '\0';
			if (fscanf(f, "%500[0-9.]", restNumber)) {
				strcat(buffer, restNumber);
			}

			int isFloat = 0;
			char *ptr = buffer;
			while (ptr[0] != '\0') {
				if (ptr[0] == '.') {
					isFloat = 1;
					break;
				}
				ptr++;
			}

			struct ast_node *identifier = 0;

			// parsing float
			if (isFloat) {
				identifier = ast_create(AST_FLOAT, 0);
				identifier->fvalue = atof(buffer);
			}
			// parsing int
			else {
				identifier = ast_create(AST_NUMBER, atoi(buffer));
			}
			if (arrayNode) {
				ast_child_add(arrayNode, identifier);
			}
			else {
				ast_child_add(cmd, identifier);
			}
		}
		else
		// identifier?
		if ((buffer[0] >= 'a' && buffer[0] <= 'z')
		||  (buffer[0] >= 'A' && buffer[0] <= 'Z')
		||   buffer[0] == '_') {
			char restNumber[500];
			fscanf(f, "%500[a-zA-Z0-9_]", restNumber);
			strcat(buffer, restNumber);
			//printf("symbol?: %s line: %d\n", buffer, lineNumber);

			struct ast_node *identifier = 0;
			identifier = ast_create(AST_IDENTIFIER, 0);
			ast_addLex(identifier, buffer);

			// command's name
			if (cmd->children.elements == 0) {

				// native command
				if (dd_commands_isNative(identifier->lex)
				&& identifier->children.elements == 0) {
					cmd->node_type = AST_COMMAND_NATIVE;
				}
				// custom command
				else {
					cmd->node_type = AST_COMMAND_CUSTOM;
				}
			}

			// signal to attach new identifier to the last one found (in a chain)
			if (lastIdentifier) {
				ast_child_add(lastIdentifier, identifier);
				lastIdentifier = 0;
			}
			else
			if (arrayNode) {
				ast_child_add(arrayNode, identifier);
			}
			else {
				ast_child_add(cmd, identifier);
			}
		}
		else
		if (buffer[0] == ')') {
			finishedCommand = 1;
			ast_child_add(node, cmd);
		}
		else {
			//fscanf(f, "%*s");
			//fscanf(f, "%1[^\n]", buffer);
			printf("ignored symbol: >%c<\n", buffer[0]);
			//printf("result: %d\n", result);
		}
	}

	if (cmd && !finishedCommand) {
		printf("lexer: error while parsing command\n");
		exit(-1);
	}
}

void lexer_convertToAst(struct ast_node *node, const char *filename) {

	printf("lexing: %s\n", filename);

	int lineNumber = 1;

	FILE *input_file = 0;
	input_file = fopen(filename, "r");
	if (!input_file) {
		printf("avdl error: Unable to open '%s': %s\n", filename, strerror(errno));
		exit(-1);
	}

	while (!feof(input_file)) {
		lexer_parse_command(input_file, node, lineNumber, 0);
	}

	fclose(input_file);

}
