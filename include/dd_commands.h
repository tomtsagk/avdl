#ifndef DD_COMMANDS_H
#define DD_COMMANDS_H

#include "ast_node.h"

struct keyword_function {
	char *keyword;
	struct ast_node *(*function)(struct ast_node *name, struct ast_node *args);
};

// keywords available in the language
extern const struct keyword_function keywords[];
extern unsigned int keywords_total;

// parse a single command and return a node
struct ast_node *parse_command(struct ast_node *cmd_name, struct ast_node *opt_args);

/*
 * compiling platform
 */
enum AVDL_PLATFORM {
	AVDL_PLATFORM_NATIVE,
	AVDL_PLATFORM_ANDROID,
};
extern enum AVDL_PLATFORM avdl_platform;

#endif
